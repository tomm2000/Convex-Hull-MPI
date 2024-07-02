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

