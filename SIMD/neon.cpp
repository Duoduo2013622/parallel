#include<arm_neon.h>
#include<stdio.h>
#include<iostream>
#include<time.h>
using namespace std;
int n=50;
float **A;
float **B;
float **C;
float **D;
void m_reset(int n)
{

    A=new float*[n];
	B=new float*[n];
	C=new float*[n];
	D=new float*[n];
    for(int i=0;i<n;i++)
        {
            A[i]=new float[n];
			B[i]=new float[n];
			C[i]=new float[n];
			D[i]=new float[n];
        }
  for(int i=0;i<n;i++)
  {
      for(int j=0;j<i;j++)
        {
            A[i][j]=0;
			B[i][j]=0;
			C[i][j]=0;
			D[i][j]=0;
        }
      A[i][i]=1.0;
	  B[i][i]=1.0;
	  C[i][i]=1.0;
	  D[i][i]=1.0;
      for(int j=i+1;j<n;j++)
      {
         A[i][j]=rand();
		 B[i][i]=A[i][j];
		 C[i][i]=A[i][j];
		 D[i][i]=A[i][j];
      }

  }
  for(int k=0;k<n;k++)
  {

      for(int i=k+1;i<n;i++)
      {
          for(int j=0;j<n;j++)
            {
                A[i][j]+=A[k][j];
				B[i][i]=A[i][j];
				C[i][i]=A[i][j];
				D[i][i]=A[i][j];
            }
      }
  }

}
void guass_normal()
{
    struct timespec sts,ets;
    timespec_get(&sts,TIME_UTC);
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
   printf("%lld.%09lld",dsec,dnsec);
	
}
void guass_neon_up()
{
    struct timespec sts,ets;
    timespec_get(&sts, TIME_UTC);
    for(int k=0;k<n;k++)
    {
        float32x4_t vt = vdupq_n_f32(B[k][k]); // 存储的四个 float32 都初始化为A[k][k]
        int j;
        for(j=k+1;j+4<=n;j+=4)
        {
            float32x4_t va=vld1q_f32(&B[k][j]);
            va=vdivq_f32(va,vt);
            vst1q_f32(&B[k][j], va);// 将 q0 中 4 个 float32，赋值给以 d1 为起始地址的 4 个 float32

        }
        while(j<n)
            {
                B[k][j]=B[k][j]/B[k][k];
                j++;
            }
        B[k][k]=1.0;
        for(int i=k+1;i<n;i++)
        {
            for(int j=k+1;j<n;j++)
                B[i][j]-=B[i][k]*B[k][j];
            B[i][k]=0;
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
	
   printf("%lld.%09lld",dsec,dnsec);

}
void guass_neon_down()
{
    struct timespec sts,ets;
    timespec_get(&sts,TIME_UTC);
    for(int k=0;k<n;k++)
    {
        for(int j=k+1;j<n;j++)
            C[k][j]/=C[k][k];
        C[k][k]=1.0;
        for(int i=k+1;i<n;i++)
        {
            float32x4_t vaik=vdupq_n_f32(C[i][k]);
            int j=k+1;
            for(j=k+1;j+4<=n;j+=4)
            {
                float32x4_t vakj=vld1q_f32(&C[k][j]);
                float32x4_t vaij=vld1q_f32(&C[i][j]);
                float32x4_t vx=vmulq_f32(vakj,vaik);
                vaij=vsubq_f32(vaij,vx);

                vst1q_f32(&C[i][j], vaij);

            }
            while(j<n)
            {
                C[i][j]-=C[k][j]*C[i][k];
                j++;
            }
            C[i][k]=0;

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
   printf("%lld.%09lld",dsec,dnsec);
	
}
void guass_neon()
{
    struct timespec sts,ets;
    timespec_get(&sts,TIME_UTC);
    for(int k=0;k<n;k++)
    {
        float32x4_t vt = vdupq_n_f32(D[k][k]); // 存储的四个 float32 都初始化为A[k][k]
        int j;
        for(j=k+1;j+4<=n;j+=4)
        {
            float32x4_t va=vld1q_f32(&D[k][j]);
            va=vdivq_f32(va,vt);
            vst1q_f32(&D[k][j], va);// 将 q0 中 4 个 float32，赋值给以 d1 为起始地址的 4 个 float32

        }
        while(j<n)
            {
                D[k][j]=D[k][j]/D[k][k];
                j++;
            }
        D[k][k]=1.0;
       for(int i=k+1;i<n;i++)
        {
            float32x4_t vaik=vdupq_n_f32(D[i][k]);
            int j=k+1;
            for(j=k+1;j+4<=n;j+=4)
            {
                float32x4_t vakj=vld1q_f32(&D[k][j]);
                float32x4_t vaij=vld1q_f32(&D[i][j]);
                float32x4_t vx=vmulq_f32(vakj,vaik);
                vaij=vsubq_f32(vaij,vx);

                vst1q_f32(&D[i][j], vaij);

            }
            while(j<n)
            {
                D[i][j]-=D[k][j]*D[i][k];
                j++;
            }
            D[i][k]=0;

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
   printf("%lld.%09lld",dsec,dnsec);
	
}

int main()
{

    while(n<1500)
    {
        m_reset(n);
        
        cout<<n<<"   ";
        guass_normal();

       
        cout<<"     ";
        guass_neon_up();
        cout<<"     ";
       
        guass_neon_down();
        
        cout<<"     ";
        guass_neon();
        cout<<"     ";
        //all_paerll_duiqi();
        cout<<endl;

        
        n+=100;
    }
}