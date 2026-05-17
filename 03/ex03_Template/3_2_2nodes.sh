#!/bin/bash

########################################################################
# HPDC SS 2026 - Exercise 3.2  (2 Nodes)
# Matrix Multiply - Scaling Ranks on 2 Nodes
#
# Runs: world sizes [2, 4, 12, 24, 36, 48] on 2 nodes with 2048x2048.
# Tasks are distributed evenly across both nodes.
# Results are collected into 3_2_2nodes.csv.
#
# Usage: sbatch 3_2_2nodes.sh
########################################################################

#SBATCH --partition exercise-hpdc   # partition name
#SBATCH --exclusive                 # exclusive access for reliable benchmarking
#SBATCH --nodes 2                   # 2 nodes
#SBATCH --ntasks 48                 # maximum tasks needed in this script
#SBATCH --ntasks-per-core 1         # max 1 task per core (default)
#SBATCH --time 01:00:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-ex3-2-2n    # job name
#SBATCH --output job_%x-%j.txt      # output file (%j = job ID)

echo "nnodes: $SLURM_NNODES"
echo "ntasks: $SLURM_NTASKS"
echo "nodes:  $SLURM_JOB_NODELIST"

BINARY="./bin/exercise2"
N=2048
# Sizes distributed evenly on 2 nodes (ntasks-per-node = np/2)
SIZES="2 4 12 24 36 48"

# Create CSV header
echo "world_size,nodes,time_s" > 3_2_2nodes.csv

for np in $SIZES; do
    echo "--- Running: $np ranks on 2 nodes ---"
    output=$(mpirun -np $np $BINARY --size $N)
    echo "$output"
    time_s=$(echo "$output" | awk '/^Time:/ {print $2}')
    echo "$np,2,$time_s" >> 3_2_2nodes.csv
done

echo ""
echo "Results written to 3_2_2nodes.csv"
