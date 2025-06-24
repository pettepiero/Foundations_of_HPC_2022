#!/bin/bash
#SBATCH -A dssc
#SBATCH --job-name=openmp-scal
#SBATCH --partition=THIN
#SBATCH --nodelist=thin002
#SBATCH --time=01:00:0
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --output=./outputs/slurm-%j-openmp-scal.txt

module load openMPI/4.1.6
# 0 -> ordered, 1 -> static
evolution=1
if [ "$evolution" -eq 0 ]; then
    echo "ordered evolution"
fi
if [ "$evolution" -eq 1 ]; then
	echo "static evolution"
fi

echo "Running game of life serially"
export OMP_NUM_THREADS=1
export OMP_PROC_BIND=false
export OMP_PLACES=threads

# Create a folder with today's date
today=$(date +%Y-%m-%d)
output_dir="./outputs/$today"
mkdir -p "$output_dir/timings"

output_file="$output_dir/timings/serial-$SLURM_JOB_ID.csv"

echo "OMP_PLACES = $OMP_PLACES , OMP_PROC_BIND = $OMP_PROC_BIND , OMP_NUM_THREADS = $OMP_NUM_THREADS"

echo "dim,time" > "$output_file"
for dim in 1000 2000 4000 8000 16000
do
        echo "Using dimension $dim"
        
        output=$(./build/serial-gol.x -i -e $evolution -k $dim -s 0)
        time=$(echo "$output" | tail -n 1)
        echo "$dim,$time" >> "$output_file"

        echo "$output" >> "./outputs/slurm-$SLURM_JOB_ID.txt"
	gprof_output_file="$output_dir/profile-$SLURM_JOB_ID-$dim.txt"
	gprof ./build/gol.x gmon.out > "$gprof_output_file"
	mv gmon.out "$output_dir/gmon-$SLURM_JOB_ID-$dim.out"
done

echo "Finished"
