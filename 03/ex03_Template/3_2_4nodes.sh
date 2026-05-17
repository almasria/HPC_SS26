#!/bin/bash

########################################################################
# HPDC SS 2026 - Exercise 3.2  (4 Nodes)
# Matrix Multiply - Scaling Ranks on 4 Nodes
#
# Runs: world sizes [4, 12, 24, 48, 72, 96] on 4 nodes with 2048x2048.
# Tasks are distributed evenly across all four nodes.
# Results are collected into 3_2_4nodes.csv.
#
# Usage: sbatch 3_2_4nodes.sh
########################################################################

#SBATCH --partition exercise-hpdc   # partition name
#SBATCH --exclusive                 # exclusive access for reliable benchmarking
#SBATCH --nodes 4                   # 4 nodes
#SBATCH --ntasks 96                 # maximum tasks needed in this script
#SBATCH --ntasks-per-core 1         # max 1 task per core (default)
#SBATCH --time 01:30:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-ex3-2-4n    # job name
#SBATCH --output job_%x-%j.txt      # output file (%j = job ID)

echo "nnodes: $SLURM_NNODES"
echo "ntasks: $SLURM_NTASKS"
echo "nodes:  $SLURM_JOB_NODELIST"

BINARY="./bin/exercise2"
N=2048
# Sizes distributed evenly on 4 nodes (ntasks-per-node = np/4)
SIZES="4 12 24 48 72 96"

# Create CSV header
echo "world_size,nodes,time_s" > 3_2_4nodes.csv

for np in $SIZES; do
    echo "--- Running: $np ranks on 4 nodes ---"
    output=$(mpirun -np $np $BINARY --size $N)
    echo "$output"
    time_s=$(echo "$output" | awk '/^Time:/ {print $2}')
    echo "$np,4,$time_s" >> 3_2_4nodes.csv
done

echo ""
echo "Results written to 3_2_4nodes.csv"
