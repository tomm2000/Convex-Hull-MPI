#!/bin/bash
#SBATCH --job-name=progetto_scpd
#SBATCH --output=output/output_%j.txt
#SBATCH --nodes=48
#SBATCH --ntasks=1728
#SBATCH --nodelist=broadwell-[015-064]
#SBATCH --ntasks-per-node=36
#SBATCH --time=00:10:00
#SBATCH --partition=broadwell

# npoints variable to be used by the program
export npoints=2000000000

# make clean
make
# run the experiment 5 times, print "====================" to separate the results
srun -pmix=mpi build/main npoints=$npoints seed=1 hybrid=false
echo "===================="
srun -pmix=mpi build/main npoints=$npoints seed=2 hybrid=false
echo "===================="
srun -pmix=mpi build/main npoints=$npoints seed=3 hybrid=false
echo "===================="
srun -pmix=mpi build/main npoints=$npoints seed=4 hybrid=false
echo "===================="
srun -pmix=mpi build/main npoints=$npoints seed=5 hybrid=false


