#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=openmp-scal
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --exclusive
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --output=./outputs/slurm-%j-openmp-scal.txt

module load openMPI/4.1.6/gnu/14.2.1

echo "OpenMP scalability"
echo "Running game of life with different #threads"
echo "to collect data for time/threads plots"

output_file="./outputs/timings/openmp-scal-$SLURM_JOB_ID.csv"
max_num_threads=24 



# Outer loop repeats the measurements for different matrix dimensions
# The following dimensions are tested: (100, 500, 1000, 1500, 2000)

if [ "$1" == "-s" ]; then
    output_file="./outputs/timings/openmp-scal-$SLURM_JOB_ID-serial.csv"
fi
echo "dim,nthreads,time" > "$output_file"
for dim in 1000 1500 2000
do
    # Check if single thread flag is provided
    if [ "$1" == "-s" ]; then
        echo "Using dimension $dim"
        echo "Running with a single thread"

        export OMP_NUM_THREADS=1
        output=$(srun ---ntasks-per-socket=1 ./build/gol.x -i -k $dim)
        time=$(echo "$output" | tail -n 1)
        echo "$dim,1,$time" >> "$output_file"
        echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID-serial.txt"
    else
        echo "Using dimension $dim"
        echo "Running with 1 to 64 cores per socket"
        # Run the loop max_num_threads times
        for ((i=1; i<=$max_num_threads; i+=1))
        do
            echo "Running with $i threads"
            export OMP_NUM_THREADS=$i
            
            output=$(srun --ntasks-per-socket=1 ./build/gol.x -i -k $dim)

            time=$(echo "$output" | tail -n 1)
            echo "$dim,$i,$time" >> "$output_file"

            echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
        done
    fi
done

echo "Finished"
