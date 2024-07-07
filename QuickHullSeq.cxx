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

  #pragma region 2. extremes
  //================= 2. Find the 2 extreme points =================
  glob_start_time = clock();

  #ifdef TIMING
  start_time = clock();
  #endif

  // find the extreme points
  Point left = points[0], right = points[0];

  for (int i = 1; i < numPoints; i++) {
    if (points[i].x < left.x) {
      left = points[i];
    } else if (points[i].x > right.x) {
      right = points[i];
    }
  }

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "Time to find extreme points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  #ifdef DEBUG
  cout << "Left: " << left.toString() << ", right: " << right.toString() << endl;
  #endif
  
  //================================================================
  #pragma endregion

  #pragma region 3. Split points
  //================= 3. Split points into two halves =================
  #ifdef TIMING
  start_time = clock();
  #endif

  vector<Point> upperHalf, lowerHalf = vector<Point>();
  upperHalf.reserve(numPoints / 2);
  lowerHalf.reserve(numPoints / 2);

  float dx = right.x - left.x;
  float dy = right.y - left.y;

  for (int i = 0; i < numPoints; i++) {
    // if its the left or right point, skip it
    if (points[i].x == left.x && points[i].y == left.y) { continue; }
    if (points[i].x == right.x && points[i].y == right.y) { continue; }
    
    float line = dy * (points[i].x - left.x) - dx * (points[i].y - left.y);

    if (line > 0) {
      upperHalf.push_back(points[i]);
    } else if (line < 0) {
      lowerHalf.push_back(points[i]);
    }
  }

  // FIXME: is this free necessary?
  free(points);

  #ifdef DEBUG
  cout << "Upper half: " << upperHalf.size() << ", lower half: " << lowerHalf.size() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "Time to split points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  #pragma region 4. Recursion
  //================= 4. Recurse on the two halves =================
  vector<Point> hull;

  hull.push_back(right);
  hull.push_back(left);

  QuickHull_sequential(upperHalf, left, right, hull, 0);
  QuickHull_sequential(lowerHalf, right, left, hull, 0);
  //================================================================
  #pragma endregion

  cout << "Hull size: " << hull.size() << endl;

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  cout << "Total time: " << glob_elapsed_time * 1000.0 << "ms" << endl;

  return 0;
}
