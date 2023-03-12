#include <iostream>
#include<sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include<ctime>
using namespace std;

void  recursion(double *a,int n)
{
    if(n==1)
    {
        return;
    }
    else
    {
        for(int i=0;i<n/2;i++)
        {
            a[i]+=a[n-i-1];
        }
        n/=2;
        recursion(a,n);
    }
}
int main()
{
    int n=8192*8;
    int c=100;
    double* a=new double[n];
    double sum=0.0;
    clock_t head,tail;
    for(int i=0;i<n;i++)
    {
        a[i]=i;
    }
    head = clock();
    for(int j=0;j<c;j++)
    {
    	for(int i=0;i<n;i++)
        {
            sum+=a[i];
        }
    }
    tail=clock();
    cout <<"Direct accumulation: "<<(tail-head)*1000.0/c/CLOCKS_PER_SEC<< "ms" << endl ;


    sum=0.0;
    double sum1=0.0,sum2=0.0;
    head=clock();
    for(int j=0;j<c;j++)
    {
    	for(int i=0;i<n;i+=2)
        {
            sum1+=a[i];
            sum2+=a[i+1];
        }
    }
    tail=clock();
    cout <<"Multi link: "<<(tail-head)*1000.0/c/CLOCKS_PER_SEC<< "ms" << endl ;


    sum=0.0;
    head=clock();
    for(int z=0;z<c;z++)
    {
    	for(int m=n;m>1;m/=2)
        {
            for(int i=0;i<m/2;i++)
            {
                a[i]=a[i*2]+a[i*2 +1];
            }
        }
    }
    sum=a[0];
    tail=clock();
    cout <<"Double cycle: "<<(tail-head)*1000.0/c/CLOCKS_PER_SEC<< "ms" << endl ;


    sum=0.0;
    for(int i=0;i<n;i++)
    {
        a[i]=i;
    }
    head=clock();
    for(int i=0;i<c;i++)
    {
    	recursion(a,n);
    }
    sum=a[0];
    tail=clock();
    cout <<"Recursive function: "<<(tail-head)*1000.0/c/CLOCKS_PER_SEC<< "ms" << endl;
    return 0;
}
