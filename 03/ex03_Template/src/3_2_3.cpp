/**
 * HPDC SS 2026 - Exercise 3, Part 2 & 3
 * Matrix Multiply - Scaling Benchmarks
 *
 * This is the same MPI parallel matrix multiply program as in Part 1,
 * extended for benchmarking purposes:
 *
 * Part 2 - Scaling Ranks (Section 3.2):
 *   Execute with 2048x2048 matrices on varying node/rank configurations.
 *   Compare with your sequential (optimized) baseline from Exercise 2.
 *   Report execution time, speed-up, and efficiency.
 *   Use the provided SLURM scripts (3_2_1node.sh, 3_2_2nodes.sh, 3_2_4nodes.sh).
 *
 * Part 3 - Scaling Problem Size (Section 3.3):
 *   Execute with 12 and 24 ranks (distributed evenly on 2 nodes).
 *   Vary N from 128 to 16384 (powers of two).
 *   Compute and report GFLOP/s.
 *   Use the provided SLURM script (3_3.sh).
 *
 * CLI parameters (parsed with chCommandLine.h):
 *   --size <N>     Matrix dimension (default: 2048)
 *
 * Output format expected by the .sh scripts:
 *   "Time: <seconds>"
 *
 * Note: Adapt your implementation from 3_1.cpp; the core MPI logic is the same.
 *       You only need to accept N via CLI and output the timing in the required format.
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chCommandLine.h>

// TODO: Add any additional includes you need


// ---------------------------------------------------------------------------
// Sequential matrix multiply (for speed-up comparison in Part 2)
// ---------------------------------------------------------------------------
void seq_matmul(const double* A, const double* B, double* C, int N) {
    // TODO: Implement (can reuse from 3_1.cpp)
}


// ---------------------------------------------------------------------------
// Initialize matrices: A[i,j] = i+j,  B[i,j] = i*j
// Called only on master rank.
// ---------------------------------------------------------------------------
void init_matrices(double* A, double* B, int N) {
    // TODO: Implement (can reuse from 3_1.cpp)
}


// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Parse problem size from CLI
    int N = 2048; // default for Part 2
    chCommandLineGet<int>(&N, "size", argc, argv);

    // Allocation
    // TODO: Dynamically allocate A, B, C


    // Initialization (master only, NOT timed)
    if (rank == 0) {
        init_matrices(/* TODO */);
    }


    // Start timing (after initialization)
    double t_start = MPI_Wtime();


    // Broadcast A and B
    // TODO: MPI_Bcast A and B


    // Parallel matrix multiply
    // TODO: Distribute rows, compute local result


    // Gather / reduce C to master
    // TODO: Collect partial results


    // Stop timing
    double t_end = MPI_Wtime();


    // Output (rank 0 only)
    // IMPORTANT: Keep this output format – the .sh scripts parse it.
    if (rank == 0) {
        double elapsed = t_end - t_start;
        std::cout << "Time: " << elapsed << std::endl;

        // TODO (Part 3 only): also compute and print GFLOP/s
        // FLOPS for N x N matrix multiply = 2 * N^3
        // double flops = 2.0 * (double)N * N * N;
        // double gflops = (flops / elapsed) / 1e9;
        // std::cout << "GFLOP/s: " << gflops << std::endl;
    }


    // Free memory
    // TODO: Free A, B, C


    MPI_Finalize();
    return 0;
}
