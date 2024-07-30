/**
 * ==============================================================================
 * implementazione distribuita del QuickHull con MPI
 * ==============================================================================
 */

#include "lib.hxx"
#include "QuickHull.hxx"

using namespace std;

#define TIMING
// #define DEBUG

#define LOAD_FROM_FILE
#define RADIUS 100000000

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
  size_t NPOINTS;
  MPI_File_read_at(fh, sizeof(int) * 2, &NPOINTS, 1, MPI_UNSIGNED_LONG_LONG, &status);

  #ifdef DEBUG
  cout << "<process " << rank << "> Number of points total: " << NPOINTS << endl;
  #endif

  size_t pointsPerProcess = ceil(NPOINTS / numP);
  size_t start = rank * pointsPerProcess;
  size_t end = start + pointsPerProcess;
  if (end > NPOINTS) { end = NPOINTS; }
  if (rank == numP - 1) { end = NPOINTS; }
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
  if (rank == 0) {
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
  int seed = clock();

  // if there is a second argument, use it as seed
  if (argc > 2) { seed = atoi(argv[2]); }
  
  size_t pointsPerProcess = ceil(NPOINTS / numP);
  size_t start = rank * pointsPerProcess;
  size_t end = start + pointsPerProcess;
  if (end > NPOINTS) { end = NPOINTS; }
  if (rank == numP - 1) { end = NPOINTS; }
  pointsPerProcess = end - start;

  Point *points = new Point[pointsPerProcess];

  int seed_seed = seed * 23415 + rank * 12345;
  srand(seed_seed);
  seed = rand();
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

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) {
    cout << "Time to generate points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  #pragma endregion
  #endif

  if (rank == 0) {
    cout << "Each process has ~" << pointsPerProcess << " points" << endl;
    size_t size_per_process = pointsPerProcess * sizeof(Point);
    cout << "Total size per process: " << (size_t) size_per_process / 1000000 << " MB" << endl;

    // size of the data:
    size_t size = NPOINTS * sizeof(Point);
    cout << "Total size of the data: " << (size_t) size / 1000000 << " MB" << endl;
  }
  
  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Init time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }

  #pragma region 2. Local hull
  //================= 2. Calculate the local hull =================
  glob_start_time = clock();

  #ifdef TIMING
  start_time = clock();
  #endif

  vector<Point> hull;

  QuickHullInit(points, pointsPerProcess, hull);

  free(points);

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) {
    cout << "Time to local hull: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  #ifdef DEBUG
  cout << "Local hull size: " << hull.size() << endl;
  #endif
  //================================================================
  #pragma endregion

  #pragma region 3. Merge hulls
  //================= 3. Merge hulls =================
  if (rank == 0) {
    #ifdef TIMING
    start_time = clock();
    #endif

    for (int i = 1; i < numP; i++) {
      #ifdef DEBUG
      cout << "<process " << rank << "> Receiving points from process " << i << endl;
      #endif

      size_t pointsLeftProcess;
      MPI_Recv(&pointsLeftProcess, 1, MPI_UNSIGNED_LONG_LONG, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      #ifdef DEBUG
      cout << "<process " << rank << "> Points left process " << i << ": " << pointsLeftProcess << endl;
      #endif

      Point *points = new Point[pointsLeftProcess];
      MPI_Recv(points, pointsLeftProcess, PointType, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      #ifdef DEBUG
      cout << "<process " << rank << "> Received points: " << pointsLeftProcess << " from process " << i << endl;
      #endif

      for (size_t j = 0; j < pointsLeftProcess; j++) {
        hull.push_back(points[j]);
      }
    }
    cout << "Merged hull size: " << hull.size() << endl;

    #ifdef TIMING
    end_time = clock();
    elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    if (elapsed * 1000 > 1) {
      cout << "Time to gather hulls: " << elapsed * 1000.0 << "ms" << endl;
    }
    #endif

    #ifdef TIMING
    start_time = clock();
    #endif

    vector<Point> finalHull;
    QuickHullInit(hull.data(), hull.size(), finalHull);

    if (finalHull.size() < 100000) {
      savePointsToFile(finalHull, "hull.txt");
    }

    #ifdef TIMING
    end_time = clock();
    elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    cout << "Time to final hull: " << elapsed * 1000.0 << "ms" << endl;
    #endif

    cout << "Hull size: " << finalHull.size() << endl;
  } else {
    // send number of points
    size_t NpointsLeft = hull.size();
    MPI_Send(&NpointsLeft, 1, MPI_UNSIGNED_LONG_LONG, 0, msg_tag, MPI_COMM_WORLD);

    #ifdef DEBUG
    cout << "<process " << rank << "> Sending points: " << NpointsLeft << endl;
    #endif

    // send points
    Point *pointsArray = new Point[hull.size()];
    std::copy(hull.begin(), hull.end(), pointsArray);

    MPI_Send(pointsArray, hull.size(), PointType, 0, msg_tag, MPI_COMM_WORLD);

    #ifdef DEBUG
    cout << "<process " << rank << "> Sent points: " << NpointsLeft << endl;
    #endif
  }

  // the child processes that finish early will wait for the parent to finish
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();

  glob_end_time = clock();
  glob_elapsed_time = ((double) (glob_end_time - glob_start_time)) / CLOCKS_PER_SEC;

  if (rank == 0) {
    cout << "Elapsed time: " << glob_elapsed_time * 1000.0 << "ms" << endl;
  }

  return 0;
}