#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mpi.h>


// customBarrier implement a barrier
void customBarrier(int rank, int iterations, int size) {
    // Synchronize all processes before starting the timer
    MPI_Barrier(MPI_COMM_WORLD);

    double starttime = MPI_Wtime();
    double endtime;
    
    // Dummy variable for message payload
    int dummy = 0;
    
    // Loop for the specified number of iterations
    for (int i = 0; i < iterations; i++) {
        if (rank == 0) {
            // Coordinator waits for arrival messages from all workers
            for (int p = 1; p < size; p++) {
                MPI_Recv(&dummy, 1, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            // Coordinator broadcasts release message to all workers
            for (int p = 1; p < size; p++) {
                MPI_Send(&dummy, 1, MPI_INT, p, 1, MPI_COMM_WORLD);
            }
        } else {
            // Worker sends arrival message to coordinator
            MPI_Send(&dummy, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            // Worker blocks until it receives the release message
            MPI_Recv(&dummy, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    
    // Stop the timer
    endtime = MPI_Wtime();

    // Output result on last rank: This rank is expected to receive the release last and should therefore be the correct time when the barrier was fully released
    // This cout will then be used in the .sh to collect the measurements in the .csv
    if (rank == size-1) std::cout << "Custom Barrier Time: " << ((endtime-starttime) / ((double) iterations / 1000)) << " ms" << std::endl;
}

// use for build in Barrier
void builtInBarrier(int rank, int iterations, int size) {
    // Synchronize all processes before starting the timer
    MPI_Barrier(MPI_COMM_WORLD);

    // Initialize starttime
    double starttime = MPI_Wtime();
    double endtime;

    // Execute the built-in barrier for the specified iterations
    for (int i = 0; i < iterations; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Stop the timer
    endtime = MPI_Wtime();

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

    // Set a default iteration count
    int iterations = 10000;
    
    // Check for the --custom and --built-in flags sent by 2_1.sh
    bool runCustom = false;
    bool runBuiltIn = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--custom") == 0 || strcmp(argv[i], "-custom") == 0) {
            runCustom = true;
        }
        if (strcmp(argv[i], "--built-in") == 0 || strcmp(argv[i], "-built-in") == 0) {
            runBuiltIn = true;
        }
    }
    
    // If no flags are provided, run both as a default fallback
    if (!runCustom && !runBuiltIn) {
        runCustom = true;
        runBuiltIn = true;
    }
    
    // Run the custom barrier benchmark if requested
    if (runCustom) {
        customBarrier(rank, iterations, size);
    }
    
    // Run the built-in barrier benchmark if requested
    if (runBuiltIn) {
        builtInBarrier(rank, iterations, size);
    }
    
    MPI_Finalize(); // Finalize MPI
    
    return 0;
}