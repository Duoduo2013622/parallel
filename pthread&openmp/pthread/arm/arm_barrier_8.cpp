#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include<arm_neon.h>
#include<stdio.h>
#include<time.h>
using namespace std;
int n=100;
int max_n=2000;
float **A;
float **M;
int threadcount=7;
pthread_barrier_t barrier_Divsion;
pthread_barrier_t barrier_Elimination;

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
         A[i][j]=rand();
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

typedef struct{
    int k;
    int t_id;

}threadParam_t;
void *threadFunc(void *param)
{
    threadParam_t *p=(threadParam_t*)param;
    int t_id=p->t_id;

    for(int k=0;k<n;k++)
    {
        if(t_id==0)
        {
            for(int j=k+1;j<n;j++)
            {
                A[k][j]=A[k][j]/A[k][k];
            }
            A[k][k]=1.0;
        }
        pthread_barrier_wait(&barrier_Divsion);




        
        for(int i=k+1+t_id;i<n;i+=threadcount)//循环瓜分任务
        {
            for(int j=k+1;j<n;j++)
            {
                A[i][j]=A[i][j]-A[i][k]*A[k][j];
            }
            A[i][k]=0;
        }


        
        pthread_barrier_wait(&barrier_Elimination);//第二个同步点
    }

    pthread_exit(NULL);
}
int main()
{
    while(n<=max_n)
    {



   
    m_reset();
    struct timespec sts,ets;
    timespec_get(&sts, TIME_UTC);
  
    pthread_barrier_init(&barrier_Divsion,NULL,threadcount);
    pthread_barrier_init(&barrier_Elimination,NULL,threadcount);
  
    pthread_t* handles;//创建线程
    handles=(pthread_t*)malloc(threadcount*sizeof(pthread_t));
    threadParam_t* param=(threadParam_t*)malloc(threadcount*sizeof(threadParam_t));
    for(int t_id=0;t_id<threadcount;t_id++)
    {
        param[t_id].t_id=t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
    }
    for(int t_id=0;t_id<threadcount;t_id++)
    {

        pthread_join(handles[t_id],NULL);
    }

    pthread_barrier_destroy(&barrier_Divsion);
    pthread_barrier_destroy(&barrier_Elimination);

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


    n=n+100;
    }


    return 0;
}
