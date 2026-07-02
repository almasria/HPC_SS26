#!/bin/bash

########################################################################
# Exercise 5.1 & 5.2: Ring Allreduce - All processes on ONE node
# To use this template, call it with the following command:
# sbatch 5_1_1node.sh
########################################################################


# SLURM job script parameters, for more parameters and information see: https://slurm.schedmd.com/sbatch.html
#SBATCH --partition exercise-hpdc   # partition name, for HPDC, use exercise-hpdc
#SBATCH --exclusive                 # FOR BENCHMARKING: request exclusive access to nodes
#SBATCH --nodes 1                   # number of nodes
#SBATCH --ntasks 8                  # total number of tasks (processes)
#SBATCH --ntasks-per-node 8         # all 8 processes on one node
#SBATCH --ntasks-per-core 1         # maximum number of tasks per core, this is the default
#SBATCH --time 00:30:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-ex5-1node   # job name
#SBATCH --output job_%x-%j.txt      # output file name, %j is replaced by job ID by slurm

echo "nnodes:" $SLURM_NNODES
echo "ntasks:" $SLURM_NTASKS
echo "nodes:" $SLURM_JOB_NODELIST

mpirun bin/ring_allreduce_2
