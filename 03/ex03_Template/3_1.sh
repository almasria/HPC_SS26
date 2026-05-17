#!/bin/bash

########################################################################
# HPDC SS 2026 - Exercise 3.1
# Matrix Multiply - Parallel MPI Version (verification + 5x5 output)
#
# Usage: sbatch 3_1.sh
########################################################################

#SBATCH --partition exercise-hpdc   # partition name
#SBATCH --nodes 1                   # number of nodes
#SBATCH --ntasks 2                  # 2 worker ranks as required
#SBATCH --ntasks-per-core 1         # max 1 task per core (default)
#SBATCH --time 00:10:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-ex3-1       # job name
#SBATCH --output job_%x-%j.txt      # output file (%j = job ID)

echo "nnodes: $SLURM_NNODES"
echo "ntasks: $SLURM_NTASKS"
echo "nodes:  $SLURM_JOB_NODELIST"

# Run the parallel matrix multiply with 2 ranks
# The binary prints: verification result + 5x5 C matrix + timing
mpirun ./bin/exercise1

# To inspect rank-to-core mapping:
# mpirun --display-map ./bin/exercise1
