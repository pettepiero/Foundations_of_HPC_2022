#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=mpi-scal
#SBATCH --partition=THIN
#SBATCH --nodelist=thin[001-003,008]
#SBATCH --time=02:00:0
#SBATCH --ntasks-per-node=2 
#SBATCH --cpus-per-task=12
#SBATCH --nodes=4
#SBATCH --exclusive
#SBATCH --output=./outputs/slurm-%j-mpi-scal.txt
module load openMPI/4.1.6
# 0 -> ordered, 1 -> static
evolution=1
if [ "$evolution" = 0 ]; then
    echo "MPI scalability for ordered evolution"
fi

if [ "$evolution" = 1 ]; then
    echo "MPI scalability for static evolution"
fi

echo "Running game of life with different #MPI tasks"
echo "to collect data for time/tasks plots for THIN nodes"

# Create a folder with today's date
today=$(date +%Y-%m-%d)
output_dir="./outputs/$today"
mkdir -p "$output_dir/timings"

output_file="$output_dir/timings/mpi-scal-$SLURM_JOB_ID.csv"

mapping="socket"

echo "mapping by $mapping"
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=12

num_steps=500

echo "dim,mpi-tasks,time" > "$output_file"
for dim in 5000 10000 15000 20000 25000 30000; do
#for dim in 20000 40000 80000; do 
	for ntasks in 1 2 4 6 8; do
		echo "Running $ntasks MPI tasks"
		    
		output=$(mpirun --map-by $mapping -n $ntasks ./build/gol.x -i -n $num_steps -e $evolution -k $dim -s 0)
		    
	        time=$(echo "$output" | tail -n 1)
	        echo "$dim,$ntasks,$time" >> "$output_file"
	        echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
		echo "Finished $ntasks tasks run"
	done
done

echo "Finished"
