#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 2048

void init_A(double *A)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            A[i * N + j] = i + j;
}

void init_B(double *B)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            B[i * N + j] = i * j;
}

void matmul_seq(double *A, double *B, double *C)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
            C[i * N + j] = 0.0;
            for (int k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
        }
}

void matmul_local(double *A, double *B, double *C, int rows)
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < N; j++)
        {
            C[i * N + j] = 0.0;
            for (int k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
        }
}

int main(int argc, char **argv)
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // ---- SCATTERV SETUP ----
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));

    int base = N / size;
    int rem = N % size;

    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        sendcounts[i] = (base + (i < rem ? 1 : 0)) * N;
        displs[i] = offset;
        offset += sendcounts[i];
    }

    int local_rows = sendcounts[rank] / N;

    double *local_A = (double *)malloc(local_rows * N * sizeof(double));
    double *local_C = (double *)malloc(local_rows * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));

    double *A = NULL, *C = NULL, *C_seq = NULL;

    if (rank == 0)
    {
        A = (double *)malloc(N * N * sizeof(double));
        C = (double *)malloc(N * N * sizeof(double));
        C_seq = (double *)malloc(N * N * sizeof(double));

        init_A(A);
        init_B(B);
    }

    // ---- DISTRIBUTE A ----
    MPI_Scatterv(A, sendcounts, displs, MPI_DOUBLE,
                 local_A, sendcounts[rank], MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    matmul_local(local_A, B, local_C, local_rows);

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();

    // ---- GATHERV RESULT ----
    MPI_Gatherv(local_C, sendcounts[rank], MPI_DOUBLE,
                C, sendcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        matmul_seq(A, B, C_seq);

        int correct = 1;
        for (int i = 0; i < N * N; i++)
        {
            if (fabs(C[i] - C_seq[i]) > 1e-6)
            {
                correct = 0;
                break;
            }
        }

        printf("Execution time: %f seconds\n", end - start);
        printf("Verification: %s\n", correct ? "SUCCESS" : "FAILED");
    }

    free(local_A);
    free(local_C);
    free(B);
    free(sendcounts);
    free(displs);

    if (rank == 0)
    {
        free(A);
        free(C);
        free(C_seq);
    }

    MPI_Finalize();
    return 0;
}