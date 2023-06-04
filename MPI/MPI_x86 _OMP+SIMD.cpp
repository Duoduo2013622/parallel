#include "mpi.h" 
#include <iostream> 
#include <emmintrin.h>
#include <immintrin.h>
#include<algorithm>
using namespace std;
const int n = 1000, mpisize = 8;
float m1[n][n], time1;
bool parallel = 1;
void printres(int n)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			std::cout << m1[i][j] << " ";
		}
		std::cout << std::endl;
	}
}
int main(int argc, char* argv[])
{
	int rank;
	double st, ed;
	MPI_Status status;
	MPI_Init(0, 0);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//获取当前进程号
	int r1 = rank * (n / mpisize), r2 = (rank == mpisize - 1) ? n - 1 : (rank + 1) * (n / mpisize) - 1;
	if (rank == 0)
	{	//0号进程实现初始化
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				m1[i][j] = 0;
			}
			m1[i][i] = 1.0;
			for (int j = i + 1; j < n; j++)
			{
				m1[i][j] = rand() % 1000 + 1;
			}
		}
		for (int k = 0; k < n; k++)
		{
			for (int i = k + 1; i < n; i++)
			{
				for (int j = 0; j < n; j++)
				{
					m1[i][j] = int((m1[i][j] + m1[k][j])) % 1000 + 1.0;
				}
			}
		}
		for (int j = 1; j < mpisize; j++)
		{
			int t1 = j * (n / mpisize), t2 = (j == mpisize - 1) ? n - 1 : (j + 1) * (n / mpisize) - 1;
			MPI_Send(&m1[t1][0], n * (t2 - t1 + 1), MPI_FLOAT, j, n + 1, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Recv(&m1[r1][0], n * (r2 - r1 + 1), MPI_FLOAT, 0, n + 1, MPI_COMM_WORLD, &status);
	}
	MPI_Barrier(MPI_COMM_WORLD);	//各进程同步
	st = MPI_Wtime();	 //计时
	for (int k = 0; k < n; k++)
	{
		if (rank == 0)
		{	//0号进程负责除法部分
			for (int j = k + 1; j < n; j++)
			{
				m1[k][j] /= m1[k][k];
			}
			m1[k][k] = 1.0;
			for (int j = 1; j < mpisize; j++)
			{
				MPI_Send(&m1[k][0], n, MPI_FLOAT, j, k + 1, MPI_COMM_WORLD);
			}
		}
		else
		{
			MPI_Recv(&m1[k][0], n, MPI_FLOAT, 0, k + 1, MPI_COMM_WORLD, &status);
		}
		if (r2 >= k + 1)
		{		//负责k+1行之后的各进程并发进行减法
			for (int i = max(k + 1, r1); i <= r2; i++)
			{
				for (int j = k + 1; j < n; j++)
				{
					m1[i][j] -= m1[i][k] * m1[k][j];
				}
				m1[i][k] = 0;
				if (i == k + 1 && rank != 0)
				{
					MPI_Send(&m1[i][0], n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);	//减法结果广播回0号进程
				}
			}
		}
		
		if (rank == 0 && k + 1 > r2 && k + 1 < n)//MPI_Barrier(MPI_COMM_WORLD);
		{
			MPI_Recv(&m1[k + 1][0], n, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);	//各进程同步
	ed = MPI_Wtime();	 //计时
	MPI_Finalize();
	if (rank == 0)
	{	//只有0号进程中有最终结果
		printf("cost time:%.4lf\n", ed - st);

	}
	return 0;
}