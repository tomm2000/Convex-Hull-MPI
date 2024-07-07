/*
> mpicxx -o QuickHullMPI.out QuickHullMPI.cxx && mpirun -n 4 ./QuickHullMPI.out
*/
#include "lib.hxx"
#include "QuickHullDistributed.hxx"
#include "QuickHullSequential.hxx"

using namespace std;

// #define TIMING
// #define DEBUG

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

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Init time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }
  //================================================================
  #pragma endregion

  #pragma region 2. extremes
  //================= 2. Find the 2 extreme points =================
  glob_start_time = clock();

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

  Point *leftPoints = new Point[numP];
  Point *rightPoints = new Point[numP];

  MPI_Allgather(&left, 1, PointType, leftPoints, 1, PointType, MPI_COMM_WORLD);
  MPI_Allgather(&right, 1, PointType, rightPoints, 1, PointType, MPI_COMM_WORLD);

  left = leftPoints[0];
  right = rightPoints[0];

  for (int i = 1; i < numP; i++) {
    if (leftPoints[i].x < left.x) {
      left = leftPoints[i];
    } else if (leftPoints[i].x == left.x && leftPoints[i].y < left.y) {
      left = leftPoints[i];
    }

    if (rightPoints[i].x > right.x) {
      right = rightPoints[i];
    } else if (rightPoints[i].x == right.x && rightPoints[i].y > right.y) {
      right = rightPoints[i];
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

  // FIXME: is this free necessary?
  free(points);

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

  #pragma region 4. Recursion
  //================= 4. Recurse on the two halves =================
  vector<Point> hull;

  hull.push_back(right);
  hull.push_back(left);

  vector<Point> pointsLeft  = QuickHullDistributed(PointType, numP, rank, upperHalf, left, right, hull, 0);
  vector<Point> _pointsLeft = QuickHullDistributed(PointType, numP, rank, lowerHalf, right, left, hull, 0);

    // Comleted distributed quickhull
    // cout << "Completed distributed quickhull" << endl;
    cout << "<process " << rank << "> Points left: " << pointsLeft.size() << endl;

    // Running sequential quickhull with {} points
    cout << "<process " << rank << "> Running sequential quickhull with " << pointsLeft.size() << " points" << endl;

  MPI_Barrier(MPI_COMM_WORLD);

  pointsLeft.insert(pointsLeft.end(), _pointsLeft.begin(), _pointsLeft.end());
  int NpointsLeft = pointsLeft.size();

  // gather points left
  if (rank == 0) {
    for (int i = 1; i < numP; i++) {
      cout << "Receiving points from process " << i << endl;

      int pointsLeftProcess;
      MPI_Recv(&pointsLeftProcess, 1, MPI_INT, i, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      Point *points = new Point[pointsLeftProcess];
      MPI_Recv(points, pointsLeftProcess, PointType, i, empty_points, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (int j = 0; j < pointsLeftProcess; j++) {
        pointsLeft.push_back(points[j]);
      }
    }

    // print points left
    cout << "Points left: " << pointsLeft.size() << endl;

    QuickHull_sequential(pointsLeft, left, right, hull, 0);
  } else {
    // send number of points
    MPI_Send(&NpointsLeft, 1, MPI_INT, 0, msg_tag, MPI_COMM_WORLD);

    // send points
    Point *pointsArray = new Point[NpointsLeft];
    std::copy(pointsLeft.begin(), pointsLeft.end(), pointsArray);

    MPI_Send(pointsArray, NpointsLeft, PointType, 0, empty_points, MPI_COMM_WORLD);
  }


  if (rank == 0) {
    cout << "Hull size: " << hull.size() << endl;

    if (hull.size() < 1000) {
      savePointsToFile(hull, "hull.txt");
    }
  }
  //================================================================
  #pragma endregion

  // Terminate MPI
  MPI_Finalize();

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Elapsed time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }

  return 0;
}
