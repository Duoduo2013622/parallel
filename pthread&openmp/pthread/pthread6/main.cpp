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
int n=100;
int max_n=2000;
float **A;
float **M;
int threadcount=8;
sem_t sem_leader;
sem_t sem_Divsion[7];
sem_t sem_Elimination[7];
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
void guass_normal(ofstream &result)
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
	result << (double)(sum/3)<<" ";

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
        else{
            sem_wait(&sem_Divsion[t_id-1]);
        }


        if(t_id==0)
        {

            for(int i=0;i<threadcount-1;i++)
            {
                sem_post(&sem_Divsion[i]);
            }
        }

        //循环瓜分任务
        for(int i=k+1+t_id;i<n;i+=threadcount)
        {
            for(int j=k+1;j<n;j++)
            {
                A[i][j]=A[i][j]-A[i][k]*A[k][j];
            }
            A[i][k]=0.0;
        }


        //所有线程进入下一轮
        if(t_id==0)
        {
            for(int i=0;i<threadcount-1;i++)
            {

                sem_wait(&sem_leader);
            }
            for(int i=0;i<threadcount-1;i++)
            {

                sem_post(&sem_Elimination[i]);
            }
        }
        else{   sem_post(&sem_leader);
                sem_wait(&sem_Elimination[t_id-1]);}
    }

    pthread_exit(NULL);
}
int main()
{
    ofstream result("C:\\Users\\cuiji\\Desktop\\bing_pthread\\bing_pthread6.txt");
    result<<"pthread_静态信号量改_循环分组"<<endl;
    while(n<=max_n)
    {


    cout<<n<<endl;
    //creat
    m_reset();
    long long sum=0;
    for(int re=0;re<3;re++)
    {
    long long head,tail,freq ; // timers
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq );
    QueryPerformanceCounter((LARGE_INTEGER *)&head);
    //
    sem_init(&sem_leader,0,0);
    for(int t_id=0;t_id<threadcount-1;t_id++)
    {
        sem_init(&sem_Divsion[t_id],0,0);
        sem_init(&sem_Elimination[t_id],0,0);

    }

    //创建线程
    pthread_t* handles;
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
    sem_destroy(&sem_leader);
    for(int t_id=0;t_id<threadcount-1;t_id++)
    {
        sem_destroy(&sem_Divsion[t_id]);
        sem_destroy(&sem_Elimination[t_id]);

    }


    QueryPerformanceCounter((LARGE_INTEGER *)&tail );
    sum+=(tail-head)*1000.0 / freq;
    }
	result << (double)(sum/3)<<endl;










        //









    n=n+100;
    }





    return 0;
}
