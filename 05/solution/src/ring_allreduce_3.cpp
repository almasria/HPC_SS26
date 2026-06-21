/* HPDC SS 2026, Exercise 5.3 - Ring Allreduce, scaling the process count.
 *
 * Fixed problem size N = 1,000,000 floats. The process count is swept from
 * 2 to 8 by carving sub-communicators of size k out of MPI_COMM_WORLD, so a
 * SINGLE 8-process job produces the whole scaling column. Launch this binary
 * once per node layout (see 5_3_1node.sh / 5_3_8nodes.sh).
 *
 * Rank ordering note: MPI_Comm_split keeps ranks ordered by key=world-rank,
 * so world rank 0 is sub-rank 0 in every group and prints all rows. For the
 * "one node per process" layout (8 nodes, 1 task each, block placement),
 * world ranks 0..k-1 sit on k distinct nodes -> a genuine k-node measurement.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

/* C's % is remainder (can be negative); we need a true modulo. */
static inline int mod(int a, int b) {
    return ((a % b) + b) % b;
}

/* Ring Allreduce = Ring Reduce-Scatter + Ring Allgather (see Exercise 5.1). */
void ring_allreduce(const float *sendbuf, float *recvbuf, int count, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    memcpy(recvbuf, sendbuf, count * sizeof(float));
    if (size == 1) return;

    int next = mod(rank + 1, size);
    int prev = mod(rank - 1, size);

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
    int maxlen = base + (rem > 0 ? 1 : 0);
    float *tmp = (float *) malloc((maxlen > 0 ? maxlen : 1) * sizeof(float));

    /* Ring Reduce-Scatter */
    for (int i = 0; i < size - 1; ++i) {
        int send_idx = mod(rank - i,       size);
        int recv_idx = mod(rank - (i + 1), size);
        MPI_Sendrecv(recvbuf + offset[send_idx], len[send_idx], MPI_FLOAT, next, 0,
                     tmp,                         len[recv_idx], MPI_FLOAT, prev, 0,
                     comm, MPI_STATUS_IGNORE);
        float *dst = recvbuf + offset[recv_idx];
        for (int kk = 0; kk < len[recv_idx]; ++kk)
            dst[kk] += tmp[kk];
    }
    /* Ring Allgather */
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

/* Time one collective: barrier, measure, take MAX over the group's ranks,
 * average over 'reps'. Returns seconds (meaningful on sub-rank 0). */
static double time_ring(const float *in, float *out, int N, MPI_Comm comm, int reps) {
    double acc = 0.0;
    for (int r = 0; r < reps; ++r) {
        MPI_Barrier(comm);
        double t0 = MPI_Wtime();
        ring_allreduce(in, out, N, comm);
        double t = MPI_Wtime() - t0, tmax = 0.0;
        MPI_Reduce(&t, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
        acc += tmax;
    }
    return acc / reps;
}

static double time_mpi(const float *in, float *out, int N, MPI_Comm comm, int reps) {
    double acc = 0.0;
    for (int r = 0; r < reps; ++r) {
        MPI_Barrier(comm);
        double t0 = MPI_Wtime();
        MPI_Allreduce(in, out, N, MPI_FLOAT, MPI_SUM, comm);
        double t = MPI_Wtime() - t0, tmax = 0.0;
        MPI_Reduce(&t, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
        acc += tmax;
    }
    return acc / reps;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int wrank, wsize;
    MPI_Comm_rank(MPI_COMM_WORLD, &wrank);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    const int N       = 1000000;   /* fixed problem size */
    const int reps    = 5;
    const int p_max   = (wsize < 8 ? wsize : 8);
    const double rel_eps = 1e-5;

    if (wrank == 0) {
        printf("# Ring Allreduce process scaling | N=%d | reps=%d | launched procs=%d\n",
               N, reps, wsize);
        printf("# %-4s %18s %18s %8s\n", "P", "ring_avg[s]", "mpi_avg[s]", "check");
    }

    for (int k = 2; k <= p_max; ++k) {
        int color = (wrank < k) ? 0 : MPI_UNDEFINED;
        MPI_Comm sub;
        MPI_Comm_split(MPI_COMM_WORLD, color, wrank, &sub);

        if (wrank < k) {
            int srank, ssize;
            MPI_Comm_rank(sub, &srank);
            MPI_Comm_size(sub, &ssize);

            float *input  = (float *) malloc((size_t) N * sizeof(float));
            float *result = (float *) malloc((size_t) N * sizeof(float));
            float *ref    = (float *) malloc((size_t) N * sizeof(float));
            for (int i = 0; i < N; ++i)
                input[i] = (float) (srank + 1) * 0.5f + (float) (i % 17);

            /* correctness once */
            ring_allreduce(input, result, N, sub);
            MPI_Allreduce(input, ref, N, MPI_FLOAT, MPI_SUM, sub);
            int local_err = 0;
            for (int i = 0; i < N; ++i) {
                double a = result[i], b = ref[i];
                double denom = (fabs(b) > 1e-30) ? fabs(b) : 1.0;
                if (fabs(a - b) / denom > rel_eps) local_err++;
            }
            int total_err = 0;
            MPI_Reduce(&local_err, &total_err, 1, MPI_INT, MPI_SUM, 0, sub);

            /* warm-up (untimed) */
            ring_allreduce(input, result, N, sub);
            MPI_Allreduce(input, ref, N, MPI_FLOAT, MPI_SUM, sub);

            double ring_avg = time_ring(input, result, N, sub, reps);
            double mpi_avg  = time_mpi (input, ref,    N, sub, reps);

            if (srank == 0) {  /* == world rank 0 */
                printf("  P=%-2d %18.9f %18.9f %8s\n",
                       k, ring_avg, mpi_avg, total_err == 0 ? "OK" : "FAIL");
                fflush(stdout);
            }

            free(input);
            free(result);
            free(ref);
            MPI_Comm_free(&sub);
        }

        MPI_Barrier(MPI_COMM_WORLD);  /* idle ranks rejoin here */
    }

    MPI_Finalize();
    return 0;
}
