#include <iostream>
#include <cstdlib>  
#include <iomanip> 
#include <algorithm>  
#include <sys/time.h>
#include <mpi.h>
using namespace std;
const int N=2048;
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
    int m = MPI::COMM_WORLD.Get_size();  // 总mpi节点数
    int q = N % m;
    int _n = (N - q) / m;
    int matrix_size;  
    if(rank < q && q != 0)
        matrix_size = _n+1;
    else
        matrix_size = _n;
        
    // 该节点负责计算的行号
    float **mpi_line = new float*[matrix_size];
    for(int i=0; i<matrix_size; i++)
        mpi_line[i] = new float[N];
    float *row_k = new float[N];
	if(rank==0)
	{
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

        int count = 0;
        for(int i = 0; i<N; i++)
        {
            int dest = i % m;
            if(dest != rank)
                MPI_Send(A[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
            else{
                // 自己负责的行移至mpi_line
                for(int j = 0; j<N; j++)
                    mpi_line[count][j] = A[i][j];
                count ++;
            }
        }
        for(int i=0; i<matrix_size; i++)
        {
            if(i==0){//接收
                for(int k=0; k<rank; k++){
                    int src = k%m;
                    MPI_Recv(row_k, N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // 消去
                    matrix_elimination(mpi_line, matrix_size, row_k, N, k, i);
                }
            }
            else{
                for(int j=1; j<=m-1;j++){
                    int k = m *(i-1) + j + rank;
                    int src = k%m;
                    MPI_Recv(row_k, N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // 消去
                    matrix_elimination(mpi_line, matrix_size, row_k, N, k, i);
                }
            }
            // 除法
            int k = m *i + rank;
            for(int j=k+1; j<N; j++)
                mpi_line[i][j] = mpi_line[i][j] / mpi_line[i][k];
            mpi_line[i][k] = 1.0;
            matrix_elimination(mpi_line, matrix_size, mpi_line[i], N, k, i+1);

            int end_row = N-1;
            int real_row = i*m + rank;
            if((i == matrix_size-1) && (end_row-m+2 <=real_row) &&(real_row <= end_row))
            {
                for(int row=real_row+1; row<=end_row; row++){
                	int dest = row % m;
                    if(dest != rank)
                        MPI_Send(mpi_line[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
                }
            }
            else
            {
                for(int dest=0; dest<m; dest++)
                    if(dest != rank) 
                        MPI_Send(mpi_line[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
            }

        }
        
        count = 0;
        for(int i = 0; i<N; i++)
        {
            int src = i % m;
            if(rank != src){
                MPI_Recv(A[i], N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else{
                for(int j = 0; j<N; j++)
                    A[i][j] =  mpi_line[count][j];
                count ++;
            }
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
        // 从第0节点接收原始数据
    	for(int i=0; i<matrix_size; i++)
        {
            MPI_Recv(mpi_line[i], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for(int i=0; i<matrix_size; i++)
        {
            if(i==0){
                for(int k=0; k<rank; k++){
                    int src = k%m;
                    MPI_Recv(row_k, N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // 消去
                    matrix_elimination(mpi_line, matrix_size, row_k, N, k, i);
                }
            }
            else{
                for(int j=1; j<=m-1;j++){
                    int k = m *(i-1) + j + rank;
                    int src = k%m;
                    MPI_Recv(row_k, N, MPI_FLOAT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // 消去
                    matrix_elimination(mpi_line, matrix_size, row_k, N, k, i);
                }
            }

            // 除法
            int k = m *i + rank;
            for(int j=k+1; j<N; j++)
                mpi_line[i][j] = mpi_line[i][j] / mpi_line[i][k];
            mpi_line[i][k] = 1.0;
            // 消去
            matrix_elimination(mpi_line, matrix_size, mpi_line[i], N, k, i+1);
            int end_row = N-1;
            int real_row = i*m + rank;
            if((i == matrix_size-1) && (end_row-m+2 <=real_row) &&(real_row <= end_row))
            {
                for(int row=real_row+1; row<=end_row; row++){
                	int dest = row % m;
                    if(dest!=rank)
                        MPI_Send(mpi_line[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
                }
            }
            else
            {
                for(int dest=0; dest<m; dest++)
                    if(dest != rank) 
                        MPI_Send(mpi_line[i], N, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
            }

        }
        for(int i = 0; i<matrix_size; i++)
        {
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