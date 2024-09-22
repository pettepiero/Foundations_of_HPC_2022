#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=OBLAS-matrix-scaling
#SBATCH --partition=THIN
#SBATCH --time=02:00:0
#SBATCH --cpus-per-task=12
#SBATCH --ntasks-per-node=1 
#SBATCH --mem=200gb
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --output=./outputs/output_files/slurm-%j.txt

echo "OpenBLAS matrix scaling"
echo "********************************"
echo "Loading modules"
module load openBLAS/0.3.26-omp
OPENBLASROOT=${OPENBLAS_ROOT}
export OMP_NUM_THREADS=12
export OMP_PLACES=threads

output_file="./outputs/matrix_scaling/oblas-matrx-scaling-$SLURM_JOB_ID.csv"

# Add header
echo "Measurement,Matrix Size (n),Seconds,GFLOPS,Precision,Bind(threads)"> "$output_file"

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
		
			out=$(srun -n1 --cpus-per-task=12 ./gemm_oblas_single.x $i $i $i)
			
			# Extract the relevant information (seconds and GFLOPS)
		        seconds=$(echo "$out" | tail -n 1 | awk '{print $2}')
		        gflops=$(echo "$out" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Single,$binding" >> "$output_file"
		
			out=$(srun -n1 --cpus-per-task=12 ./gemm_oblas_double.x $i $i $i)
			
			# Extract the relevant information (seconds and GFLOPS)
		        seconds=$(echo "$out" | tail -n 1 | awk '{print $2}')
		        gflops=$(echo "$out" | tail -n 1 | awk '{print $4}')
		
			echo "$j,$i,$seconds,$gflops,Double,$binding" >> "$output_file"
		
		done
	done
done
#
echo "Finished"
