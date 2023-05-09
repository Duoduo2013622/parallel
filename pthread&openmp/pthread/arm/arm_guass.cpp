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
int threadcount=3;

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
	printf("%lld.%09llds",dsec,dnsec);
	cout<<endl;


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
int main()
{
    cout<<"arm-guass"<<endl;
    while(n<=max_n)
    {


        m_reset();
        guass_normal();


    n=n+100;

    }


    return 0;
}
