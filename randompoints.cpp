/*
mpicxx -o randompoints randompoints.cxx && mpirun -n 4 ./randompoints
*/

#include <iostream>
#include <fstream>
using namespace std;

// #define TIMING
// #define DEBUG

#define RADIUS 100

int main(int argc, char *argv[]) {
  clock_t start_time, end_time;
  double elapsed_time;

  start_time = clock(); 

  const int NPOINTS = 10000000;

  // create a file to write the points
  ofstream file;
  file.open("points.bin");

  // write the number of points
  file.write((char*)&NPOINTS, sizeof(int));

  // write the points
  for (int i = 0; i < NPOINTS; i++) {
    int x = rand() % (2 * RADIUS) - RADIUS;
    int y = rand() % (2 * RADIUS) - RADIUS;
    file.write((char*)&x, sizeof(int));
    file.write((char*)&y, sizeof(int));
  }

  file.close();

  end_time = clock();
  elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
  cout << "Time to generate the points: " << elapsed_time * 1000 << "ms" << endl;

  return 0;
}