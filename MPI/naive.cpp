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

int main(int argc, char ** argv)
{
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    float **A =new float*[N];
    for(int i=0; i<N; i++)
        A[i] = new float[N];
    matrix_initialize(A, N, 100);
    float **A_copy = new float*[N];
    for(int i=0; i<N; i++)
       A_copy[i] = new float[N];
    copy_matrix(A_copy, A, N);
    // gettimeofday(&start, NULL);//精确到微秒
    start_time = MPI_Wtime();
    naive_lu(A_copy, N);
    // gettimeofday(&end, NULL);
    end_time = MPI_Wtime();
	show_matrix(A, N);
    cout << "N = "<<N<< endl;
    cout << "time is " << (end_time - start_time)*1000 <<"ms"<< endl;
    MPI_Finalize();
    // timersub(&end, &start, &diff);
    // timecost= diff.tv_sec + (1.0 * diff.tv_usec)/1000;
	// show_matrix(A_copy, N);
    // cout << "time is " << timecost << "ms" <<endl;
}



