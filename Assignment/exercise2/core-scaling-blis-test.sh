#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=BLIS-core-scaling
#SBATCH --partition=EPYC
#SBATCH --time=0:30:0
#SBATCH -n 128
#SBATCH -N1
#SBATCH --output=./outputs/output_files/slurm-%j.txt
#SBATCH --error=./errors/errors-%j.err
#SBATCH --mail-user=ppette00

echo "BLIS core scaling"
echo "********************************"
echo "Loading modules"

export LD_LIBRARY_PATH=/u/dssc/ppette00/myblis/lib:$LD_LIBRARY_PATH

output_file="./outputs/core_scaling/blis-core-scaling-$SLURM_JOB_ID.csv"

# Add header
echo "Number of CPUs,Seconds,GFLOPS,Precision"> "$output_file"


for ((i=2;i<=128;i=i*2))
do

	echo "Iteration $i"
	export BLIS_NUM_THREADS=$i # check out: https://github.com/flame/blis/blob/master/docs/Multithreading.md

	output=$(srun -n1 --cpus-per-task=$i ./gemm_blis_single.x 10000 10000 10000)

	# Extract relevant information
	seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
	gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')

	echo "$i, $seconds, $gflops,Single" >> "$output_file"
done

echo "Finished"

