#include<arm_neon.h>
#include<stdio.h>
#include<iostream>
#include<time.h>
#include <malloc.h>
#include <pthread.h>
using namespace std;
int n=100;
int max_n=2000;
float **A;
float **M;
int threadcount=7;
void m_reset()
{

    A=new float*[n];
	M=new float*[n];
    for(int i=0;i<n;i++)
	{
		A[i]=new float[n];
		M[i]=new float[n];
	}
  for(int i=0;i<n;i++)
  {
      for(int j=0;j<i;j++)
	  {
        A[i][j]=0;
		M[i][j]=0;
	  }
      A[i][i]=1.0;
	  M[i][i]=1.0;
      for(int j=i+1;j<n;j++)
      {
         A[i][j]=i+j;
		 M[i][j]=A[i][j];
      }

  }

  for(int k=0;k<n;k++)
  {

      for(int i=k+1;i<n;i++)
      {
          for(int j=0;j<n;j++)
		  {
			A[i][j]+=A[k][j];
			M[i][j]=A[i][j];
		  }

      }
  }


}
void guass_swap()
{
    for(int i=0;i<n;i++)
  {
      for(int j=0;j<n;j++)
	  {
        A[i][j]=M[i][j];

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
	//printf("%lld.%09llds",dsec,dnsec);
	cout<<dsec<<"."<<dnsec<<" ";

}
typedef struct{
    int k;
    int t_id;

}threadParam_t;

void *threadfunc(void *param)
{
    threadParam_t *p=(threadParam_t*)param;
    int k=p->k;
    int t_id=p->t_id;
    int i=k+t_id+1;
    for(;i<n;i=i+threadcount)
    {
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
                A[i][j]-=A[k][j]*A[i][k];
                j++;
            }
            A[i][k]=0;
    }


    pthread_exit(NULL);
}
int main()
{
    cout<<"arm_pthread_Neon_动态_循环分组"<<endl;
    while(n<=max_n)
    {


        //creat
        m_reset();
        struct timespec sts,ets;
        timespec_get(&sts, TIME_UTC);
        for(int k=0;k<n;k++)
        {
            if(k>=n-threadcount)
            {
                threadcount--;

            }
            float32x4_t vt = vdupq_n_f32(A[k][k]); // 存储的四个 float32 都初始化为A[k][k]
        int j;
        for(j=k+1;j+4<=n;j+=4)
        {
            float32x4_t va=vld1q_f32(&A[k][j]);
            va=vdivq_f32(va,vt);
            vst1q_f32(&A[k][j], va);// 将 q0 中 4 个 float32，赋值给以 d1 为起始地址的 4 个 float32

        }
        while(j<n)
            {
                A[k][j]=A[k][j]/A[k][k];
                j++;
            }
        A[k][k]=1.0;

            pthread_t* handles;
            handles=(pthread_t*)malloc(threadcount*sizeof(pthread_t));
            threadParam_t* param=(threadParam_t*)malloc(threadcount*sizeof(threadParam_t));
            for(int t_id=0;t_id<threadcount;t_id++)
            {
                param[t_id].k=k;
                param[t_id].t_id=t_id;

            }
            for(int t_id=0;t_id<threadcount;t_id++)
            {
                pthread_create(&handles[t_id],NULL,threadfunc,&param[t_id]);

            }
            for(int t_id=0;t_id<threadcount;t_id++)
            {
                pthread_join(handles[t_id],NULL);
            }
            cout<<k<<endl;
        }
        threadcount=7;
        timespec_get(&ets,TIME_UTC);
    time_t dsec=ets.tv_sec-sts.tv_sec;
    long dnsec=ets.tv_nsec-sts.tv_nsec;
    if(dnsec<0)
    {
        dsec--;
        dnsec+=1000000000ll;
    }
	//printf("%lld.%09llds",dsec,dnsec);
	cout<<dsec<<"."<<dnsec<<endl;;






        //
//        guass_swap();
//        guass_normal();












    n=n+100;

    }











    return 0;
}
