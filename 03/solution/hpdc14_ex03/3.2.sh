#!/bin/bash
#SBATCH -p exercise-hpdc
#SBATCH -N 4
#SBATCH --ntasks=96
#SBATCH --exclusive
#SBATCH --time=02:00:00
#SBATCH --output=results_%j.out

mpicxx -O3 3.1.cpp -o matmul

echo "===== Sequential baseline ====="
srun -N 1 -n 1 ./matmul > seq.txt

run_case () {
    nodes=$1
    p=$2

    echo "Nodes: $nodes | World size: $p"

    srun -N $nodes -n $p ./matmul > node${nodes}_p${p}.txt
}

echo "===== 1 NODE RUNS ====="
for p in 2 4 8 12 16 20 24
do
    run_case 1 $p
done

echo "===== 2 NODE RUNS ====="
for p in 2 4 12 24 36 48
do
    run_case 2 $p
done

echo "===== 4 NODE RUNS ====="
for p in 4 12 24 48 72 96
do
    run_case 4 $p
done

echo "===== DONE ====="