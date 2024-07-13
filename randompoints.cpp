/*
g++ randompoints.cpp -o build/randompoints -O3 && ./build/randompoints [#points=100]
*/

#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
using namespace std;

// #define TIMING
// #define DEBUG

#define POINTS_CIRCLE
// #define POINTS_TORUS
// #define POINTS_GAUSSIAN
// #define POINTS_CIRCUMFERENCE
// #define POINTS_SQUARE

#define RADIUS 1000000
#define TORUS_STROKE RADIUS / 4

int main(int argc, char *argv[]) {
  // if there are 2 arguments, the first one is the number of points
  size_t NPOINTS = 1000;

  int seed = 0;

  if (argc == 2) {
    NPOINTS = stol(argv[1]);
  } else if (argc == 3) {
    NPOINTS = stol(argv[1]);
    seed = atoi(argv[2]);
  } else {
    seed = time(NULL);
  }

  cout << "Generating " << NPOINTS << " points with seed " << seed << endl;

  clock_t start_time, end_time;
  double elapsed_time;

  start_time = clock(); 

  // create a file to write the points
  ofstream file;
  file.open("points.bin");

  // first 4 bytes is the number of bytes used by size_t
  int size_t_bytes = sizeof(size_t);
  file.write(reinterpret_cast<char*>(&size_t_bytes), sizeof(int));

  // second 4 bytes is the size of each point
  int point_size = sizeof(long) * 2;
  file.write(reinterpret_cast<char*>(&point_size), sizeof(int));

  // write the number of points
  file.write(reinterpret_cast<char*>(&NPOINTS), sizeof(size_t));

  srand(seed);

  // write the points
  const size_t BUFFER_SIZE = 100;
  long buffer[BUFFER_SIZE * 2];

  for (size_t i = 0; i < NPOINTS; i += BUFFER_SIZE) {
    // int pointsToWrite = min(BUFFER_SIZE, NPOINTS - i);
    size_t pointsToWrite = BUFFER_SIZE;
    if (i + pointsToWrite > NPOINTS) { pointsToWrite = NPOINTS - i; }

    for (size_t j = 0; j < pointsToWrite; j++) {

      #ifdef POINTS_CIRCLE
      // generate points uniformly distributed in a circle
      double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;
      double r = sqrt((double) rand() / RAND_MAX) * RADIUS;

      long x = r * cos(angle);
      long y = r * sin(angle);
      #endif

      #ifdef POINTS_TORUS
      // generate points uniformly distributed in a torus
      double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;

      // radius should be between RADIUS + TORUS_STROKE and RADIUS - TORUS_STROKE
      double r = (RADIUS - TORUS_STROKE) + (double) rand() / RAND_MAX * 2 * TORUS_STROKE;

      long x = r * cos(angle);
      long y = r * sin(angle);
      #endif

      #ifdef POINTS_GAUSSIAN
      // generate points uniformly distributed in a gaussian
      double x = 0;
      double y = 0;

      for (int k = 0; k < 12; k++) {
        x += (double) rand() / RAND_MAX;
        y += (double) rand() / RAND_MAX;
      }

      x -= 6;
      y -= 6;

      x *= RADIUS;
      y *= RADIUS;

      x = (long) x;
      y = (long) y;
      #endif
      
      #ifdef POINTS_CIRCUMFERENCE
      // generate points uniformly distributed in a circumference
      double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;

      long x = RADIUS * cos(angle);
      long y = RADIUS * sin(angle);
      #endif

      #ifdef POINTS_SQUARE
      // generate points uniformly distributed in a square
      long x = (rand() % (2 * RADIUS)) - RADIUS;
      long y = (rand() % (2 * RADIUS)) - RADIUS;
      #endif


      buffer[j * 2] = x;
      buffer[j * 2 + 1] = y;
    }

    file.write(reinterpret_cast<char*>(buffer), sizeof(long) * pointsToWrite * 2);
  }

  file.close();

  end_time = clock();
  elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
  
  // Time to generate {} points: {}ms, seed: {}
  cout << "Time to generate " << NPOINTS << " points: " << elapsed_time * 1000 << "ms, seed: " << seed << endl;

  return 0;
}