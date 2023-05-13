#include <iostream>
#include <windows.h>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <omp.h>  // OpenMP 必带头文件
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
#include <smmintrin.h> //SSE4.1
#include <nmmintrin.h> //SSSE4.2
#define MAX_THREADS 7
using namespace std;
int n = 2000;
int max_n = 2000;
float** A;
float** M;
int threadcount = 7;

void m_reset()
{

	A = new float* [n];
	M = new float* [n];
	for (int i = 0; i < n; i++)
	{
		A[i] = new float[n];
		M[i] = new float[n];
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < i; j++)
		{
			A[i][j] = 0;
			M[i][j] = 0;
		}
		A[i][i] = 1.0;
		M[i][i] = 1.0;
		for (int j = i + 1; j < n; j++)
		{
			A[i][j] = i + j;
			M[i][j] = A[i][j];
		}

	}

	for (int k = 0; k < n; k++)
	{

		for (int i = k + 1; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				A[i][j] += A[k][j];
				M[i][j] = A[i][j];
			}

		}
	}


}
void guass_swap()
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			A[i][j] = M[i][j];

		}

	}

}
void guass_normal(ofstream& result)
{
	long long sum = 0;



	long long head, tail, freq; // timers
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&head);
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
	QueryPerformanceCounter((LARGE_INTEGER*)&tail);

	result << (tail - head) * 1000.0 / freq << " ";;

}
void omp_row()
{

	omp_set_num_threads(MAX_THREADS);
#pragma omp parallel
	{


		for (int k = 0; k < n; k++)
		{

		#pragma omp single
			{
				int temp = A[k][k];
				for (int j = k + 1; j < n; j++)
				{
					A[k][j] = A[k][j] / temp;
				}
				A[k][k] = 1.0;
			}

			#pragma omp for schedule(static, MAX_THREADS)
			for (int i = k + 1; i < n; i++)
			{
				int temp = A[i][k];
				__m128 vaik = _mm_set1_ps(A[i][k]);
				int j = k + 1;
				for (j = k + 1; j + 4 <= n; j += 4)
				{
					__m128 vakj = _mm_loadu_ps(&A[k][j]);
					__m128 vaij = _mm_loadu_ps(&A[i][j]);
					__m128 vx = _mm_mul_ps(vakj, vaik);
					vaij = _mm_sub_ps(vaij, vx);

					_mm_storeu_ps(&A[i][j], vaij);

				}
				while (j < n)
				{
					A[i][j] -= A[k][j] * temp;
					j++;
				}
				A[i][k] = 0;

			}
		}

	}
}
int main()
{

	ofstream result("C:\\Users\\86153\\Desktop\\openMP\\openMP_3.txt");
	while (n <= max_n)
	{
		cout << n << endl;
		m_reset();






		long long head, tail, freq; // timers
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		QueryPerformanceCounter((LARGE_INTEGER*)&head); //开始计时
		omp_row();
		QueryPerformanceCounter((LARGE_INTEGER*)&tail);
		result << (tail - head) * 1000.0 / freq << endl;

		n = n + 100;
	}
	return 0;
}
