// Example usage
#include <iostream>
#include <vector>
#include "src/convex_hull.hxx"
#include "src/point.hxx"
#include "src/graham_scan.hxx"
#include "src/point.hxx"
#include "src/utils.hxx"
#include "src/point_generator.hxx"

#define PRE_DISTRIBUTED
// #define CENTRALIZED
// #define FILE_IO

int main(int argc, char *argv[]) {
  #pragma region Initialization
  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Get the number of processes
  int numP, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Datatype PointType = registerPointType();
  #pragma endregion

  Timer timer = Timer();
  size_t numPointsTotal = stol(readArg(argc, argv, "npoints", "1000000"));
  uint seed = stoi(readArg(argc, argv, "seed", "0"));
  bool useHybrid = readArg(argc, argv, "hybrid", "false") == "true";
  
  if (seed == 0) { seed = time(NULL); }

  if (rank == 0) {
    printf("MPI tasks: %d | Use hybrid: %s | Seed: %d | Total points: %lu \n", numP, useHybrid ? "true" : "false", seed, numPointsTotal);
  }

  Point *points;

  #ifdef PRE_DISTRIBUTED
  size_t numPointsPerProcess = numPointsTotal / numP;
  if (rank == numP - 1) {
    numPointsPerProcess += numPointsTotal % numP;
  }

  if (rank == 0) {
    timer.start("points");
  }

  seed += rank * 2124;

  points = new Point[numPointsPerProcess];
  generate_points(numPointsPerProcess, points, PointGeneratorType::CIRCLE, seed);

  if (rank == 0) {
    timer.stop("points");
  }


  MPI_Barrier(MPI_COMM_WORLD);

  std::vector<Point> hull;

  if (rank == 0) {
    timer.start("final");
  }

  convex_hull_predistributed(
    PointType,
    MPI_COMM_WORLD,
    points,
    numPointsPerProcess,
    hull,
    ConvexHullAlgorithm::GRAHAM_SCAN,
    &timer,
    useHybrid
  );

  #endif

  #ifdef CENTRALIZED
  if (rank == 0) {
    timer.start("points");
    points = new Point[numPointsTotal];

    cout << "Generating " << numPointsTotal << " points on master process" << endl;
    generate_points(numPointsTotal, points, PointGeneratorType::CIRCLE, seed);
    timer.stop("points");
    timer.printTimer("points");
    fflush(stdout);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  std::vector<Point> hull;

  if (rank == 0) {
    timer.start("final");
  }

  convex_hull_distributed(
    PointType,
    MPI_COMM_WORLD,
    points,
    numPointsTotal,
    hull,
    ConvexHullAlgorithm::GRAHAM_SCAN,
    &timer,
    useHybrid
  );
  #endif

  #ifdef FILE_IO
  timer.start("points");

  MPI_File fh;
  MPI_File_open(MPI_COMM_WORLD, "results/points.bin", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

  // first (|int| bytes = 4) bytes is the number of bytes used by size_t
  int size_t_bytes;
  MPI_File_read_at(fh, 0, &size_t_bytes, 1, MPI_INT, MPI_STATUS_IGNORE);

  printf("size_t_bytes: %d\n", size_t_bytes);
  fflush(stdout);

  // second (|int| bytes = 4) is the size of each point
  int point_size;
  MPI_File_read_at(fh, sizeof(int), &point_size, 1, MPI_INT, MPI_STATUS_IGNORE);

  printf("point_size: %d\n", point_size);
  fflush(stdout);

  // write the number of points (|size_t| bytes)
  size_t numPoints;
  MPI_File_read_at(fh, sizeof(int) * 2, &numPoints, 1, MPI_UNSIGNED_LONG_LONG, MPI_STATUS_IGNORE);

  printf("numPoints: %lu\n", numPoints);
  fflush(stdout);

  size_t pointsPerProcess = ceil(numPoints / numP);
  size_t start = rank * pointsPerProcess;
  size_t end = start + pointsPerProcess;
  if (end > numPoints) { end = numPoints; }
  if (rank == numP - 1) { end = numPoints; }

  printf("rank: %d | start: %lu | end: %lu\n", rank, start, end);
  fflush(stdout);

  points = new Point[pointsPerProcess];
  MPI_File_read_at(
    fh,
    sizeof(int) * 2 + size_t_bytes + start * point_size,
    points,
    pointsPerProcess,
    PointType,
    MPI_STATUS_IGNORE
  );

  printf(points[0].toString().c_str());
  fflush(stdout);

  MPI_File_close(&fh);
  timer.stop("points");

  std::vector<Point> hull;

  if (rank == 0) {
    timer.start("final");
  }

  convex_hull_predistributed(
    PointType,
    MPI_COMM_WORLD,
    points,
    pointsPerProcess,
    hull,
    ConvexHullAlgorithm::GRAHAM_SCAN,
    &timer,
    useHybrid
  );
  #endif

  if (rank == 0) {
    timer.stop("final");
    printf("--------\n");
    timer.printTimer("calculation");
    timer.printTimer("communication");
    timer.printTimer("final");
    timer.printTimer("points");
    printf("size: %lu\n", hull.size());
  }

  // save_points_binary(points, numPointsPerProcess, "results/points.bin");
  // save_points_ascii(hull, "results/hull.txt");
  MPI_Finalize();

  return 0;
}