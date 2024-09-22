#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=MKL-matrix-scaling
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=12
#SBATCH --ntasks-per-node=1 
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "mkl matrix scaling"
echo "********************************"
export LD_LIBRARY_PATH=/u/dssc/ppette00/intel/oneapi/mkl/2024.2/lib/intel64:$LD_LIBRARY_PATH
export OMP_NUM_THREADS=12
export OMP_PLACES=threads

output_file="./outputs/matrix_scaling/mkl-matrx-scaling-$SLURM_JOB_ID.csv"

# Add header
echo "Measurement,Matrix Size (n),Seconds,GFLOPS,Precision,Bind"> "$output_file"

for binding in 'close' 'spread'
do
	echo "Binding $binding"
	export OMP_PROC_BIND=$binding
	for ((j=1; j<=5;j+=1))
	do
		echo "Measurement $j"
	
		for((i=2000; i <= 20000; i+=500))
		do
			echo "Iteration $i"
		
			mkl_out=$(srun -n1 --cpus-per-task=12 ./gemm_mkl_single.x $i $i $i)
			
			# Extract the relevant information (seconds and GFLOPS)
		        seconds=$(echo "$mkl_out" | tail -n 1 | awk '{print $2}')
		        gflops=$(echo "$mkl_out" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Single,$binding" >> "$output_file"
		
			mkl_out=$(srun -n1 --cpus-per-task=12 ./gemm_mkl_double.x $i $i $i)
			
			# Extract the relevant information (seconds and GFLOPS)
		        seconds=$(echo "$mkl_out" | tail -n 1 | awk '{print $2}')
		        gflops=$(echo "$mkl_out" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Double,$binding" >> "$output_file"
		
		done
	done
done
#
echo "Finished"
