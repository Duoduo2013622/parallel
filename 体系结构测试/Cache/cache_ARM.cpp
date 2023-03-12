#include <iostream>
#include<sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include<ctime>
using namespace std;

int main()
{
    int n=5000;
    int c=100;
    double** b=new double*[n];
    clock_t head,tail;
    for(int i=0;i<n;i++)
    {
        b[i]=new double[n];
    }
    double* a=new double[n];
    double* sum=new double[n];
    for(int i=0;i<n;i++)
    {
        a[i]=i;
        for(int j=0;j<n;j++)
        {
            b[i][j]=i+j;
        }
    }
    head=clock();
    for(int z=0;z<c;z++)
    {
    	for(int i=0;i<n;i++)
    	{
            sum[i]=0.0;
            for(int j=0;j<n;j++)
            {
                sum[i]+=b[j][i]*a[j];
            }
        }
    }
    tail=clock();
    cout <<"Col: "<<(tail-head)/1000.0/c<< "ms" << endl ;



    for(int i=0;i<n;i++)
    {
        sum[i]=0.0;
    }
    head = clock();
    for(int z=0;z<c;z++)
    {
    	for(int j=0;j<n;j++)
        {
            for(int i=0;i<n;i++)
            {
                sum[i]+=b[j][i]*a[j];
            }
        }
    }
    tail=clock();
    cout <<"Row: "<<(tail-head)/1000.0/c<< "ms" << endl ;
    return 0;
}
