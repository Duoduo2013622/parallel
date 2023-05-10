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
using namespace std;
int n=100;
int max_n=2000;
float **A;
float **M;
int threadcount=7;
pthread_barrier_t barrier_Divsion;
pthread_barrier_t barrier_Elimination;
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

            for(int j=k+1+t_id;j<n;j+=threadcount)
            {
                A[k][j]=A[k][j]/A[k][k];
            }


        pthread_barrier_wait(&barrier_Divsion);
        A[k][k]=1.0;



        //ѭ���Ϸ�����
        int t=((int)((n-k-1)/threadcount)+1)*t_id;
    int I=k+t+1;
    int i=I;
    for(;(i<n)&&(i-I)<((int)((n-k-1)/threadcount)+1);i=i+1)
    {
        for(int j=k+1;j<n;j++)
        {
            A[i][j]=A[i][j]-A[i][k]*A[k][j];

        }
        A[i][k]=0;
    }
//        for(int i=k+1+t_id;i<n;i+=threadcount)
//        {
//            for(int j=k+1;j<n;j++)
//            {
//                A[i][j]=A[i][j]-A[i][k]*A[k][j];
//            }
//            A[i][k]=0;
//        }


        //�ڶ���ͬ����
        pthread_barrier_wait(&barrier_Elimination);
    }

    pthread_exit(NULL);
}
int main()
{
    ofstream result("C:\\Users\\86153\\Desktop\\pthread8_2.txt");
    result<<"pthread_barrier_�ֿ����"<<endl;
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
    pthread_barrier_init(&barrier_Divsion,NULL,threadcount);
    pthread_barrier_init(&barrier_Elimination,NULL,threadcount);
    //�����߳�
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

    pthread_barrier_destroy(&barrier_Divsion);
    pthread_barrier_destroy(&barrier_Elimination);

    QueryPerformanceCounter((LARGE_INTEGER *)&tail );
    sum+=(tail-head)*1000.0 / freq;
    }
	result << (double)(sum/3)<<endl;;


    n=n+100;
    }







        //














    return 0;
}
