#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 2048

// initialise Matrix with random numbers
void fill_matrix(double* mat, int rows, int cols) {
}

// Matrixmultiplication
void local_matmul(double* A, double* B, double* C, int local_rows) {

}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    //your main program here (unoptimized version)

    MPI_Finalize();
    return 0;
}
