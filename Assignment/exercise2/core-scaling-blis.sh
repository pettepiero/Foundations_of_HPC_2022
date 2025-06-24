#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=blis-core-scaling
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=12
#SBATCH --ntasks-per-node=2
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "BLIS core scaling"
echo "Expecting already compiled executables for THIN architecture"
echo "********************************"
echo "Loading modules"

export LD_LIBRARY_PATH=/u/dssc/ppette00/myblis/lib:$LD_LIBRARY_PATH
export OMP_PLACES=sockets

if [ "$1" == "-s" ]; then
    	output_file="./outputs/core_scaling/blis-scal-$SLURM_JOB_ID-serial.csv"
else
	output_file="./outputs/core_scaling/blis-core-scaling-$SLURM_JOB_ID-init.csv"
fi

matrix_size=10000

# Add header
echo "Measurement,Number of CPUs,Seconds,GFLOPS,Precision,Bind(threads)"> "$output_file"

bindings_tested="close spread"
bindings_list=$(echo $bindings_tested | tr ' ' ',')
data="$SLURM_JOB_ID,$OMP_PLACES,$matrix_size,$bindings_list,$output_file"
echo "$data" > "./outputs/metadata/blis-metadata-$SLURM_JOB_ID.txt"

for binding in $bindings_tested 
do
	echo "Binding $binding"
	export OMP_PROC_BIND=$binding
	for ((j=1; j<=5;j+=1))
	do
		echo "Measurement $j"
	
		for i in 1 2 4 8 12 16 20 24 
		do
			echo "Iteration $i"
			export BLIS_NUM_THREADS=$i # check out: https://github.com/flame/blis/blob/master/docs/Multithreading.md
			
			output=$(srun -n1 --cpus-per-task=$i ./gemm_blis_single.x $matrix_size $matrix_size $matrix_size)
			
			# Extract relevant information
			seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
			gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
			
			echo "$j,$i,$seconds,$gflops,Single,$binding" >> "$output_file"
			
			output=$(srun -n1 --cpus-per-task=$i ./gemm_blis_double.x $matrix_size $matrix_size $matrix_size)
			
			# Extract relevant information
			seconds=$(echo "$output" | tail -n 1 | awk '{print $2}')
			gflops=$(echo "$output" | tail -n 1 | awk '{print $4}')
			
			echo "$j,$i,$seconds,$gflops,Double,$binding" >> "$output_file"
		done
	done
done
echo "Finished"

