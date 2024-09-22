#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=oblas-core-scaling
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=24
#SBATCH --ntasks-per-node=1 
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "OpenBLAS core scaling"
echo "Expecting already compiled executables for THIN architecture"
echo "********************************"
echo "Loading modules"
module load openBLAS/0.3.26-omp

OPENBLASROOT=${OPENBLAS_ROOT}
export OMP_PLACES=threads

output_file="./outputs/core_scaling/oblas-core-scaling-$SLURM_JOB_ID.csv"
matrix_size=10000
# Add header
echo "Measurement,Number of CPUs,Seconds,GFLOPS,Precision,Bind(threads)"> "$output_file"

for binding in 'close' 'spread'
do
	export OMP_PROC_BIND=$binding
	for ((j=1;j<=5;j+=1))
	do
		echo "Measurement $j"
	
		for ((i=1;i<=24;i+=1))
		do
	
			echo "Iteration $i"
	
			output=$(srun -n1 --cpus-per-task=$i ./gemm_oblas_single.x $matrix_size $matrix_size $matrix_size)
	
			# Extract relevant information
			seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
			gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
	
			echo "$j,$i,$seconds,$gflops,Single,$binding" >> "$output_file"
	
			output=$(srun -n1 --cpus-per-task=$i ./gemm_oblas_double.x $matrix_size $matrix_size $matrix_size)
	
			# Extract relevant information
			seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
			gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
	
			echo "$j,$i,$seconds,$gflops,Double,$binding" >> "$output_file"
		done
	done
done
echo "Finished"

