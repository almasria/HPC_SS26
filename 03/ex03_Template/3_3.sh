#!/bin/bash

########################################################################
# HPDC SS 2026 - Exercise 3.3
# Matrix Multiply - Scaling Problem Size
#
# Runs with 12 and 24 ranks (evenly distributed on 2 nodes).
# Varies matrix size from 128 to 16384 (powers of two).
# Collects time and GFLOP/s into 3_3.csv.
#
# NOTE: For 16384x16384 matrices, memory usage is significant.
#       Make sure your allocation can handle it (16384^2 * 8 bytes ~ 2 GB per matrix).
#
# Usage: sbatch 3_3.sh
########################################################################

#SBATCH --partition exercise-hpdc   # partition name
#SBATCH --exclusive                 # exclusive access for reliable benchmarking
#SBATCH --nodes 2                   # 2 nodes (even distribution)
#SBATCH --ntasks 24                 # maximum tasks needed (24 ranks)
#SBATCH --ntasks-per-core 1         # max 1 task per core (default)
#SBATCH --time 02:00:00             # time limit (hh:mm:ss) — large N can be a bit slow
#SBATCH --job-name hpdc-ex3-3       # job name
#SBATCH --output job_%x-%j.txt      # output file (%j = job ID)

echo "nnodes: $SLURM_NNODES"
echo "ntasks: $SLURM_NTASKS"
echo "nodes:  $SLURM_JOB_NODELIST"

BINARY="./bin/exercise2"
# Problem sizes: powers of two from 128 to 16384
SIZES="128 256 512 1024 2048 4096 8192 16384"
# Rank configurations
RANKS="12 24"

# Create CSV header
echo "world_size,problem_size,flops,time_s,gflops" > 3_3.csv

for np in $RANKS; do
    for N in $SIZES; do
        echo "--- Running: $np ranks, N=$N ---"
        output=$(mpirun -np $np $BINARY --size $N)
        echo "$output"
        time_s=$(echo "$output"  | awk '/^Time:/    {print $2}')
        gflops=$(echo "$output"  | awk '/^GFLOP\/s:/ {print $2}')
        # FLOPS = 2 * N^3
        flops=$(python3 -c "print(2 * $N**3)")
        echo "$np,$N,$flops,$time_s,$gflops" >> 3_3.csv
    done
done

echo ""
echo "Results written to 3_3.csv"
