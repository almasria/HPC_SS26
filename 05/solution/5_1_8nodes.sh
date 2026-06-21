#!/bin/bash

########################################################################
# Exercise 5.1 & 5.2: Ring Allreduce - Each process on a DIFFERENT node
# To use this template, call it with the following command:
# sbatch 5_1_8nodes.sh
#
# NOTE: Do NOT use --exclusive here (as stated in the exercise sheet)
########################################################################


# SLURM job script parameters, for more parameters and information see: https://slurm.schedmd.com/sbatch.html
#SBATCH --partition exercise-hpdc   # partition name, for HPDC, use exercise-hpdc
#SBATCH --nodes 8                   # number of nodes
#SBATCH --ntasks 8                  # total number of tasks (processes)
#SBATCH --ntasks-per-node 1         # one process per node
#SBATCH --ntasks-per-core 1         # maximum number of tasks per core, this is the default
#SBATCH --time 00:30:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-ex5-8nodes  # job name
#SBATCH --output job_%x-%j.txt      # output file name, %j is replaced by job ID by slurm

echo "nnodes:" $SLURM_NNODES
echo "ntasks:" $SLURM_NTASKS
echo "nodes:" $SLURM_JOB_NODELIST

mpirun bin/ring_allreduce
