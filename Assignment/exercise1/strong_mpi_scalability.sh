#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="Strong_scal"
#SBATCH --partition=EPYC
#SBATCH --nodes=4
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=64
#SBATCH --time=02:00:00
#SBATCH --output=./outputs/slurm-%j-strong-mpi.txt

module load architecture/AMD
module load openMPI/4.1.5/gnu/12.2.1

export OMP_PLACES=cores
export OMP_PROC_BIND=close
export MPI_PLACES=sockets
export MPI_PROC_BIND=close

echo "MPI Strong Scalability Test"
echo "Running fixed problem size with different #tasks"
echo "to collect data for time/tasks plots"

output_file="./outputs/timings/time-tasks-$SLURM_JOB_ID.csv"
fixed_problem_size=1000 

echo "tasks,time" > "$output_file"

for tasks in 1 2 4 8 16 32 64
do
    echo "Running with $tasks tasks"
    output=$(mpirun -np $tasks ./build/mpi_gol.x -p $fixed_problem_size)
    
    time=$(echo "$output" | tail -n 1)
    echo "$tasks,$time" >> "$output_file"
    
    echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
done

echo "Finished"
