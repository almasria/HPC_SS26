#include <cmath>
#include <iostream>
#include <cstdlib>
#include <chCommandLine.h>
#include <chTimer.hpp>
#include <mpi.h>


// customBarrier implement a barrier
void customBarrier(int rank, int iterations, int size) {

	double starttime = MPI_Wtime();
	double endtime;
	
	// Output result on last rank: This rank is expected to receive the release last and should therefore be the correct time when the barrier was fully released
	//This cout will then be used in the .sh to collect the measurements in the .csv
	if (rank == size-1) std::cout << "Custom Barrier Time: " << ((endtime-starttime) / ((double) iterations / 1000)) << " ms" << std::endl;
}

// use for build in Barrier
void builtInBarrier(int rank, int iterations, int size) {
	double starttime;
	double endtime;

	//This cout will then be used in the .sh to collect the measurements in the .csv
	if (rank == size-1) std::cout << "Built-In Barrier Time: " << ((endtime-starttime) / ((double) iterations / 1000)) << " ms" << std::endl;
}

//
// Main
//
int main(int argc, char * argv[]) {
	MPI_Init(&argc , &argv); // Initialize MPI

	// Get global rank and size
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Implement your test

	
	MPI_Finalize(); // Finalize MPI
	
	return 0;
}


