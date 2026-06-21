/* HPDC SS 2026, Exercise 5 - Ring Allreduce */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/* C's % is the remainder operator and can return negative values for
 * negative operands. We need a true modulo, so wrap it. */
static inline int mod(int a, int b) {
    return ((a % b) + b) % b;
}

/* Ring Allreduce = Ring Reduce-Scatter + Ring Allgather.
 *
 *   sendbuf : input array of 'count' floats (unchanged)
 *   recvbuf : output array of 'count' floats (gets the element-wise sum
 *             across all ranks); must be allocated by the caller
 *
 * The operation is performed in-place inside recvbuf. */
void ring_allreduce(const float *sendbuf, float *recvbuf, int count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    /* Start from a copy of the local input; everything below works on recvbuf. */
    memcpy(recvbuf, sendbuf, count * sizeof(float));

    /* Trivial case: nothing to exchange. */
    if (size == 1) return;

    int next = mod(rank + 1, size); /* successor   (we always send here)   */
    int prev = mod(rank - 1, size); /* predecessor (we always receive here) */

    /* ---- Chunk layout: as equal as possible -----------------------------
     * There are exactly 'size' chunks. If count is not divisible by size,
     * the first (count % size) chunks are one element larger. We precompute
     * the offset and length of each chunk so the code below can address them
     * directly. */
    int base = count / size;
    int rem  = count % size;

    int *offset = (int *) malloc(size * sizeof(int));
    int *len    = (int *) malloc(size * sizeof(int));
    int o = 0;
    for (int c = 0; c < size; ++c) {
        len[c]    = base + (c < rem ? 1 : 0);
        offset[c] = o;
        o += len[c];
    }

    /* Temp buffer to receive an incoming chunk before adding it locally.
     * Sized to the largest possible chunk (at least 1 to keep malloc happy). */
    int maxlen = base + (rem > 0 ? 1 : 0);
    float *tmp = (float *) malloc((maxlen > 0 ? maxlen : 1) * sizeof(float));

    /* ---- Ring Reduce-Scatter (size-1 iterations) ------------------------
     * Iteration i:
     *   send    chunk (r - i)       mod s  to   successor
     *   receive chunk (r - (i+1))   mod s  from predecessor
     *   add the received values onto the local copy of that chunk.
     * After the last iteration, chunk (r+1) mod s of rank r holds the full
     * sum of that chunk over all ranks. */
    for (int i = 0; i < size - 1; ++i) {
        int send_idx = mod(rank - i,       size);
        int recv_idx = mod(rank - (i + 1), size);

        MPI_Sendrecv(recvbuf + offset[send_idx], len[send_idx], MPI_FLOAT, next, 0,
                     tmp,                         len[recv_idx], MPI_FLOAT, prev, 0,
                     comm, MPI_STATUS_IGNORE);

        float *dst = recvbuf + offset[recv_idx];
        for (int k = 0; k < len[recv_idx]; ++k)
            dst[k] += tmp[k];
    }

    /* ---- Ring Allgather (size-1 iterations) -----------------------------
     * Iteration i:
     *   send    chunk (r + 1 - i) mod s  to   successor
     *   receive chunk (r - i)     mod s  from predecessor
     *   overwrite the local chunk with what is received (no addition).
     * send_idx and recv_idx are always adjacent chunks, so the send and
     * receive regions never overlap -> safe to receive straight into recvbuf. */
    for (int i = 0; i < size - 1; ++i) {
        int send_idx = mod(rank + 1 - i, size);
        int recv_idx = mod(rank - i,     size);

        MPI_Sendrecv(recvbuf + offset[send_idx], len[send_idx], MPI_FLOAT, next, 1,
                     recvbuf + offset[recv_idx], len[recv_idx], MPI_FLOAT, prev, 1,
                     comm, MPI_STATUS_IGNORE);
    }

    free(offset);
    free(len);
    free(tmp);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Array length: optional first command-line argument, default 1000. */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;

    float *input  = (float *) malloc(N * sizeof(float));
    float *result = (float *) malloc(N * sizeof(float));
    float *ref    = (float *) malloc(N * sizeof(float));

    /* Fill with rank-dependent data so every chunk contributes differently. */
    for (int i = 0; i < N; ++i)
        input[i] = (float) (rank + 1) * 0.5f + (float) (i % 17);

    /* Our implementation. */
    double t0 = MPI_Wtime();
    ring_allreduce(input, result, N, MPI_COMM_WORLD);
    double t_ring = MPI_Wtime() - t0;

    /* Reference: MPI's native allreduce. */
    double t1 = MPI_Wtime();
    MPI_Allreduce(input, ref, N, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    double t_mpi = MPI_Wtime() - t1;

    /* Correctness check with a RELATIVE epsilon. With large sums the float
     * spacing exceeds 1, so absolute deviations can look like integers; a
     * relative tolerance handles this. */
    const double rel_eps = 1e-5; /* 0.001 % */
    int local_errors = 0;
    for (int i = 0; i < N; ++i) {
        double a = result[i], b = ref[i];
        double denom = (fabs(b) > 1e-30) ? fabs(b) : 1.0;
        if (fabs(a - b) / denom > rel_eps) {
            if (local_errors < 5)
                printf("[rank %d] mismatch at %d: got %.6f expected %.6f\n",
                       rank, i, a, b);
            local_errors++;
        }
    }

    int total_errors = 0;
    MPI_Reduce(&local_errors, &total_errors, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double max_ring = 0.0, max_mpi = 0.0;
    MPI_Reduce(&t_ring, &max_ring, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_mpi,  &max_mpi,  1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("N=%d  procs=%d  ->  %s (%d mismatches)\n",
               N, size, total_errors == 0 ? "CORRECT" : "WRONG", total_errors);
        printf("ring_allreduce: %.6f s   MPI_Allreduce: %.6f s\n", max_ring, max_mpi);
    }

    free(input);
    free(result);
    free(ref);

    MPI_Finalize();
    return 0;
}
