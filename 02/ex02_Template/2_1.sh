#!/bin/bash

########################################################################
# This is a template for running an MPI program on the CSG cluster.
# To use this template, call it with the following command:
# sbatch 2_1.sh
########################################################################


# SLURM job script parameters, for more parameters and information see: https://slurm.schedmd.com/sbatch.html
#SBATCH --partition exercise-hpdc   # partition name, for HPDC, use exercise-hpdc
##SBATCH --exclusive                # FOR BENCHMARKING: remove the trailing # to request exclusive access nodes
#SBATCH --nodes 1                   # number of nodes
#SBATCH --ntasks 2                  # total number of tasks (processes)
# SBATCH --ntasks-per-node 2         # OPTIONAL for more control: number of tasks per node, ensure that nodes*ntasks-per-node == ntasks
# SBATCH --cpus-per-task 1           # OPTIONAL for more control: number of processors per task
#SBATCH --ntasks-per-core 1         # maximum number of tasks per core, this is the default
#SBATCH --time 00:30:00             # time limit (hh:mm:ss)
#SBATCH --job-name hpdc-template    # job name
#SBATCH --output job_%x-%j.txt      # output file name, %j is replaced by job ID by slurm

echo "nnodes:" $SLURM_NNODES
echo "ntasks:" $SLURM_NTASKS
echo "nodes:" $SLURM_JOB_NODELIST

# options for the number of nodes and tasks are set automatically by slurm, for more information on changing options yourself, see: https://docs.open-mpi.org/en/v5.0.x/man-openmpi/man1/mpirun.1.html#launch-options
mpirun ./bin/exercise1

####################################################################################################################################
#example structure how you could do your .sh files
####################################################################################################################################
sizes="2 4 6 8 10 12 14 16 18 20 22 24"

# Create headline for output csv
echo "processes, custom_latency_ms, built_in_latency_ms" > 2_1.csv

for size in $sizes; do
    output_custom=$(mpirun --oversubscribe -np $size ./bin/exercise1 --custom)
    echo "$output_custom"
    latency_custom=`echo "$output_custom" | awk '/Custom Barrier Time:/ {print $4}'`

    output_built_in=$(mpirun --oversubscribe -np $size ./bin/exercise1 --built-in)
    echo "$output_built_in"
    latency_built_in=`echo "$output_built_in" | awk '/Built-In Barrier Time:/ {print $4}'`

    echo "$size, $latency_custom, $latency_built_in" >> 2_1.csv
done
#####################################################################################################################################

# To get information about the mapping of tasks to cores and nodes, you can use the following command:
# mpirun --display-map template sample_cli_parameter