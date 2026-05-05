#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 2048

// initialise Matrix with random numbers
void fill_matrix(double* mat, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        mat[i] = (double)rand() / RAND_MAX;
    }
}

// Matrixmultiplication
void local_matmul(double* A, double* B, double* C, int local_rows) {
    // Naive i-j-k loop order
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0.0;
            for (int k = 0; k < N; k++) {
                // Accessing B[k][j] means jumping by N memory addresses every iteration
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    //your main program here (unoptimized version)

    // Distribute rows across MPI processes
    int local_rows = N / size;
    
    // Allocate memory (flattened 1D arrays for 2D matrices)
    double* A = (double*)malloc(local_rows * N * sizeof(double));
    double* B = (double*)malloc(N * N * sizeof(double));
    double* C = (double*)malloc(local_rows * N * sizeof(double));

    // Initialize matrices with random values
    srand(time(NULL) + rank);
    fill_matrix(A, local_rows, N);
    fill_matrix(B, N, N); // Every rank needs the full B matrix

    // Synchronize before starting the timer
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    // Execute the naive matrix multiplication
    local_matmul(A, B, C, local_rows);

    // Synchronize before stopping the timer
    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    // Find the maximum time taken among all ranks
    double time_taken = end - start;
    double max_time;
    MPI_Reduce(&time_taken, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Report results on Rank 0
    if (rank == 0) {
        // Total floating point operations = 2 * N^3
        double gflops = (2.0 * N * N * N) / (max_time * 1e9);
        printf("--- Naive Implementation ---\n");
        printf("Execution time: %f seconds\n", max_time);
        printf("Achieved Performance: %f GFLOP/s\n", gflops);
    }

    free(A);
    free(B);
    free(C);

    MPI_Finalize();
    return 0;
}
