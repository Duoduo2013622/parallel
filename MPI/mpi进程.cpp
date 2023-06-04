#include <iostream>
#include <cstdlib>  
#include <iomanip> 
#include <algorithm>  
#include <sys/time.h>
#include <mpi.h>


int main(int argc, char* argv[])
{
int rank;
double st, ed;
MPI_Status status;
MPI_Init(0, 0);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);^^I//获取当前进程编号
int r1 = rank * (n / mpi_num), r2 = (rank == mpi_num - 1) ? n - 1 :
(rank + 1)*(n / mpi_num) - 1;
if (rank == 0) //0 号进程进行初始化
{
for (int i = 0; i < n; i++)
{
for (int j = 0; j < n; j++)
data[i][j] = 0;
data[i][i] = 1.0;
for (int j = i + 1; j < n; j++)
data[i][j] = i+j;
}
for (int k = 0; k < n; k++)
for (int i = k + 1; i < n; i++)
for (int j = 0; j < n; j++)
data[i][j] = int((data[i][j] + data[k][j])) ;
for (int j = 1; j < mpi_num; j++)
{
int t1 = j * (n / mpi_num), t2 = (j == mpi_num - 1) ? n - 1 :
(j + 1)*(n / mpi_num) - 1;
MPI_Send(&data[t1][0], n*(t2-t1+1), MPI_FLOAT, j, n + 1, MPI_COMM_WORLD);
}
}
else
MPI_Recv(&data[r1][0], n*(r2-r1+1), MPI_FLOAT,0, n+1, MPI_COMM_WORLD, &status);
MPI_Barrier(MPI_COMM_WORLD);//进程同步
st = MPI_Wtime();//计时
for (int k = 0; k < n; k++)
{
if (rank == 0) //0 号进程负责除法部分
{
for (int j = k + 1; j < n; j++)
data[k][j] /= data[k][k];
data[k][k] = 1.0;
for (int j = 1; j < mpi_num; j++)
43 MPI_Send(&data[k][0], n, MPI_FLOAT, j, k + 1, MPI_COMM_WORLD);//广播到各进程
44 }
45 else
46 MPI_Recv(&data[k][0], n, MPI_FLOAT, 0, k + 1, MPI_COMM_WORLD, &status);
47 if (r2 >= k + 1) //负责 k+1 行之后的各进程并发进行减法
48 {
49 for (int i = max(k + 1, r1); i <= r2; i++)
50 {
51 for (int j = k + 1; j < n; j++)
52 data[i][j] -= data[i][k] * data[k][j];
53 data[i][k] = 0;
54 if (i == k + 1 && rank != 0)
55 //减法结果广播回 0 号进程
56 MPI_Send(&data[i][0], n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
57 }
58 }
59 if (rank == 0 && k + 1 > r2&&k+1<n)
60 MPI_Recv(&data[k + 1][0], n, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
61 }
62 MPI_Barrier(MPI_COMM_WORLD);//各进程同步
63 ed = MPI_Wtime();//计时结束
64 MPI_Finalize();
65 if (rank == 0)
66 {
67 printf("cost time:%.4lf\n", ed - st);//输出时间
68 }
69 return 0;
70 }