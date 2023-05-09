#include <iostream>
#include <windows.h>
#include <fstream>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
#include <smmintrin.h> //SSE4.1
#include <nmmintrin.h> //SSSE4.2
#include <semaphore.h>

using namespace std;

int n=1000;
int max_n=1000;
float **A;
float **M;
int threadcount=7;
sem_t sem_main;
sem_t sem_workerstart[7];
sem_t sem_workerend[7];
//creat
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
    long long sum=0;
    for(int re=0;re<3;re++)
    {
    long long head,tail,freq ; // timers
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq );
    QueryPerformanceCounter((LARGE_INTEGER *)&head);
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
	QueryPerformanceCounter((LARGE_INTEGER *)&tail );
	sum+=(tail-head)*1000.0 / freq;
    }


}
typedef struct{
    //int k;
    int t_id;

}threadParam_t;
//void *threadfunc(void *param)
//{
//    threadParam_t *p=(threadParam_t*)param;
//    int t_id=p->t_id;
//    int i=k+t_id+1;
//    for(;i<n;i=i+threadcount)
//    {
//        for(int j=k+1;j<n;j++)
//        {
//            A[i][j]=A[i][j]-A[i][k]*A[k][j];
//
//        }
//        A[i][k]=0;
//    }
//
//
//    pthread_exit(NULL);
//}
void *threadFunc(void *param)
{
    threadParam_t *p=(threadParam_t*)param;
    int t_id=p->t_id;

    for(int k=0;k<n;k++)
    {
        sem_wait(&sem_workerstart[t_id]);
        int i=k+t_id+1;
        for(;i<n;i=i+threadcount)
        {
            for(int j=k+1;j<n;j++)
            {
                A[i][j]=A[i][j]-A[i][k]*A[k][j];

            }
            A[i][k]=0;
        }
        //

        sem_post(&sem_main);
        sem_wait(&sem_workerend[t_id]);
    }
    pthread_exit(NULL);
}


int main()
{
//    ofstream result("C:\\Users\\cuiji\\Desktop\\bing_pthread\\bing_pthread3_2.txt");
//    result<<"串行高斯 pthread_静态信号量_循环分组"<<endl;

    while(n<=max_n)
    {

        cout<<n<<endl;
        //creat
        m_reset();





        sem_init(&sem_main,0,0);
        for(int t_id=0;t_id<threadcount;t_id++)
        {
            sem_init(&sem_workerstart[t_id],0,0);
            sem_init(&sem_workerend[t_id],0,0);
        }

        pthread_t* handles;
        handles=(pthread_t*)malloc(threadcount*sizeof(pthread_t));
        threadParam_t* param=(threadParam_t*)malloc(threadcount*sizeof(threadParam_t));
        for(int t_id=0;t_id<threadcount;t_id++)
        {
            param[t_id].t_id=t_id;
            pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
        }


        for(int k=0;k<n;k++)
        {

            if(k>=n-threadcount)
            {
                threadcount--;

            }
            for(int j=k+1;j<n;j++)
            {
                A[k][j]=A[k][j]/A[k][k];
            }
            A[k][k]=1.0;

            //开始唤醒线程
            for(int t_id=0;t_id<threadcount;t_id++)
            {

                sem_post(&sem_workerstart[t_id]);
            }
            //主线程睡眠

            for(int t_id=0;t_id<threadcount;t_id++)
            {

                sem_wait(&sem_main);
            }
            //主程序再次唤醒

            for(int t_id=0;t_id<threadcount;t_id++)
            {

                sem_post(&sem_workerend[t_id]);
            }

        }
        for(int t_id=0;t_id<threadcount;t_id++)
        {

            pthread_join(handles[t_id],NULL);
        }
        sem_destroy(&sem_main);
        for(int t_id=0;t_id<threadcount;t_id++)
        {
            sem_destroy(&sem_workerstart[t_id]);
            sem_destroy(&sem_workerend[t_id]);
        }



        threadcount=7;










        //














    n=n+100;

    }




    return 0;
}
