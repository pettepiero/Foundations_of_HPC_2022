#!/bin/bash
#SBATCH -A dssc
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=24
#SBATCH --ntasks-per-node=1 
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "BLIS core scaling"
echo "Expecting already compiled executables for THIN architecture"
echo "********************************"
echo "Loading modules"

export LD_LIBRARY_PATH=/u/dssc/ppette00/myblis/lib:$LD_LIBRARY_PATH
export OMP_PLACES=cores
export OMP_PROC_BIND=close

output_file="./outputs/core_scaling/blis-core-scaling-$SLURM_JOB_ID.csv"
matrix_size=10000

# Add header
echo "Measurement,Number of CPUs,Seconds,GFLOPS,Precision"> "$output_file"


for ((j=1; j<=20;j+=1))
do
	echo "Measurement $j"

	for ((i=1;i<=24;i+=1))
	do

		echo "Iteration $i"
		export BLIS_NUM_THREADS=$i # check out: https://github.com/flame/blis/blob/master/docs/Multithreading.md
	
		output=$(srun -n1 --cpus-per-task=$i ./gemm_blis_single.x $matrix_size $matrix_size $matrix_size)
	
		# Extract relevant information
		seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
		gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
	
		echo "$j, $i, $seconds, $gflops,Single" >> "$output_file"
	
		output=$(srun -n1 --cpus-per-task=$i ./gemm_blis_double.x $matrix_size $matrix_size $matrix_size)
	
		# Extract relevant information
		seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
		gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
	
		echo "$j, $i, $seconds, $gflops,Double" >> "$output_file"
	done
done

echo "Finished"

