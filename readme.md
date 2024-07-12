to generate random points: \
- `g++ randompoints.cpp -o build/randompoints -O3` (compile only once)
- `./build/randompoints [#points=100]` \
or
- `g++ randompoints.cpp -o build/randompoints -O3 && ./build/randompoints [#points=100]`

to run the algorithm: \
- `mpicxx QuickHullMPI.cxx -o build/QuickHullMPI -O3` (compile only once)
- `mpirun -n 4 ./build/QuickHullMPI` \
or
- `mpicxx QuickHullMPI.cxx -o build/QuickHullMPI -O3 && mpirun -n 4 ./build/QuickHullMPI`