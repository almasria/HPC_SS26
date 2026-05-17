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

// TODO: Add any additional includes you need


// ---------------------------------------------------------------------------
// Sequential matrix multiply (reference implementation for verification)
// C = A * B, all matrices are N x N stored in row-major order
// ---------------------------------------------------------------------------
void seq_matmul(const double* A, const double* B, double* C, int N) {
    // TODO: Implement sequential matrix multiply for verification
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


    // Initialization (master only, NOT timed)
    if (rank == 0) {
        init_matrices(/* TODO: pass A, B, N */);
    }


    // Start timing
    // TODO: Record start time with MPI_Wtime() after initialization


    // Broadcast A and B from master to all ranks
    // (Broadcast is part of measured time, as specified)
    // TODO: MPI_Bcast A and B


    // Parallel matrix multiply
    // Hint: Distribute rows of A among ranks. Each rank computes
    //       its local rows of C using the full B matrix.
    // TODO: Compute local_rows, determine offset, multiply your rows


    // Gather C back to master (or leave distributed)
    // NOTE: You do NOT need to distribute C further; it is assumed 0 initially.
    // TODO: (Optional) MPI_Gather or MPI_Reduce the result to rank 0 for printing


    // Stop timing
    // TODO: Record end time and compute elapsed time on rank 0


    // Verify result (compare parallel C with sequential C on rank 0)
    if (rank == 0) {
        // TODO: Compute sequential reference result and compare with parallel C
        // Print PASS or FAIL
    }


    // Print C for 5x5 case (rank 0 only)
    if (rank == 0 && N == 5) {
        std::cout << "\nResult matrix C (5x5):\n";
        // TODO: Print C in a readable format (rows C[0..N-1])
    }


    // Print timing
    if (rank == 0) {
        // TODO: Print elapsed time in seconds
    }


    // Free memory
    // TODO: Free A, B, C


    MPI_Finalize();
    return 0;
}
