#!/bin/bash
#SBATCH -A dssc
#SBATCH --partition=THIN
#SBATCH --time=0:30:0
#SBATCH --cpus-per-task=24
#SBATCH --ntasks-per-node=1 
#SBATCH --nodes=1
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "OpenBLAS core scaling"
echo "********************************"
echo "Loading modules"
module load openBLAS/0.3.26-omp

OPENBLASROOT=${OPENBLAS_ROOT}

output_file="./outputs/core_scaling/oblas-core-scaling-$SLURM_JOB_ID.csv"

# Add header
echo "Number of CPUs,Seconds,GFLOPS,Precision"> "$output_file"


for ((i=2;i<=24;i+=2))
do

	echo "Iteration $i"

	output=$(srun -n1 --cpus-per-task=$i ./gemm_oblas_single.x 10000 10000 10000)

	# Extract relevant information
	seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
	gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')

	echo "$i, $seconds, $gflops,Single" >> "$output_file"
done

echo "Finished"

