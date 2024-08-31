#!/bin/bash
#SBATCH --job-name=progetto_scpd       # Job name
#SBATCH --output=output/output_%j.txt       # Output file (%j expands to jobId)
#SBATCH --nodes=12
#SBATCH --ntasks=432
#SBATCH --nodelist=broadwell-[015-022]
#SBATCH --ntasks-per-node=36
#SBATCH --time=00:20:00              # Time limit hh:mm:ss
#SBATCH --partition=broadwell        # Partition name

make clean
make
srun -pmix=mpi build/main npoints=2000000000 hybrid=false seed=1
