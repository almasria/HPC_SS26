/* HPDC SS 2026, Exercise 5 - Ring Allreduce */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // TODO: Implement Ring Allreduce (Ring Reduce-Scatter + Ring Allgather)

    MPI_Finalize();
    return 0;
}
