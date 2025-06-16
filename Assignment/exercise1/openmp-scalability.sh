#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=openmp-scal
#SBATCH --partition=THIN
#SBATCH --nodelist=thin008
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=12
#SBATCH --nodes=1
#SBATCH --output=./outputs/slurm-%j-openmp-scal.txt

module load openMPI/4.1.6
# 0 -> ordered, 1 -> static
evolution=1
if [ "$evolution" -eq 0 ]; then
    echo "OpenMP scalability for ordered evolution"
fi
if [ "$evolution" -eq 1 ]; then
	echo "OpenMP scalability for static evolution"
fi

echo "Running game of life with different #threads"
echo "to collect data for time/threads plots"

# Create a folder with today's date
today=$(date +%Y-%m-%d)
output_dir="./outputs/$today"
mkdir -p "$output_dir/timings"

output_file="$output_dir/timings/openmp-scal-$SLURM_JOB_ID.csv"
max_num_threads=12

export OMP_PLACES=cores
export OMP_PROC_BIND=close
echo "OMP_PLACES = $OMP_PLACES , OMP_PROC_BIND = $OMP_PROC_BIND"

echo "dim,nthreads,time" > "$output_file"
#for dim in 1000 2000 4000 8000 16000
for dim in 15000 20000 25000 
do
        echo "Using dimension $dim"
        echo "Running with 1 to 12 cores per socket"
        # Run the loop max_num_threads times
        for i in 1 2 4 6 8 10 12 
        do
            echo "Running with $i threads"
            export OMP_NUM_THREADS=$i
            
            output=$(./build/gol.x -i -e $evolution -k $dim -s 0)
            time=$(echo "$output" | tail -n 1)
            echo "$dim,$i,$time" >> "$output_file"

            echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
        done
done

echo "Finished"
