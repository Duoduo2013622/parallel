#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include <iomanip>


using namespace std;

int main() {
    // Matrix dimensions
    int M = 1024;  // Number of rows in matrix A
    int N = 1024;  // Number of columns in matrix B
    int K = 1024;  // Number of columns in matrix A and rows in matrix B

    // Initialize matrices A, B, and C
    float *A = new float[M * K];
    float *B = new float[K * N];
    float *C = new float[M * N];
    for (int i = 0; i < M * K; i++) {
        A[i] = random_float();
    }
    for (int i = 0; i < K * N; i++) {
        B[i] = random_float();
    }

    // Create SYCL queue
    queue q;

    // Test different tile sizes
    for (int tileX = 2; tileX <= 16; tileX += 2) {
        for (int tileY = 2; tileY <= 16; tileY += 2) {
            // Copy input matrices to the device
            buffer<float, 1> bufferA(A, range<1>(M * K));
            buffer<float, 1> bufferB(B, range<1>(K * N));
            buffer<float, 1> bufferC(C, range<1>(M * N));

            // Compute matrix multiplication on GPU
            double gpu_time = gpu_kernel(bufferA.get_access<access::mode::read>(q),
                                         bufferB.get_access<access::mode::read>(q),
                                         bufferC.get_access<access::mode::write>(q),
                                         M, N, K, tileX, tileY, q);

            // Compute matrix multiplication on CPU for comparison
            float *cpuC = new float[M * N];
            double cpu_time = cpu_kernel(A, B, cpuC, M, N, K);

            // Verify results
            int errors = verify(cpuC, bufferC.get_access<access::mode::read>(), M * N);
            if (errors > 0) {
                cout << "ERROR: Results do not match" << endl;
            }

            // Print results
            cout << "Tile Size (tileX, tileY): (" << tileX << ", " << tileY << ")" << endl;
            cout << "GPU Execution Time: " << gpu_time << " milliseconds" << endl;
            cout << "CPU Execution Time: " << cpu_time << " milliseconds" << endl;
            cout << endl;

            // Cleanup
            delete[] cpuC;
        }
    }

    // Cleanup
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}