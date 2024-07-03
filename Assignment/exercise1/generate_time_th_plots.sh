#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=time-th-plots
#SBATCH --partition=EPYC
#SBATCH --time=0:30:0
#SBATCH --exclusive
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --output=./outputs/slurm-%j.txt

echo "Running game of life with different #threads"
echo "to collect data for time/threads plots"

output_file="./outputs/timings/time-th-$SLURM_JOB_ID.csv"

echo "nthreads,time" > "$output_file"

# Check if -s flag is provided
if [ "$1" == "-s" ]; then
    echo "Running with a single thread"
    export OMP_NUM_THREADS=1
    output=$(srun -n1 ./build/gol.x -i -k 1000)
    time=$(echo "$output" | tail -n 1)
    echo "1,$time" >> "$output_file"
    echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
else
    # Run the loop 50 times
    for ((i=1; i<=50; i+=1))
    do
        echo "Running with $i threads"
        export OMP_NUM_THREADS=$i
        
        output=$(srun -n1 ./build/gol.x -i -k 1000)

        time=$(echo "$output" | tail -n 1)
        echo "$i,$time" >> "$output_file"

        echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
    done
fi

echo "Finished"
