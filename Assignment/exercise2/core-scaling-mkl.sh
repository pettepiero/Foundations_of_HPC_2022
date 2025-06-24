#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=MKL-core-scaling
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=12
#SBATCH --ntasks-per-node=1 
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "MKL core scaling"
echo "Expecting already compiled executables for THIN architecture"
echo "********************************"
echo "Loading modules"

export LD_LIBRARY_PATH=/u/dssc/ppette00/intel/oneapi/mkl/2024.2/lib/intel64:$LD_LIBRARY_PATH
export OMP_PLACES=threads

if [ "$1" == "-s" ]; then
    	output_file="./outputs/core_scaling/mkl-scal-$SLURM_JOB_ID-serial.csv"
else
	output_file="./outputs/core_scaling/mkl-core-scaling-$SLURM_JOB_ID-init.csv"
fi

matrix_size=10000

# Add header
echo "Measurement,Number of CPUs,Seconds,GFLOPS,Precision,Bind(threads)"> "$output_file"

bindings_tested="close spread"
bindings_list=$(echo $bindings_tested | tr ' ' ',')
data="$SLURM_JOB_ID,$OMP_PLACES,$matrix_size,$bindings_list,$output_file"
echo "$data" > "./outputs/metadata/mkl-metadata-$SLURM_JOB_ID.txt"

for binding in $bindings_tested 
do
	export OMP_PROC_BIND=$binding
	for ((j=1; j<=5;j+=1))
	do
		echo "Measurement $j"
	
		for ((i=1;i<=12;i+=1))
		do
			echo "Iteration $i"
		
			output=$(srun -n1 --cpus-per-task=$i ./gemm_mkl_single.x $matrix_size $matrix_size $matrix_size)
		
			# Extract relevant information
			seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
			gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Single,$binding" >> "$output_file"
		
			output=$(srun -n1 --cpus-per-task=$i ./gemm_mkl_double.x $matrix_size $matrix_size $matrix_size)
		
			# Extract relevant information
			seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
			gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Double,$binding" >> "$output_file"
		
		done
	done
done
echo "Finished"
