#!/bin/bash
#SBATCH --job-name=openmp_test       # Job name
#SBATCH --output=output/output_%j.txt       # Output file (%j expands to jobId)
#SBATCH --nodes=12
#SBATCH --ntasks=12
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=35
#SBATCH --time=00:20:00              # Time limit hh:mm:ss
#SBATCH --partition=broadwell        # Partition name

# Export the number of threads to be used by OpenMP
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

make clean
make
srun -pmix=mpi build/main npoints=2000000000 hybrid=true seed=1

