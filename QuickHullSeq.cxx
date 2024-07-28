/*
make && mpirun -n 4 ./QuickHullMPI.out
*/
#include "lib.hxx"
#include "QuickHull.hxx"

using namespace std;

// #define TIMING
// #define DEBUG

// #define LOAD_FROM_FILE
#define RADIUS 100000000

int main(int argc, char *argv[]) { 
  clock_t glob_start_time, glob_end_time, start_time, end_time;
  double glob_elapsed_time, elapsed;

  glob_start_time = clock();

  #pragma region 1. Input
  // check if there is an argument
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <number of points>" << endl;
    return 1;
  }

  #pragma region 1. Input
  //================= 1. Generate random points =================
  #ifdef TIMING
  start_time = clock();
  #endif

  size_t NPOINTS = stol(argv[1]);
  int seed = clock();

  // if there is a second argument, use it as seed
  if (argc > 2) { seed = atoi(argv[2]); }

  Point *points = new Point[NPOINTS];

  int seed_seed = seed * 23415;
  srand(seed_seed);
  seed = rand();
  srand(seed);

  // generate points in a circle
  for (size_t i = 0; i < NPOINTS; i++) {
    // generate points uniformly distributed in a circle
    double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;
    double r = sqrt((double) rand() / RAND_MAX) * RADIUS;

    Point p;
    p.x = r * cos(angle);
    p.y = r * sin(angle);

    points[i] = p;
  }
  #pragma endregion
  
  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  cout << "Init time: " << glob_elapsed_time * 1000.0 << "ms" << endl;

  #pragma region 4. Local hull
  //================= 4. Recurse on the two halves =================
  glob_start_time = clock();

  vector<Point> hull;

  QuickHullInit(points, NPOINTS, hull);

  // FIXME: is this free necessary?
  free(points);

  if (hull.size() < 100000) {
    savePointsToFile(hull, "hull.txt");
  }

  cout << "Hull size: " << hull.size() << endl;

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  cout << "Elapsed time: " << glob_elapsed_time * 1000.0 << "ms" << endl;

  return 0;
}