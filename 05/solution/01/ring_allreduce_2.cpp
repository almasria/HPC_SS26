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

/* ---------------------------------------------------------------------------
 * Benchmark (Exercise 5.2)
 *
 *   - 8 processes (set by the SLURM script / mpirun).
 *   - Problem size swept in powers of two from 2^16 to 2^22 floats.
 *   - Only the collective call is timed; allocation / MPI_Init are excluded.
 *   - Each measurement is the MAX time over all ranks (the collective is only
 *     done when the slowest rank finishes), averaged over 5 runs.
 *   - Correctness is verified once per size against MPI_Allreduce.
 *
 * Run the SAME binary under each SLURM script (1 node vs. 8 nodes); each run
 * fills two columns of the results table.
 * ------------------------------------------------------------------------- */

static double time_collective_ring(const float *in, float *out, int N,
                                    MPI_Comm comm, int reps) {
    double acc = 0.0;
    for (int r = 0; r < reps; ++r) {
        MPI_Barrier(comm);            /* line everyone up before timing      */
        double t0 = MPI_Wtime();
        ring_allreduce(in, out, N, comm);
        double t = MPI_Wtime() - t0;  /* this rank's wall time               */
        double tmax = 0.0;
        MPI_Reduce(&t, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
        acc += tmax;                  /* only meaningful on rank 0           */
    }
    return acc / reps;
}

static double time_collective_mpi(const float *in, float *out, int N,
                                   MPI_Comm comm, int reps) {
    double acc = 0.0;
    for (int r = 0; r < reps; ++r) {
        MPI_Barrier(comm);
        double t0 = MPI_Wtime();
        MPI_Allreduce(in, out, N, MPI_FLOAT, MPI_SUM, comm);
        double t = MPI_Wtime() - t0;
        double tmax = 0.0;
        MPI_Reduce(&t, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
        acc += tmax;
    }
    return acc / reps;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int reps     = 5;   /* average over 5 runs                         */
    const int exp_lo   = 16;  /* 2^16 floats                                 */
    const int exp_hi   = 22;  /* 2^22 floats                                 */
    const double rel_eps = 1e-5;

    if (rank == 0) {
        printf("# Ring Allreduce benchmark | procs=%d | reps=%d\n", size, reps);
        printf("# %-8s %18s %18s %10s\n",
               "size", "ring_avg[s]", "mpi_avg[s]", "check");
    }

    for (int e = exp_lo; e <= exp_hi; ++e) {
        int N = 1 << e;   /* 2^e floats */

        float *input  = (float *) malloc((size_t) N * sizeof(float));
        float *result = (float *) malloc((size_t) N * sizeof(float));
        float *ref    = (float *) malloc((size_t) N * sizeof(float));

        for (int i = 0; i < N; ++i)
            input[i] = (float) (rank + 1) * 0.5f + (float) (i % 17);

        /* ---- correctness (once) ---- */
        ring_allreduce(input, result, N, MPI_COMM_WORLD);
        MPI_Allreduce(input, ref, N, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
        int local_err = 0;
        for (int i = 0; i < N; ++i) {
            double a = result[i], b = ref[i];
            double denom = (fabs(b) > 1e-30) ? fabs(b) : 1.0;
            if (fabs(a - b) / denom > rel_eps) local_err++;
        }
        int total_err = 0;
        MPI_Reduce(&local_err, &total_err, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        /* ---- warm-up (not measured): touches pages, warms caches/links ---- */
        ring_allreduce(input, result, N, MPI_COMM_WORLD);
        MPI_Allreduce(input, ref, N, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

        /* ---- timed runs ---- */
        double ring_avg = time_collective_ring(input, result, N, MPI_COMM_WORLD, reps);
        double mpi_avg  = time_collective_mpi (input, ref,    N, MPI_COMM_WORLD, reps);

        if (rank == 0) {
            printf("  2^%-6d %18.9f %18.9f %10s\n",
                   e, ring_avg, mpi_avg, total_err == 0 ? "OK" : "FAIL");
            fflush(stdout);
        }

        free(input);
        free(result);
        free(ref);
    }

    MPI_Finalize();
    return 0;
}
