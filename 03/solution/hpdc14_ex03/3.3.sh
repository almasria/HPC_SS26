#!/bin/bash
#SBATCH -p exercise-hpdc
#SBATCH -N 2
#SBATCH --ntasks=24
#SBATCH --exclusive
#SBATCH --time=02:00:00
#SBATCH --output=results_3_3_%j.out

mpicxx -O3 3.1.cpp -o matmul

echo "===== MATRIX SIZE SCALING ====="

run_case () {
    p=$1
    n=$2

    echo "Ranks: $p | Size: $n x $n"

    srun -N 2 -n $p ./matmul $n > size${n}_p${p}.txt
}

for p in 12 24
do
    for n in 128 256 512 1024 2048 4096 8192 16384
    do
        run_case $p $n
    done
done

echo "===== DONE ====="