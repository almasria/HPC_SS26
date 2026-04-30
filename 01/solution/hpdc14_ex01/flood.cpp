#include <iostream>
#include <vector>
#include <iomanip>
#include <mpi.h>

int main(int argc, char **argv)
{
    // Use a specific threading level if required by the cluster,
    // though Init is usually fine for this test.
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 2)
    {
        if (world_rank == 0)
            std::cerr << "Error: Use 2 processes." << std::endl;
        MPI_Finalize();
        return 1;
    }

    const int MAX_EXP = 20;
    const int ITERATIONS = 1000;

    if (world_rank == 0)
    {
        std::cout << std::left << std::setw(12) << "# Bytes"
                  << std::setw(20) << "Blocking (MB/s)"
                  << std::setw(20) << "Non-Blocking (MB/s)" << std::endl;
        std::cout << std::string(52, '-') << std::endl;
    }

    for (int i = 0; i <= MAX_EXP; i++)
    {
        int message_size = 1 << i; // Use int to match MPI count parameters
        std::vector<char> buffer(message_size, 'f');
        double start_time, end_time;

        // --- 1. BLOCKING SEND TEST ---
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        for (int j = 0; j < ITERATIONS; j++)
        {
            if (world_rank == 0)
            {
                MPI_Send(buffer.data(), message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Recv(buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        end_time = MPI_Wtime();
        double bw_blocking = (static_cast<double>(message_size) * ITERATIONS) / ((end_time - start_time) * 1024 * 1024);

        // --- 2. NON-BLOCKING SEND TEST ---
        // Using a raw array for requests to be as compatible as possible
        MPI_Request *requests = new MPI_Request[ITERATIONS];

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        if (world_rank == 0)
        {
            for (int j = 0; j < ITERATIONS; j++)
            {
                MPI_Isend(buffer.data(), message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &requests[j]);
            }
            MPI_Waitall(ITERATIONS, requests, MPI_STATUSES_IGNORE);
        }
        else
        {
            for (int j = 0; j < ITERATIONS; j++)
            {
                MPI_Irecv(buffer.data(), message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &requests[j]);
            }
            MPI_Waitall(ITERATIONS, requests, MPI_STATUSES_IGNORE);
        }
        end_time = MPI_Wtime();

        double bw_nonblocking = (static_cast<double>(message_size) * ITERATIONS) / ((end_time - start_time) * 1024 * 1024);

        if (world_rank == 0)
        {
            std::cout << std::left << std::setw(12) << message_size
                      << std::setw(20) << std::fixed << std::setprecision(2) << bw_blocking
                      << std::setw(20) << std::fixed << std::setprecision(2) << bw_nonblocking << std::endl;
        }

        delete[] requests;
    }

    MPI_Finalize();
    return 0;
}