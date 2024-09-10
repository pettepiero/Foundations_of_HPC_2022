#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=MKL-core-scaling
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --ntasks-per-node=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=24
#SBATCH --mem=200gb
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt
#SBATCH --error=./errors/errors-%j.err

echo "MKL core scaling"
echo "********************************"

export LD_LIBRARY_PATH=/u/dssc/ppette00/intel/oneapi/mkl/2024.2/lib/intel64:$LD_LIBRARY_PATH

output_file="./outputs/core_scaling/mkl-core-scaling-$SLURM_JOB_ID.csv"

# Add header
echo "Number of CPUs,Seconds,GFLOPS,Precision"> "$output_file"


for ((i=2;i<=24;i+=2))
do

	echo "Iteration $i"

	output=$(srun -n1 --cpus-per-task=$i ./gemm_mkl_single.x 10000 10000 10000)
	
	# Extract relevant information
	seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
	gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')

	echo "$i, $seconds, $gflops,Single" >> "$output_file"
done

echo "Finished"

