#!/bin/bash
#SBATCH -p exercise-hpdc
#SBATCH -N 2
#SBATCH -n 2
#SBATCH --ntasks-per-node=1
#SBATCH --exclusive
#SBATCH --output=mat_mult.out

mpirun --bind-to none -np 1 ./3.1