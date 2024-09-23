#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=mpi-scal
#SBATCH --partition=THIN
#SBATCH --time=01:00:0
#SBATCH --ntasks-per-node=2 
#SBATCH --cpus-per-task=12
#SBATCH --nodes=4
#SBATCH --exclusive
#SBATCH --output=./outputs/slurm-%j-mpi-scal.txt

module load openMPI/4.1.6/gnu/14.2.1

echo "MPI scalability"
echo "Running game of life with different #MPI tasks"
echo "to collect data for time/tasks plots"

output_file="./outputs/timings/mpi-scal-$SLURM_JOB_ID.csv"
max_num_threads=12
mapping="node"
binding="socket"
echo "mapping by $mapping, binding by $binding"
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=12

# Outer loop repeats the measurements for different matrix dimensions

if [ "$1" == "-s" ]; then
    output_file="./outputs/timings/openmp-scal-$SLURM_JOB_ID-serial.csv"
fi
echo "dim,mpi-tasks,time" > "$output_file"
for dim in 1000 2000 4000 8000 16000 20000
do
	for ntasks in 1 2 3 4 5 6 7 8; do
		echo "Running $ntasks MPI tasks"
		    
		output=$(mpirun --map-by $mapping --bind-to $binding -n $ntasks ./build/gol.x -i -k $dim)
		    
	        time=$(echo "$output" | tail -n 1)
	        echo "$dim,$ntasks,$time" >> "$output_file"
	        echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID-serial.txt"
		echo "Finished $ntasks tasks run"
	done
done

echo "Finished"
