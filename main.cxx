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

int main(int argc, char *argv[]) {
  #pragma region Initialization
  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Get the number of processes
  int numP, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    printf("Number of MPI tasks: %d\n", numP);
  }

  MPI_Datatype PointType = registerPointType();
  #pragma endregion

  Timer timer = Timer();
  size_t numPointsTotal = stol(readArg(argc, argv, "npoints", "1000000"));
  int seed = stoi(readArg(argc, argv, "seed", "0"));
  bool useHybrid = readArg(argc, argv, "hybrid", "false") == "true";
  
  if (seed == 0) { seed = time(NULL); }
  int memUsage = estimateMemoryUsage(numPointsTotal);

  if (rank == 0) {
    cout << "Use hybrid: " << useHybrid << endl;
    cout << "Seed: " << seed << endl;
    cout << "Estimated memory usage: " << memUsage << " MB" << endl;
    cout << "Total number of points: " << numPointsTotal << endl;
  }

  Point *points;

  #ifdef PRE_DISTRIBUTED
  size_t numPointsPerProcess = numPointsTotal / numP;
  if (rank == numP - 1) {
    numPointsPerProcess += numPointsTotal % numP;
  }

  if (rank == 0) {
    cout << "Generating ~" << numPointsPerProcess << " points per process on " << numP << " processes" << endl;
    timer.start("points");
  }

  seed += rank * 2124;

  points = new Point[numPointsPerProcess];
  // generate_points(numPointsPerProcess, points, PointGeneratorType::CIRCLE, seed);
  generate_points_parallel(numPointsPerProcess, points, PointGeneratorType::CIRCLE, seed);

  if (rank == 0) {
    timer.stop("points");
    timer.printTimer("points");
    cout << "Generated " << numPointsPerProcess << " points on master process" << endl;
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
    ConvexHullAlgorithm::QUICK_HULL,
    &timer,
    useHybrid
  );
  #else
  // if (rank == 0) {
  //   points = new Point[numPointsTotal];

  //   cout << "Generating " << numPointsTotal << " points on master process" << endl;
  //   generate_points(numPointsTotal, points, PointGeneratorType::CIRCLE, seed);
  // }

  // MPI_Barrier(MPI_COMM_WORLD);

  // std::vector<Point> hull;

  // if (rank == 0) {
  //   timer.start("final");
  // }

  // // convex_hull_distributed(PointType, MPI_COMM_WORLD, points, numPointsTotal, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer, useHybrid);
  // convex_hull_parallel(points, numPointsTotal, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer);
  #endif

  if (rank == 0) {
    timer.stop("final");
    printf("========================================\n");
    timer.printTimer("calculation");
    timer.printTimer("communication");
    timer.printTimer("final");
    printf("size: %lu\n", hull.size());
  }

  save_points_binary(points, numPointsPerProcess, "results/points.bin");
  save_points_ascii(hull, "results/hull.txt");
  MPI_Finalize();

  return 0;
}