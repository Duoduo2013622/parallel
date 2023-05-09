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
void *threadfunc(void *param)
{
    threadParam_t *p=(threadParam_t*)param;
    int k=p->k;
    int t_id=p->t_id;
    int i=k+t_id+1;
    for(;i<n;i=i+threadcount)
    {
        for(int j=k+1;j<n;j++)
        {
            A[i][j]=A[i][j]-A[i][k]*A[k][j];

        }
        A[i][k]=0;
    }


    pthread_exit(NULL);
}
void *threadfunc_sse(void *param)
{
    threadParam_t *p=(threadParam_t*)param;
    int k=p->k;
    int t_id=p->t_id;
    int i=k+t_id+1;
    for(;i<n;i=i+threadcount)
        {
            __m128 vaik=_mm_set1_ps(A[i][k]);
            int j=k+1;
            for(j=k+1;j+4<=n;j+=4)
            {
                __m128 vakj=_mm_loadu_ps(&A[k][j]);
                __m128 vaij=_mm_loadu_ps(&A[i][j]);
                __m128 vx=_mm_mul_ps(vakj,vaik);
                vaij=_mm_sub_ps(vaij,vx);

                _mm_storeu_ps(&A[i][j], vaij);

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

    ofstream result("C:\\Users\\86153\\Desktop\\pthread_sse.txt");
    result<<"串行高斯 pthread_动态_循环分组_SSE"<<endl;
    while(n<=max_n)
    {
        cout<<n<<endl;
        //creat
        m_reset();

        guass_normal(result);

        guass_swap();


        long long sum=0;
    for(int re=0;re<3;re++)
    {
        long long head,tail,freq ; // timers
        //pthread+sse
        QueryPerformanceFrequency((LARGE_INTEGER*)&freq );
        QueryPerformanceCounter((LARGE_INTEGER *)&head);
        for(int k=0;k<n;k++)
        {
            if(k>=n-threadcount)
            {
                threadcount--;

            }
//            for(int j=k+1;j<n;j++)
//            {
//                A[k][j]=A[k][j]/A[k][k];
//            }
//            A[k][k]=1.0;

            //sse
            __m128 vt = _mm_set1_ps(A[k][k]); // ?洢????? float32 ????????A[k][k]
            int j;
            for(j=k+1;j+4<=n;j+=4)
            {
                __m128 va= _mm_loadu_ps(&A[k][j]);
                va= _mm_div_ps(va,vt);
                _mm_storeu_ps(&A[k][j], va);// ?? q0 ?? 4 ?? float32????????? d1 ????????? 4 ?? float32

            }
            while(j<n)
            {
                A[k][j]=A[k][j]/A[k][k];
                j++;
            }
            A[k][k]=1.0;
            //sse

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
                pthread_create(&handles[t_id],NULL,threadfunc_sse,&param[t_id]);

            }
            for(int t_id=0;t_id<threadcount;t_id++)
            {
                pthread_join(handles[t_id],NULL);
            }

        }
        threadcount=7;
        QueryPerformanceCounter((LARGE_INTEGER *)&tail );
        sum+=(tail-head)*1000.0 / freq;
    }
	result << (double)(sum/3)<<endl;;



        //pthread
//        guass_swap();
//        QueryPerformanceFrequency((LARGE_INTEGER*)&freq );
//        QueryPerformanceCounter((LARGE_INTEGER *)&head);
//        for(int k=0;k<n;k++)
//        {
//            if(k>=n-threadcount)
//            {
//                threadcount--;
//
//            }
//            for(int j=k+1;j<n;j++)
//            {
//                A[k][j]=A[k][j]/A[k][k];
//            }
//            A[k][k]=1.0;
//
//            pthread_t* handles;
//            handles=(pthread_t*)malloc(threadcount*sizeof(pthread_t));
//            threadParam_t* param=(threadParam_t*)malloc(threadcount*sizeof(threadParam_t));
//            for(int t_id=0;t_id<threadcount;t_id++)
//            {
//                param[t_id].k=k;
//                param[t_id].t_id=t_id;
//
//            }
//            for(int t_id=0;t_id<threadcount;t_id++)
//            {
//                pthread_create(&handles[t_id],NULL,threadfunc,&param[t_id]);
//
//            }
//            for(int t_id=0;t_id<threadcount;t_id++)
//            {
//                pthread_join(handles[t_id],NULL);
//            }
//            cout<<k<<endl;
//        }
//        threadcount=7;
//        QueryPerformanceCounter((LARGE_INTEGER *)&tail );
//        cout<<"pthread="<<(tail-head)*1000.0 / freq<<endl;








        //












        n=n+100;
    }
































    return 0;
}
