/*
> mpicxx -o QuickHullMPI.out QuickHullMPI.cxx && mpirun -n 4 ./QuickHullMPI.out
*/
#include "lib.hxx"
#include "QuickHullSequential.hxx"

using namespace std;

// #define TIMING
// #define DEBUG

int main(int argc, char *argv[]) { 
  clock_t glob_start_time, glob_end_time;
  double glob_elapsed_time;

  #pragma region 1. Input
  //================= 1. Read points from file =================
  #ifdef TIMING
  clock_t start_time = clock();
  #endif
  #ifdef DEBUG
  cout << "<process " << rank << "> Number of points total: " << numPoints << endl;
  #endif

  string filename = "points.bin";
  ifstream file(filename, ios::binary | ios::ate);

  if (!file.is_open()) {
    cout << "Error opening file" << endl;
    return 1;
  }

  file.seekg(0, ios::beg);

  int numPoints;
  file.read((char *) &numPoints, sizeof(int));

  Point *points = new Point[numPoints];
  file.read((char *) points, numPoints * 2 * sizeof(int));

  file.close();

  #ifdef DEBUG
  cout << "read " << numPoints << " points" << endl;
  #endif

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  cout << "Init time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  //================================================================
  #pragma endregion

  vector<Point> hull = vector<Point>();

  QuickHullInit_sequential(points, numPoints, hull);

  cout << "Hull size: " << hull.size() << endl;

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  cout << "Total time: " << glob_elapsed_time * 1000.0 << "ms" << endl;

  return 0;
}
