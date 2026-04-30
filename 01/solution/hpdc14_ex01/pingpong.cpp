#include <iostream>
#include <vector>
#include <iomanip>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Basic error checking
    if (world_size != 2)
    {
        if (world_rank == 0)
        {
            std::cerr << "Error: This program must be run with exactly 2 processes. It was run by: " << world_size << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    const int MAX_EXP = 20;     // Up to 1MB (2^20)
    const int ITERATIONS = 500; // Increased iterations for more stable results

    if (world_rank == 0)
    {
        std::cout << std::left << std::setw(15) << "# Bytes"
                  << std::setw(20) << "Full-RTT (s)"
                  << std::setw(20) << "Half-RTT (s)" << std::endl;
        std::cout << std::string(55, '-') << std::endl;
    }

    for (int i = 0; i <= MAX_EXP; i++)
    {
        size_t message_size = 1 << i;
        std::vector<char> buffer(message_size, 'a');

        // Synchronize before measurement
        MPI_Barrier(MPI_COMM_WORLD);

        double start_time = MPI_Wtime();

        for (int j = 0; j < ITERATIONS; j++)
        {
            if (world_rank == 0)
            {
                MPI_Send(buffer.data(), message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer.data(), message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else
            {
                MPI_Recv(buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        double end_time = MPI_Wtime();

        if (world_rank == 0)
        {
            double total_time = end_time - start_time;
            double avg_rtt = total_time / ITERATIONS;
            double latency = avg_rtt / 2.0;

            std::cout << std::left << std::setw(15) << message_size
                      << std::setw(20) << std::scientific << avg_rtt
                      << std::setw(20) << std::scientific << latency << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}