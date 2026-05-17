/**
 * HPDC SS 2026 - Exercise 3, Part 1
 * Matrix Multiply - Parallel MPI Version
 *
 * Task:
 *   Implement a parallel MPI program performing matrix multiplication C = A * B
 *   for square matrices with dynamic memory allocation.
 *
 * Requirements:
 *   - Dynamically allocate all matrices
 *   - Initialize using: A[i,j] = i+j,  B[i,j] = i*j
 *   - The MASTER rank (rank 0) performs all initialization
 *   - Broadcast A and B to all ranks (this IS part of measured time)
 *   - Do NOT distribute C; assume C is initialized to 0 everywhere
 *   - Measure execution time, excluding initialization
 *   - Verify that parallel result matches the sequential result
 *   - Run with 2 worker ranks and print C for a 5x5 matrix
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>
#include <algorithm>

// TODO: Add any additional includes you need


// ---------------------------------------------------------------------------
// Sequential matrix multiply (reference implementation for verification)
// C = A * B, all matrices are N x N stored in row-major order
// ---------------------------------------------------------------------------
void seq_matmul(const double* A, const double* B, double* C, int N) {
    // TODO: Implement sequential matrix multiply for verification
    // Zero C
    std::fill(C, C + (size_t)N * N, 0.0);
    for (int i = 0; i < N; ++i) {
        for (int k = 0; k < N; ++k) {
            double a = A[i * N + k];
            for (int j = 0; j < N; ++j) {
                C[i * N + j] += a * B[k * N + j];
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Initialize matrices according to the exercise specification:
//   A[i,j] = i + j
//   B[i,j] = i * j
// Both are N x N, row-major.
// This must be called ONLY by the master rank (rank 0).
// ---------------------------------------------------------------------------
void init_matrices(double* A, double* B, int N) {
    // TODO: Implement matrix initialization
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = static_cast<double>(i + j);
            B[i * N + j] = static_cast<double>(i * j);
        }
    }
}


// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Problem size: change N here or accept it via CLI
    int N = 5;

    // Allocation
    // TODO: Dynamically allocate A, B, and C (all N*N doubles)
    // Remember: ALL ranks need storage for A, B, and C.
    //           Only rank 0 initializes A and B.

    double* A = (double*)malloc(sizeof(double) * (size_t)N * N);
    double* B = (double*)malloc(sizeof(double) * (size_t)N * N);
    double* C = (double*)malloc(sizeof(double) * (size_t)N * N);
    if (!A || !B || !C) {
        fprintf(stderr, "Rank %d: allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Initialize C to 0 everywhere
    std::fill(C, C + (size_t)N * N, 0.0);

    // Initialization (master only, NOT timed)
    if (rank == 0) {
        init_matrices(A, B, N);
    }

    // Start timing
    // TODO: Record start time with MPI_Wtime() after initialization

    MPI_Barrier(MPI_COMM_WORLD); // ensure all ranks ready
    double t_start = MPI_Wtime();

    // Broadcast A and B from master to all ranks
    // (Broadcast is part of measured time, as specified)
    // TODO: MPI_Bcast A and B

    MPI_Bcast(A, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Parallel matrix multiply
    // Hint: Distribute rows of A among ranks. Each rank computes
    //       its local rows of C using the full B matrix.
    // TODO: Compute local_rows, determine offset, multiply your rows

    int base_rows = N / size;
    int remainder = N % size;
    int local_rows = base_rows + (rank < remainder ? 1 : 0);
    int start_row = rank * base_rows + std::min(rank, remainder);

    // Compute local rows
    for (int ii = 0; ii < local_rows; ++ii) {
        int i = start_row + ii;
        // Compute row i of C
        for (int k = 0; k < N; ++k) {
            double a = A[i * N + k];
            for (int j = 0; j < N; ++j) {
                C[ii * N + j] += a * B[k * N + j]; // store local rows at beginning of C buffer
            }
        }
    }


    // Gather C back to master (or leave distributed)
    // NOTE: You do NOT need to distribute C further; it is assumed 0 initially.
    // TODO: (Optional) MPI_Gather or MPI_Reduce the result to rank 0 for printing

    int* recvcounts = nullptr;
    int* displs = nullptr;
    double* C_full = nullptr; // only master will keep full matrix
    int sendcount = local_rows * N;
    if (rank == 0) {
        recvcounts = (int*)malloc(sizeof(int) * size);
        displs = (int*)malloc(sizeof(int) * size);
        if (!recvcounts || !displs) {
            fprintf(stderr, "Master allocation failed for recvcounts/displs\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        int offset = 0;
        for (int r = 0; r < size; ++r) {
            int rows_r = base_rows + (r < remainder ? 1 : 0);
            recvcounts[r] = rows_r * N;
            displs[r] = offset;
            offset += recvcounts[r];
        }
        C_full = (double*)malloc(sizeof(double) * (size_t)N * N);
        if (!C_full) {
            fprintf(stderr, "Master allocation failed for C_full\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        // initialize to zero (just in case)
        std::fill(C_full, C_full + (size_t)N * N, 0.0);
    }

    // Note: Each rank currently wrote its local rows starting at C[0..local_rows*N-1]
    // Use Gatherv to collect local blocks to master
    MPI_Gatherv(C, sendcount, MPI_DOUBLE,
                C_full, recvcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    // Stop timing
    // TODO: Record end time and compute elapsed time on rank 0

    double t_end = MPI_Wtime();
    double elapsed = t_end - t_start;

    // Verify result (compare parallel C with sequential C on rank 0)
    if (rank == 0) {
        // TODO: Compute sequential reference result and compare with parallel C
        // Print PASS or FAIL
            double* C_ref = (double*)malloc(sizeof(double) * (size_t)N * N);
        if (!C_ref) {
            fprintf(stderr, "Master allocation failed for C_ref\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        seq_matmul(A, B, C_ref, N);

        // Compare C_full with C_ref
        int mismatches = 0;
        const double tol = 1e-9;
        for (int i = 0; i < N * N; ++i) {
            if (std::fabs(C_full[i] - C_ref[i]) > tol) mismatches++;
        }

        if (mismatches == 0) {
            std::cout << "Verification: PASS (parallel result matches sequential reference)\n";
        } else {
            std::cout << "Verification: FAIL (" << mismatches << " mismatches)\n";
        }

        free(C_ref);
    }  


    // Print C for 5x5 case (rank 0 only)
    if (rank == 0 && N == 5) {
        std::cout << "\nResult matrix C (5x5):\n";
        // TODO: Print C in a readable format (rows C[0..N-1])
        for (int i = 0; i < N; ++i) {
            std::cout << "C[" << i << ",*] =";
            for (int j = 0; j < N; ++j) {
                // values are integral for this initialization; print without decimals
                long long v = llround(C_full[i * N + j]);
                std::cout << " " << v;
            }
            std::cout << "\n";
        }
    }


    // Print timing
    if (rank == 0) {
        // TODO: Print elapsed time in seconds
        std::cout << "\nElapsed time (includes broadcast and multiply, excludes master-only initialization): "
                  << elapsed << " seconds\n";
    }


    // Free memory
    // TODO: Free A, B, C
    free(A);
    free(B);
    free(C);
    if (rank == 0) {
        free(C_full);
        free(recvcounts);
        free(displs);
    }


    MPI_Finalize();
    return 0;
}
