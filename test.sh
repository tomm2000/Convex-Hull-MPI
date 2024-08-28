#!/bin/bash
#SBATCH --job-name=openmp_test       # Job name
#SBATCH --output=output/output_%j.txt       # Output file (%j expands to jobId)
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --time=00:10:00              # Time limit hh:mm:ss
#SBATCH --partition=broadwell        # Partition name

# Load necessary modules (if any)
# module load gcc

# Export the number of threads to be used by OpenMP
export OMP_NUM_THREADS=16

# Run the OpenMP program
make clean
make
build/main npoints=40000000
