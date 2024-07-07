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

  int seed = 0;

  if (argc == 3) {
    NPOINTS = atoi(argv[1]);
    seed = atoi(argv[2]);
  } else {
    seed = time(NULL);
  }

  clock_t start_time, end_time;
  double elapsed_time;

  start_time = clock(); 

  // create a file to write the points
  ofstream file;
  file.open("points.bin");

  // write the number of points
  file.write((char*)&NPOINTS, sizeof(int));

  srand(seed);

  // write the points
  const int BUFFER_SIZE = 100;
  int buffer[BUFFER_SIZE * 2];

  for (int i = 0; i < NPOINTS; i += BUFFER_SIZE) {
    int pointsToWrite = min(BUFFER_SIZE, NPOINTS - i);

    for (int j = 0; j < pointsToWrite; j++) {
      // generate points uniformly distributed in a circle
      double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;
      double r = sqrt((double) rand() / RAND_MAX) * RADIUS;

      int x = r * cos(angle);
      int y = r * sin(angle);

      buffer[j * 2] = x;
      buffer[j * 2 + 1] = y;
    }

    file.write((char*)buffer, sizeof(int) * pointsToWrite * 2);
  }

  file.close();

  end_time = clock();
  elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
  
  // Time to generate {} points: {}ms, seed: {}
  cout << "Time to generate " << NPOINTS << " points: " << elapsed_time * 1000 << "ms, seed: " << seed << endl;

  return 0;
}