#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=serial-GOL
#SBATCH --partition=EPYC
#SBATCH --time=02:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --output=./outputs/slurm-%j-serial.txt

module load openMPI/4.1.5/gnu/12.2.1

echo "Measuring serial execution times for gol.x"

output_dir="./outputs/timings"
mkdir -p "$output_dir"
output_file="$output_dir/${SLURM_JOB_ID}-serial.csv"

echo "dim,nthreads,time" > "$output_file"
for dim in 1000 1500 2000; do
    echo "Using dimension $dim"
    echo "Running with a single thread"
    total_time=0
    for i in {1..20}; do
        echo "Iteration $i"
        output=$(srun -n1 ./build/gol.x -i -k $dim)
        iteration_time=$(echo "$output" | tail -n 1 | awk '{print $NF}')
        total_time=$(echo "$total_time + $iteration_time" | bc)
    done
    avg_time=$(echo "scale=2; $total_time / 20" | bc)
    echo "$dim,1,$avg_time" >> "$output_file"
    echo "$output" >> "./outputs/slurm-${SLURM_JOB_ID}-serial.txt"
done

echo "Finished"
