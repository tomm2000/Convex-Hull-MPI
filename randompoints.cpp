/*
g++ randompoints.cpp -o randompoints && ./randompoints
*/

#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;

// #define TIMING
// #define DEBUG

#define RADIUS 200

int main(int argc, char *argv[]) {
  // if there are 2 arguments, the first one is the number of points
  int NPOINTS = 1000;
  if (argc == 2) {
    NPOINTS = atoi(argv[1]);
  }

  clock_t start_time, end_time;
  double elapsed_time;

  start_time = clock(); 

  // create a file to write the points
  ofstream file;
  file.open("points.bin");

  // write the number of points
  file.write((char*)&NPOINTS, sizeof(int));

  srand(time(NULL));

  // write the points
  for (int i = 0; i < NPOINTS; i++) {
    // generate points uniformly distributed in a circle
    double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;
    double r = sqrt((double) rand() / RAND_MAX) * RADIUS;

    int x = r * cos(angle);
    int y = r * sin(angle);

    file.write((char*)&x, sizeof(int));
    file.write((char*)&y, sizeof(int));
  }

  file.close();

  end_time = clock();
  elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
  cout << "Time to generate " << NPOINTS << " points: " << elapsed_time * 1000.0 << "ms" << endl;

  return 0;
}