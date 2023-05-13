#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <omp.h>  // OpenMP 必带头文件
#include <pthread.h>
#include<arm_neon.h>
#define MAX_THREADS 7
using namespace std;
int n = 100;
int max_n = 2000;
float** A;
float** M;
int threadcount = 7;

void m_reset()
{

	A = new float* [n];
	M = new float* [n];
	for (int i = 0; i < n; i++)
	{
		A[i] = new float[n];
		M[i] = new float[n];
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < i; j++)
		{
			A[i][j] = 0;
			M[i][j] = 0;
		}
		A[i][i] = 1.0;
		M[i][i] = 1.0;
		for (int j = i + 1; j < n; j++)
		{
			A[i][j] = i + j;
			M[i][j] = A[i][j];
		}

	}

	for (int k = 0; k < n; k++)
	{

		for (int i = k + 1; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				A[i][j] += A[k][j];
				M[i][j] = A[i][j];
			}

		}
	}


}
void guass_swap()
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			A[i][j] = M[i][j];

		}

	}

}
void guass_normal()
{
    struct timespec sts,ets;
    timespec_get(&sts, TIME_UTC);
    for (int k = 0; k < n; k++)
    {
		for (int j = k + 1; j < n; j++)
		{
			A[k][j] /= A[k][k];
		}
		A[k][k] = 1.0;
		for (int i = k + 1; i < n; i++)
		{
			for (int j = k + 1; j < n; j++)
			{
				A[i][j] -= A[i][k] * A[k][j];
			}
			A[i][k] = 0;
		}
	}
	timespec_get(&ets,TIME_UTC);
    time_t dsec=ets.tv_sec-sts.tv_sec;
    long dnsec=ets.tv_nsec-sts.tv_nsec;
    if(dnsec<0)
    {
        dsec--;
        dnsec+=1000000000ll;
    }
	printf("%lld.%09llds",dsec,dnsec);
	cout<<" ";


}
void omp_row()
{

	omp_set_num_threads(MAX_THREADS);
	#pragma omp parallel
{


	for (int k = 0; k < n; k++)
	{

		#pragma omp single
		{
			int temp = A[k][k];
			for (int j = k + 1; j < n; j++)
			{
				A[k][j] = A[k][j] / temp;
			}
			A[k][k] = 1.0;
		}

		#pragma omp for
		for (int i = k + 1; i < n; i++ )
		{
			int temp = A[i][k];
			float32x4_t vaik=vdupq_n_f32(A[i][k]);
            int j=k+1;
            for(j=k+1;j+4<=n;j+=4)
            {
                float32x4_t vakj=vld1q_f32(&A[k][j]);
                float32x4_t vaij=vld1q_f32(&A[i][j]);
                float32x4_t vx=vmulq_f32(vakj,vaik);
                vaij=vsubq_f32(vaij,vx);

                vst1q_f32(&A[i][j], vaij);

            }
            while(j<n)
            {
                A[i][j]-=A[k][j]*temp;
                j++;
            }
            A[i][k]=0;

		}
	}

	}
}
int main()
{


	while (n <= max_n)
	{

		m_reset();

		guass_normal();

		guass_swap();





		struct timespec sts,ets;
		timespec_get(&sts, TIME_UTC);

		omp_row();
		
		timespec_get(&ets,TIME_UTC);
		time_t dsec=ets.tv_sec-sts.tv_sec;
		long dnsec=ets.tv_nsec-sts.tv_nsec;
		if(dnsec<0)
		{
			dsec--;
			dnsec+=1000000000ll;
		}
		printf("%lld.%09llds",dsec,dnsec);
		cout<<endl;


		n = n + 100;
	}
	return 0;
}
