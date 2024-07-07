to generate random points: \
- `g++ randompoints.cpp -o randompoints.out -O3` (compile only once)
- `./randompoints.out [#points=100]` \
or
- `g++ randompoints.cpp -o randompoints.out -O3 && ./randompoints.out [#points=100]`

to run the algorithm: \
- `mpicxx QuickHullMPI.cxx -o QuickHullMPI.out -O3` (compile only once)
- `mpirun -n 4 ./QuickHullMPI.out` \
or
- `mpicxx QuickHullMPI.cxx -o QuickHullMPI.out -O3 && mpirun -n 4 ./QuickHullMPI.out`



segmentation fault: \
seed: 1720011655 points: 10000000 tasks: 12
./randompoints.out 10000000 1720011655
srun --mpi=pmix --nodes 1 --ntasks 12 --ntasks-per-node 12 QuickHullMPI.out