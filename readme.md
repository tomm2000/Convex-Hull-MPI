to generate random points: \
- `g++ randompoints.cpp -o randompoints.out` (compile only once)
- `./randompoints.out [#points=100]` \
or
- `g++ randompoints.cpp -o randompoints.out && ./randompoints.out [#points=100]`

to run the algorithm: \
- `mpicxx -o QuickHullMPI.out QuickHullMPI.cxx` (compile only once)
- `mpirun -n 4 ./QuickHullMPI.out` \
or
- `mpicxx -o QuickHullMPI.out QuickHullMPI.cxx && mpirun -n 4 ./QuickHullMPI.out`

