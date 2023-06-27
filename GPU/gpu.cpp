#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include <iomanip>


using namespace std;

int N=2048;

void init_guass(float* data)
{
	for(int i=0;i<N;i++)
	{
		for(int j=0;j<i;j++)
        {
            data[i*N+j]=0;
            //f[i][j]=0;
        }

		data[i*N]=1.0;
		for(int j=i+1;j<N;j++)
        {
            data[i*N+j]=i+j;

        }

	}
	for(int k=0;k<N;k++)
    {
        for(int i=k+1;i<N;i++)
	      {
	          for(int j=0;j<N;j++)
            {
                data[i*N+j]+=data[i*N];

            }
	      }
    }

}
__global__ void division_guass(float* data, int k, int N)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;//计算线程索引
    int e = data[k*N+k];
    int t = data[k*N+i];

    data[k*N+i] = (float)t/e;
    return;
}

__global__ void eliminate_guass(float* data, int k, int N)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if(i==0)
    {
        data[k*N+k]=1.0;//对角线元素设为 1
        int row = blockIdx.x+k+1;//每个块负责一行
    }

    while(row<N)
    {
        int j = threadIdx.x;
        int col = k+1+j;
         while(col < N)
         {
            data[(row*N) + col] = data[(row*N) + col] - data[(row*N)+k]*data[k*N+col];
            j += blockDim.x;
        }
         __syncthreads();//块内同步
         if (threadIdx.x == 0)
         {
            data[row * N + k] = 0;
         }
         row += gridDim.x;
     }
     return;
 }
int main()
{

    int deviceId;
    int numberOfSMs;

    cudaGetDevice(&deviceId);
    cudaDeviceGetAttribute(&numberOfSMs, cudaDevAttrMultiProcessorCount, deviceId);

    size_t size = N*N * sizeof(float);

    float *data_g;

    cudaMallocManaged(&data_g, size)

    init_guass(data_g);

    cudaEvent_t start, stop;//计时器
    float elapsedTime = 0.0;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);//开始计时

    threadsPerBlock = 512;
    numberOfBlocks = 32 * numberOfSMs


    for(int k=0;k<width;k++)
    {
        division_guass<<<numberOfBlocks,threadsPerBlock>>>(data_g,k,N);//负责除法任务的核函数
        cudaDeviceSynchronize();//CPU 与 GPU 之间的同步函数

        eliminate_guass<<<numberOfBlocks,threadsPerBlock>>>(data_g,k,N);//负责消去任务的核函数
        cudaDeviceSynchronize();

     }

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);//停止计时
    cudaEventElapsedTime(&elapsedTime, start, stop);
    printf("GPU_LU:%f ms\n", elapsedTime);



    cudaFree(data_g)


    return 0;
}
