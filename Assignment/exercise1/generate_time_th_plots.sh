#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=time-th-plots
#SBATCH --partition=EPYC
#SBATCH --time=0:30:0
#SBATCH -n 1
#SBATCH -N1
#SBATCH --output=./outputs/slurm-%j.txt

echo "Running game of life with different #threads"
echo "to collect data for time/threads plots"

output_file="./outputs/timings/time-th-$SLURM_JOB_ID.csv"

echo "nthreads,time" > "$output_file"

for ((i=1; i<=50; i+=1))
do
    echo "Running with $i threads"
    export OMP_NUM_THREADS=$i
    
    output=$(srun -n1 ./build/gol.x -i -k 500)
    time=$(echo "$output" | tail -n 1)
    echo "$i,$time" >> "$output_file"

    echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"

done

echo "Finished"
