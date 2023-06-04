#include <iostream>
#include <cstdlib>  
#include <iomanip> 
#include <algorithm>  
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
using namespace std;
const int N=2048;
const int Threads=4;
double start_time=0,end_time=0;
void matrix_initialize(float **Matrix, int N, int scale)
{
    srand((unsigned)time(NULL));  
    for(int i = 0; i<N; i++)
        for(int j = 0; j<N; j++)
                Matrix[i][j] = rand()%scale;  
}
  
void copy_matrix(float** dst, float** src, int N)
{
    for(int i = 0; i<N; i++)
        for(int j = 0; j<N; j++)
            dst[i][j] = src[i][j];
}

bool is_same(float **a, float**b, int N)
{
    for(int i = 0; i<N; i++)
        for(int j = 0; j<N; j++)
            if(a[i][j] != b[i][j])
                return false;
    return true;
}

void show_matrix(float **Matrix, int N)
{
    if(N>16)
        return;
    for(int i = 0; i<N; i++)
        {
            for(int j = 0; j<N; j++){
                   cout << fixed<< setprecision(1)<< setw(6)<< right<< Matrix[i][j];
            }
            cout << endl;
        }
    cout << endl;
}

void naive_lu(float **Matrix, int N)
{
    for(int k = 0; k<N; k++)
    {
        for(int j = k+1; j<N; j++)
            Matrix[k][j] = Matrix[k][j] / Matrix[k][k];
        Matrix[k][k] = 1.0;
        for(int i = k+1; i<N; i++){
            for(int j = k+1; j<N; j++)
                Matrix[i][j] = Matrix[i][j] - Matrix[i][k] * Matrix[k][j];
            Matrix[i][k] = 0;
        }
    }
}
//余下矩阵消去
void matrix_elimination(float **matrix, int n_row, float *row_k, int dimension, int k, int row_No/**/)
{
    #pragma omp parallel for num_threads(Threads)
    for(int i=row_No; i<n_row; i++)
    {
        for(int j=k+1; j<dimension; j++)
            matrix[i][j] = matrix[i][j] - matrix[i][k]*row_k[j];
        matrix[i][k] = 0.0;
    }
}
//mpi thread
void mpi_thread(int rank)
{
	char *proc_name = new char[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(proc_name, &name_len);
	if(rank==0)
	{
        int m = MPI::COMM_WORLD.Get_size();  // m是总的mpi节点个数
        int _n = (N - N%m) / m;
		//  0线程创建矩阵并赋初值
		float **A = new float*[N];
		for(int i=0; i<N; i++)
			A[i] = new float[N];
		matrix_initialize(A, N, 100);

        float **A_copy = new float*[N];
        for(int i=0; i<N; i++)
			A_copy[i] = new float[N];

        copy_matrix(A_copy, A, N);
        //朴素算法验证结果
        naive_lu(A_copy, N);

        //计时开始
        start_time = MPI_Wtime();

        for(int i = _n; i<N; i++)
        {
            int dest = i / _n;
            if(m == dest)
                dest--;
            if(dest != 0)
                MPI_Send(A[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);

        }
        for(int k=0; k < _n; k++)
        {
            // division
            for(int j=k+1; j<N; j++)
                A[k][j] = A[k][j] / A[k][k];
            A[k][k] = 1.0;
            int dest = rank + 1;
            if(dest <= m-1)
            	MPI_Send(A[k], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
            int rows_left = _n-1 - k;
            if(rows_left <= 0)
                break; 
            matrix_elimination(A, _n, A[k], N, k, k+1);    
        }
        for(int i=_n; i<N; i++)
        {
            int src = i/_n;
            if(src == m)
                src--;
            MPI_Recv(A[i], N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        end_time = MPI_Wtime();
		show_matrix(A, N);
        cout << "N = "<<N<<". Number of Process: " << m << endl;
        cout << "time is " << (end_time - start_time)*1000 <<"ms"<< endl;
        if(!is_same(A, A_copy, N))
        	cout << "wrong!!" << endl;
        else
        	cout << "correct" << endl;
        
	}
    else
    {
        int m = MPI::COMM_WORLD.Get_size(); 
        int _n = (N - N%m) / m;
        int begin_row, end_row;  
        begin_row = rank * _n;
        if(rank == m-1)
            end_row = N-1; 
        else
            end_row = rank * _n + _n-1;
        
        int matrix_size = end_row - begin_row + 1;
        float **mpi_line = new float*[matrix_size];
        for(int i=0; i<matrix_size; i++)
            mpi_line[i] = new float[N];

        for(int i=0; i<matrix_size; i++)
        {
            MPI_Recv(mpi_line[i], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        float *row_k = new float[N]; 
        for(int k=0; k<begin_row; k++)
        {
            int src = rank - 1;

            MPI_Recv(row_k, N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            matrix_elimination(mpi_line, matrix_size, row_k, N, k, 0);
            int dest = rank+1;
            if(dest <= m-1){
                MPI_Send(row_k, N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
            }
        }

        for(int k=begin_row; k <= end_row; k++)
        {
            int i = k - begin_row;
            for(int j=k+1; j<N; j++)
                mpi_line[i][j] = mpi_line[i][j] / mpi_line[i][k];
            mpi_line[i][k] = 1.0;

            if(rank != m-1){
                int dest = rank + 1;
                if(dest <= m-1)
                    MPI_Send(mpi_line[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
            }
            int rows_left = end_row - k;
            if(rows_left <= 0)
                break;
            
            matrix_elimination(mpi_line, matrix_size, mpi_line[i], N, k, i+1);
            
        }
        for(int i=0; i<matrix_size; i++){
            MPI_Send(mpi_line[i], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
        }
    }


}

int main(int argc, char ** argv)
{
    int rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    mpi_thread(rank);
    
    MPI_Finalize();
    return 0;
}