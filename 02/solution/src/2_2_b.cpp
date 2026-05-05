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
    // Initialize C to 0
    for (int i = 0; i < local_rows * N; i++) {
        C[i] = 0.0;
    }

    // Optimized i-k-j loop order
    for (int i = 0; i < local_rows; i++) {
        for (int k = 0; k < N; k++) {
            // Cache the value of A for the inner loop
            double r = A[i * N + k];
            for (int j = 0; j < N; j++) {
                // B is now accessed sequentially, utilizing CPU Cache perfectly
                C[i * N + j] += r * B[k * N + j];
            }
        }
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    //your main program here (optimized version)

    // Distribute rows across MPI processes
    int local_rows = N / size;
    
    // Allocate memory 
    double* A = (double*)malloc(local_rows * N * sizeof(double));
    double* B = (double*)malloc(N * N * sizeof(double));
    double* C = (double*)malloc(local_rows * N * sizeof(double));

    srand(time(NULL) + rank);
    fill_matrix(A, local_rows, N);
    fill_matrix(B, N, N); 

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    // Execute the optimized matrix multiplication
    local_matmul(A, B, C, local_rows);

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    double time_taken = end - start;
    double max_time;
    MPI_Reduce(&time_taken, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double gflops = (2.0 * N * N * N) / (max_time * 1e9);
        printf("--- Optimized Implementation ---\n");
        printf("Execution time: %f seconds\n", max_time);
        printf("Achieved Performance: %f GFLOP/s\n", gflops);
    }

    free(A);
    free(B);
    free(C);

    MPI_Finalize();
    return 0;
}
