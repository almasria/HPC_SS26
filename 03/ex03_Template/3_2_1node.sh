#!/bin/bash

########################################################################
# HPDC SS 2026 - Exercise 3.2  (1 Node)
# Matrix Multiply - Scaling Ranks on 1 Node
#
# Runs: world sizes [2, 4, 8, 12, 16, 20, 24] on 1 node with 2048x2048.
# Results are collected into 3_2_1node.csv.
#
# Usage: sbatch 3_2_1node.sh
########################################################################

#SBATCH --partition exercise-hpdc   # partition name
#SBATCH --exclusive                 # exclusive access for reliable benchmarking
#SBATCH --nodes 1                   # 1 node
#SBATCH --ntasks 24                 # maximum tasks needed in this script
#SBATCH --ntasks-per-core 1         # max 1 task per core (default)
#SBATCH --time 01:00:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-ex3-2-1n    # job name
#SBATCH --output job_%x-%j.txt      # output file (%j = job ID)

echo "nnodes: $SLURM_NNODES"
echo "ntasks: $SLURM_NTASKS"
echo "nodes:  $SLURM_JOB_NODELIST"

BINARY="./bin/exercise2"
N=2048
SIZES="2 4 8 12 16 20 24"

# Create CSV header
echo "world_size,nodes,time_s" > 3_2_1node.csv

for np in $SIZES; do
    echo "--- Running: $np ranks on 1 node ---"
    output=$(mpirun -np $np $BINARY --size $N)
    echo "$output"
    time_s=$(echo "$output" | awk '/^Time:/ {print $2}')
    echo "$np,1,$time_s" >> 3_2_1node.csv
done

echo ""
echo "Results written to 3_2_1node.csv"
