#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=mini-openmp-scal
#SBATCH --partition=THIN
#SBATCH --nodelist=thin002
#SBATCH --time=00:10:0
#SBATCH --cpus-per-task=12
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2
#SBATCH --output=./outputs/slurm-%j-openmp-scal-thin.txt
#SBATCH --mem=100gb

module load openMPI/4.1.6/gnu/14.2.1
# 0 -> ordered, 1 -> static
evolution=1
if [evolution==1]; then
	echo "OpenMP scalability for static evolution"
fi
num_steps=20

echo "Running game of life with different #threads"
echo "to collect data for time/threads plots"

# Create a folder with today's date
today=$(date +%Y-%m-%d)
output_dir="./outputs/$today"
mkdir -p "$output_dir/timings"

output_file="$output_dir/timings/mini-openmp-scal-$SLURM_JOB_ID-thin.csv"

export OMP_PLACES=cores
export OMP_PROC_BIND=close
echo "OMP_PLACES = $OMP_PLACES , OMP_PROC_BIND = $OMP_PROC_BIND"

echo "dim,nthreads,time" > "$output_file"

srun make clean
srun make

# for dim in 1000 2000 4000 8000 16000
for dim in 5000 10000
# for dim in 100 300
do
        echo "Using dimension $dim"
        echo "Running with 1 to 64 cores per socket"
        # Run the loop max_num_threads times
        for i in 8 16
        do
            echo "Running with $i threads"
            export OMP_NUM_THREADS=$i

            output=$(mpirun -n 1 --map-by socket ./build/gol.x -i -e $evolution -n $num_steps -k $dim -s 0)
            time=$(echo "$output" | tail -n 1)
            echo "$dim,$i,$time" >> "$output_file"

            echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
        done
done

echo "Finished"
