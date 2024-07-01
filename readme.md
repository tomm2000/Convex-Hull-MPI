
to generate the points: \
- `mpicxx -o randompoints randompoints.cxx` (compile only once)
- `mpirun -n 4 ./randompoints` \
or
- `mpicxx -o randompoints randompoints.cxx && mpirun -n 4 ./randompoints`

to run the algorithm: \
- `g++ randompoints.cpp -o randompoints` (compile only once)
- `./randompoints` \
or
- `g++ randompoints.cpp -o randompoints && ./randompoints`