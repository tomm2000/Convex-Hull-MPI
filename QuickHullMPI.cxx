/*
make && mpirun -n 4 ./QuickHullMPI.out
*/
#include "lib.hxx"
#include "QuickHull.hxx"

using namespace std;

// #define TIMING
// #define DEBUG

// #define LOAD_FROM_FILE
#define RADIUS 1000000

int main(int argc, char *argv[]) { 
  clock_t glob_start_time, glob_end_time, start_time, end_time;
  double glob_elapsed_time, elapsed;

  glob_start_time = clock();

  #pragma region Initialization
  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Get the number of processes
  int numP;
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  // Get the ID of the process
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Datatype PointType = registerPointType();
  #pragma endregion

  #ifdef LOAD_FROM_FILE
  #pragma region 1. Input
  //================= 1. Read points from file =================
  #ifdef TIMING
  start_time = clock();
  #endif

  MPI_File fh;
  MPI_File_open(MPI_COMM_WORLD, "points.bin", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

  MPI_Status status;
  size_t numPoints;
  MPI_File_read_at(fh, sizeof(int) * 2, &numPoints, 1, MPI_UNSIGNED_LONG_LONG, &status);

  #ifdef DEBUG
  cout << "<process " << rank << "> Number of points total: " << numPoints << endl;
  #endif

  size_t pointsPerProcess = ceil(numPoints / numP);
  size_t start = rank * pointsPerProcess;
  size_t end = start + pointsPerProcess;
  if (end > numPoints) { end = numPoints; }
  if (rank == numP - 1) { end = numPoints; }
  pointsPerProcess = end - start;

  #ifdef DEBUG
  cout << "<process " << rank << "> Points per process: " << pointsPerProcess << endl;
  #endif

  Point *points = new Point[pointsPerProcess];
  MPI_File_read_at(fh, sizeof(int) * 2 + sizeof(size_t) + start * sizeof(Point), points, pointsPerProcess, PointType, &status);
  MPI_File_close(&fh);

  #ifdef DEBUG
  cout << "<process " << rank << "> Points read: " << pointsPerProcess << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << "> Time to read points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion
  #else
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
  int seed = 0;

  // if there is a second argument, use it as seed
  if (argc > 2) { seed = atoi(argv[2]); }
  
  size_t pointsPerProcess = ceil(NPOINTS / numP);
  size_t start = rank * pointsPerProcess;
  size_t end = start + pointsPerProcess;
  if (end > NPOINTS) { end = NPOINTS; }
  if (rank == numP - 1) { end = NPOINTS; }
  pointsPerProcess = end - start;

  Point *points = new Point[pointsPerProcess];

  srand(seed);

  // generate points in a circle
  for (size_t i = 0; i < pointsPerProcess; i++) {
    // generate points uniformly distributed in a circle
    double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;
    double r = sqrt((double) rand() / RAND_MAX) * RADIUS;

    Point p;
    p.x = r * cos(angle);
    p.y = r * sin(angle);

    points[i] = p;
  }

  // create a points vector
  vector<Point> tempPoints = vector<Point>();
  tempPoints.reserve(pointsPerProcess);
  for (int i = 0; i < pointsPerProcess; i++) {
    tempPoints.push_back(points[i]);
  }

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

  if (elapsed * 1000 > 1) {
    cout << "Time to generate points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  #pragma endregion
  #endif
  
  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Init time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }

  #pragma region 4. Local hull
  //================= 4. Recurse on the two halves =================
  glob_start_time = clock();

  #ifdef TIMING
  start_time = clock();
  #endif

  vector<Point> hull;

  QuickHullInit(points, pointsPerProcess, hull);

  // FIXME: is this free necessary?
  free(points);

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

  if (elapsed * 1000 > 1) {
    cout << "Time to local hull: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  #ifdef DEBUG
  cout << "Local hull size: " << hull.size() << endl;
  #endif
  //================================================================
  #pragma endregion

  #pragma region 5. Merge hulls
  //================= 5. Merge the hulls from each process =================
  if (rank == 0) {
    // #ifdef TIMING
    start_time = clock();
    // #endif

    for (int i = 1; i < numP; i++) {
      // cout << "Receiving points from process " << i << endl;

      size_t pointsLeftProcess;
      MPI_Recv(&pointsLeftProcess, 1, MPI_UNSIGNED_LONG_LONG, i, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      Point *points = new Point[pointsLeftProcess];
      MPI_Recv(points, pointsLeftProcess, PointType, i, empty_points, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (size_t j = 0; j < pointsLeftProcess; j++) {
        hull.push_back(points[j]);
      }
    }
    cout << "Merged hull size: " << hull.size() << endl;

    // #ifdef TIMING
    end_time = clock();
    elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    if (elapsed * 1000 > 1) {
      cout << "Time to merge hulls: " << elapsed * 1000.0 << "ms" << endl;
    }
    // #endif

    // #ifdef TIMING
    start_time = clock();
    // #endif

    vector<Point> finalHull;
    QuickHullInit(hull.data(), hull.size(), finalHull);

    if (finalHull.size() < 100000) {
      savePointsToFile(finalHull, "hull.txt");
    }

    // #ifdef TIMING
    end_time = clock();
    elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    if (elapsed * 1000 > 1) {
      cout << "Time to final hull: " << elapsed * 1000.0 << "ms" << endl;
    }
    // #endif

    // #ifdef DEBUG
    cout << "Hull size: " << finalHull.size() << endl;
    // #endif
  } else {
    // send number of points
    size_t NpointsLeft = hull.size();
    MPI_Send(&NpointsLeft, 1, MPI_UNSIGNED_LONG_LONG, 0, msg_tag, MPI_COMM_WORLD);

    #ifdef DEBUG
    cout << "Sending " << NpointsLeft << " points to process 0" << endl;
    #endif

    // send points
    Point *pointsArray = new Point[hull.size()];
    std::copy(hull.begin(), hull.end(), pointsArray);

    MPI_Send(pointsArray, hull.size(), PointType, 0, empty_points, MPI_COMM_WORLD);
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