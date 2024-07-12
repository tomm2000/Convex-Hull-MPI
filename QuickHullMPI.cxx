/*
> mpicxx -o QuickHullMPI.out QuickHullMPI.cxx && mpirun -n 4 ./QuickHullMPI.out
*/
#include "lib.hxx"
#include "QuickHull.hxx"

using namespace std;

// #define TIMING
// #define DEBUG

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

  #pragma region 1. Input
  //================= 1. Read points from file =================
  #ifdef TIMING
  start_time = clock();
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
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
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

      int pointsLeftProcess;
      MPI_Recv(&pointsLeftProcess, 1, MPI_INT, i, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      Point *points = new Point[pointsLeftProcess];
      MPI_Recv(points, pointsLeftProcess, PointType, i, empty_points, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (int j = 0; j < pointsLeftProcess; j++) {
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
    int NpointsLeft = hull.size();
    MPI_Send(&NpointsLeft, 1, MPI_INT, 0, msg_tag, MPI_COMM_WORLD);

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
