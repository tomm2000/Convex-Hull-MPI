/*
> mpicxx -o QuickHullMPI.out QuickHullMPI.cxx && mpirun -n 4 ./QuickHullMPI.out
*/

#include "mpi.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <math.h>
#include <sstream>
using namespace std;

// #define TIMING
// #define DEBUG

enum messages {msg_tag,eos_tag,empty_points};

struct Point {
  int x, y;

  string toString() {
    return "[" + to_string(x) + ", " + to_string(y) + "]";
  }
};

void QuickHull(
  MPI_Datatype PointType, int numP, int rank,
  vector<Point> &points,
  Point a, Point b,
  vector<Point> &hull,
  int iteration
);

MPI_Datatype registerPointType() {
  // Create a new MPI datatype to represent a Point
  MPI_Datatype PointType;
  MPI_Datatype type[2] = {MPI_INT, MPI_INT};
  int blocklen[2] = {1, 1};
  MPI_Aint displacements[2];
  displacements[0] = offsetof(Point, x);
  displacements[1] = offsetof(Point, y);
  MPI_Type_create_struct(2, blocklen, displacements, type, &PointType);
  MPI_Type_commit(&PointType);

  return PointType;
}

class LineDistanceCalculator {
private:
    float dx, dy, denominator;
    Point a, b;

public:
    LineDistanceCalculator(const Point& a, const Point& b) {
        dx = b.x - a.x;
        dy = b.y - a.y;
        this->a = a;
        this->b = b;
        denominator = std::sqrt(dx * dx + dy * dy);
    }

    float distanceFromLine(const Point& p) const {
        return std::abs(dy * p.x - dx * p.y + (b.x * a.y - b.y * a.x)) / denominator;
    }
};

bool insideTriangle(Point &s, Point &a, Point &b, Point &c) {
  int as_x = s.x - a.x;
  int as_y = s.y - a.y;

  bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x > 0;

  if ((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0 == s_ab) 
      return false;
  if ((c.x - b.x) * (s.y - b.y) - (c.y - b.y)*(s.x - b.x) > 0 != s_ab) 
      return false;
  return true;
}

void savePointsToFile(vector<Point> points, string filename) {
  ofstream outputFile(filename);

  if (!outputFile.is_open())
  {
    cerr << "Error opening file: " << filename
         << std::endl;

    // Check for specific error conditions
    if (outputFile.bad())
    {
      cerr << "Fatal error: badbit is set." << endl;
    }

    if (outputFile.fail())
    {
      // Print a more detailed error message using strerror
      cerr << "Error details: " << strerror(errno)
           << endl;
    }

    // Handle the error or exit the program
    return;
  }

  for (int i = 0; i < points.size(); i++)
  {
    outputFile << points[i].x << " " << points[i].y << endl;
  }
}


int main(int argc, char *argv[]) { 
  clock_t glob_start_time, glob_end_time;
  double glob_elapsed_time;

  glob_start_time = clock();

  #pragma region Initialization
  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Get the number of processes
  int numP;
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  // Get processor name
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen;
  MPI_Get_processor_name(processor_name, &namelen);
  // Get the ID of the process
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Datatype PointType = registerPointType();
  #pragma endregion

  #pragma region 1. Input
  //================= 1. Read points from file =================
  #ifdef TIMING
  clock_t start_time = clock();
  #endif

  MPI_File fh;
  MPI_File_open(MPI_COMM_WORLD, "points.bin", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

  // points file contains the points as binary data
  // each point is represented by two integers
  // each integer is 4 bytes long and little-endian
  // the file starts with an integer that represents the number of points
  int numPoints;
  MPI_File_read(fh, &numPoints, 1, MPI_INT, MPI_STATUS_IGNORE);

  #ifdef DEBUG
  cout << "<process " << rank << "> Number of points total: " << numPoints << endl;
  #endif

  int pointsPerProcess = ceil(numPoints / numP);
  int start = rank * pointsPerProcess;
  int end = min(start + pointsPerProcess, numPoints);
  if (rank == numP - 1) { end = numPoints; }
  pointsPerProcess = end - start;

  Point *points = new Point[pointsPerProcess];
  MPI_File_read_at(fh, sizeof(int) + start * 2 * sizeof(int), points, pointsPerProcess, PointType, MPI_STATUS_IGNORE);

  MPI_File_close(&fh);

  #ifdef DEBUG
  cout << "<process " << rank << "> Points read: " << pointsPerProcess << endl;
  #endif

  #ifdef TIMING
  clock_t end_time = clock();
  double elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << "> Time to read points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Read time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }

  glob_start_time = clock();

  #pragma region 2. extremes
  //================= 2. Find the 2 extreme points =================
  #ifdef TIMING
  start_time = clock();
  #endif

  // find the local extreme points
  Point left = points[0], right = points[0];

  for (int i = 1; i < pointsPerProcess; i++) {
    if (points[i].x < left.x) {
      left = points[i];
    } else if (points[i].x > right.x) {
      right = points[i];
    }
  }

  #ifdef DEBUG
  cout << "<process " << rank << "> Local left: " << left.toString() << ", right: " << right.toString() << endl;
  #endif

  // find the global extreme points
  Point *left_candidates = new Point[numP];
  Point *right_candidates = new Point[numP];

  MPI_Allgather(&left, 1, PointType, left_candidates, 1, PointType, MPI_COMM_WORLD);
  MPI_Allgather(&right, 1, PointType, right_candidates, 1, PointType, MPI_COMM_WORLD);

  for (int i = 0; i < numP; i++) {
    if (left_candidates[i].x < left.x) {
      left = left_candidates[i];
    }

    if (right_candidates[i].x > right.x) {
      right = right_candidates[i];
    }
  }

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << "> Time to find extreme points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  #ifdef DEBUG
  cout << "<process " << rank << "> Final left: " << left.toString() << ", right: " << right.toString() << endl;
  #endif
  
  //================================================================
  #pragma endregion

  #pragma region 3. Split points
  //================= 3. Split points into two halves =================
  #ifdef TIMING
  start_time = clock();
  #endif

  vector<Point> upperHalf, lowerHalf = vector<Point>();
  upperHalf.reserve(pointsPerProcess / 2);
  lowerHalf.reserve(pointsPerProcess / 2);

  float dx = right.x - left.x;
  float dy = right.y - left.y;

  for (int i = 0; i < pointsPerProcess; i++) {
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

  #ifdef DEBUG
  cout << "<process " << rank << "> Upper half: " << upperHalf.size() << ", lower half: " << lowerHalf.size() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << "> Time to split points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  vector<Point> hull;

  hull.push_back(right);
  hull.push_back(left);

  QuickHull(PointType, numP, rank, upperHalf, left, right, hull, 0);
  QuickHull(PointType, numP, rank, lowerHalf, right, left, hull, 0);

  if (rank == 0) {
    cout << "Hull size: " << hull.size() << endl;

    if (hull.size() < 1000) {
      savePointsToFile(hull, "hull.txt");
    }
  }

  // Terminate MPI
  MPI_Finalize();

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Elapsed time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }

  return 0;
}

void QuickHull(
  MPI_Datatype PointType, int numP, int rank,
  vector<Point> &points,
  Point a, Point b,
  vector<Point> &hull,
  int iteration
) {
  #pragma region 0. Termination
  //================= 0. If there are no points left, return =================
  #ifdef TIMING
  clock_t start_time = clock();
  #endif

  int pointsLeft = points.size();
  int *pointsLeftPerProcess = new int[numP];

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, local points left: " << pointsLeft << endl;
  #endif

  MPI_Allgather(&pointsLeft, 1, MPI_INT, pointsLeftPerProcess, 1, MPI_INT, MPI_COMM_WORLD);

  pointsLeft = 0;
  for (int i = 0; i < numP; i++) {
    pointsLeft += pointsLeftPerProcess[i];
  }

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, global points left: " << pointsLeft << endl;
  #endif

  #ifdef TIMING
  clock_t end_time = clock();
  double elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to check points left: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  if (pointsLeft == 0) {
    return;
  }
  //================================================================
  #pragma endregion

  #pragma region 1. Furthest point
  //================= 1. Find the point with the maximum distance =================
  #ifdef TIMING
  start_time = clock();
  #endif
  
  float maxDistance = -1;
  Point maxPoint = Point();

  LineDistanceCalculator lineDistanceCalculator(a, b);

  for (int i = 0; i < points.size(); i++) {
    float distance = lineDistanceCalculator.distanceFromLine(points[i]);

    if (distance > maxDistance) {
      maxDistance = distance;
      maxPoint = points[i];
    }
  }

  bool valid = maxDistance > 0;

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, local max point: " << maxPoint.toString() << endl;
  #endif

  Point *maxPoints = new Point[numP];
  bool *validPoints = new bool[numP];

  MPI_Allgather(&valid, 1, MPI_C_BOOL, validPoints, 1, MPI_C_BOOL, MPI_COMM_WORLD);
  MPI_Allgather(&maxPoint, 1, PointType, maxPoints, 1, PointType, MPI_COMM_WORLD);

  maxDistance = -1;
  maxPoint = Point();

  for (int i = 0; i < numP; i++) {
    if (!validPoints[i]) { continue; }

    float tmpDistance = lineDistanceCalculator.distanceFromLine(maxPoints[i]);

    if (tmpDistance > maxDistance) {
      maxDistance = tmpDistance;
      maxPoint = maxPoints[i];
    }
  }

  // add the max point to the hull
  hull.push_back(maxPoint);

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, global max point: " << maxPoint.toString() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to find max point: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  #pragma region 3. Triangle
  //================= 3. Remove points inside the triangle =================
  #ifdef TIMING
  start_time = clock();
  #endif

  vector<Point> newPoints;
  int removedPoints = 0;

  for (int i = 0; i < points.size(); i++) {
    // if the point is the same as the max point, skip it
    if (points[i].x == maxPoint.x && points[i].y == maxPoint.y) {
      continue;
    }

    if (!insideTriangle(points[i], a, b, maxPoint)) {
      newPoints.push_back(points[i]);
    } else {
      removedPoints++;
    }
  }

  points = newPoints;

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, removed points: " << removedPoints << ", points left: " << newPoints.size() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to remove points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  //================================================================
  #pragma endregion

  #pragma region 4. Split points
  //================= 4. Split points into two halves =================
  #ifdef TIMING
  start_time = clock();
  #endif

  vector<Point> upperHalf, lowerHalf;

  for (int i = 0; i < points.size(); i++) {
    float line = (maxPoint.y - a.y) * (points[i].x - a.x) - (maxPoint.x - a.x) * (points[i].y - a.y);

    // if its the left or right point, skip it
    if (points[i].x == a.x && points[i].y == a.y) {
      continue;
    }

    if (points[i].x == maxPoint.x && points[i].y == maxPoint.y) {
      continue;
    }

    if (line > 0) {
      upperHalf.push_back(points[i]);
    } else if (line < 0) {
      lowerHalf.push_back(points[i]);
    }
  }

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, upper half: " << upperHalf.size() << ", lower half: " << lowerHalf.size() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to split points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  #pragma region 4. Recursion
  //================= 4. Recurse on the two halves =================
  QuickHull(PointType, numP, rank, upperHalf, a, maxPoint, hull, iteration + 1);
  QuickHull(PointType, numP, rank, lowerHalf, maxPoint, b, hull, iteration + 1);
  //================================================================
  #pragma endregion
}