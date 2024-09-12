#!/bin/bash
#SBATCH --job-name=progetto_scpd
#SBATCH --output=output/output_%j.txt
#SBATCH --nodes=4
#SBATCH --ntasks=4
#SBATCH --nodelist=broadwell-[015-064]
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=35
#SBATCH --time=01:00:00
#SBATCH --partition=broadwell

# Export the number of threads to be used by OpenMP
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

# npoints variable to be used by the program
export npoints=1000000000

make clean
make
# run the experiment 5 times, print "====================" to separate the results
srun -pmix=mpi build/main npoints=$npoints seed=1 hybrid=true

