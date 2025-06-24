#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=BLIS-matrix-scaling
#SBATCH --partition=EPYC
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=64
#SBATCH --ntasks-per-node=1 
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "BLIS matrix scaling"
echo "********************************"

export LD_LIBRARY_PATH=/u/dssc/ppette00/myblis/lib:$LD_LIBRARY_PATH
export BLIS_NUM_THREADS=64
export OMP_NUM_THREADS=64
export OMP_PLACES=threads
output_file="./outputs/matrix_scaling/blis-matrx-scaling-$SLURM_JOB_ID-EPYC.csv"

# Add header
echo "Measurement,Matrix Size (n),Seconds,GFLOPS,Precision,Bind"> "$output_file"

for binding in 'close' 'spread'
do
	echo "Binding $binding"
	export OMP_PROC_BIND=$binding
	# Single precision scaling
	for ((j=1; j<=5;j+=1))
	do
		echo "Measurement $j"
	
		for((i=2000; i <= 20000; i+=500))
		do
			echo "Iteration $i"
		
			out=$(srun -n1 --cpus-per-task=64 ./gemm_blis_single.x $i $i $i)
			
			# Extract the relevant information (seconds and GFLOPS)
		        seconds=$(echo "$out" | tail -n 1 | awk '{print $2}')
		        gflops=$(echo "$out" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Single,$binding" >> "$output_file"
		
			out=$(srun -n1 --cpus-per-task=64 ./gemm_blis_double.x $i $i $i)
			
			# Extract the relevant information (seconds and GFLOPS)
		        seconds=$(echo "$out" | tail -n 1 | awk '{print $2}')
		        gflops=$(echo "$out" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Double,$binding" >> "$output_file"
		
		done
	done
done
#
echo "Finished"
