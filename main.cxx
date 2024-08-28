// Example usage
#include <iostream>
#include <vector>
#include "src/convex_hull.hxx"
#include "src/point.hxx"
#include "src/graham_scan.hxx"
#include "src/point.hxx"
#include "src/utils.hxx"
#include "src/point_generator.hxx"

// 18.6855s algo 40M points
// 4.4306s algo 10M points
// 2.15396s algo 5M points
// 0.395528s algo 1M points

// #define PRE_DISTRIBUTED

int main(int argc, char *argv[]) {
  // #pragma region Initialization
  // // Initialize MPI
  // MPI_Init(&argc, &argv);

  // // Get the number of processes
  // int numP, rank;
  // MPI_Comm_size(MPI_COMM_WORLD, &numP);
  // MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // if (rank == 0) {
  //   printf("Number of processes: %d\n", numP);
  // }

  // MPI_Datatype PointType = registerPointType();
  // #pragma endregion

  Timer timer = Timer();
  size_t numPointsTotal = stol(readArg(argc, argv, "npoints", "10000000"));
  int seed = stoi(readArg(argc, argv, "seed", "0"));
  bool useHybrid = readArg(argc, argv, "hybrid", "false") == "true";
  
  if (seed == 0) { seed = time(NULL); }
  int memUsage = estimateMemoryUsage(numPointsTotal);

  // if (rank == 0) {
  //   cout << "Use hybrid: " << useHybrid << endl;
  //   cout << "Seed: " << seed << endl;
  //   cout << "Estimated memory usage: " << memUsage << " MB" << endl;
  //   cout << "Total number of points: " << numPointsTotal << endl;
  // }

  Point *points;

  #ifdef PRE_DISTRIBUTED
  size_t numPointsPerProcess = numPointsTotal / numP;
  if (rank == numP - 1) { numPointsPerProcess += numPointsTotal % numP; }

  if (rank == 0) {
    cout << "Generating ~" << numPointsPerProcess << " points per process" << endl;
  }

  seed += rank * numPointsPerProcess;

  points = new Point[numPointsPerProcess];
  generate_points(numPointsPerProcess, points, PointGeneratorType::CIRCLE, seed);

  MPI_Barrier(MPI_COMM_WORLD);

  std::vector<Point> hull;

  if (rank == 0) {
    timer.start("final");
  }

  convex_hull_parallel(points, numPointsPerProcess, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer);
  // convex_hull_predistributed(PointType, MPI_COMM_WORLD, points, numPointsPerProcess, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer, useHybrid);
  
  #else

  // if (rank == 0) {
    points = new Point[numPointsTotal];

    cout << "Generating " << numPointsTotal << " points on master process" << endl;
    generate_points(numPointsTotal, points, PointGeneratorType::CIRCLE, seed);
  // }

  // MPI_Barrier(MPI_COMM_WORLD);

  std::vector<Point> hull;

  // if (rank == 0) {
    timer.start("final");
  // }

  // convex_hull_distributed(PointType, MPI_COMM_WORLD, points, numPointsTotal, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer, useHybrid);
  convex_hull_parallel(points, numPointsTotal, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer);
  #endif

  // convex_hull(points, NPOINTS, hull, ConvexHullAlgorithm::GRAHAM_SCAN);
  // convex_hull_parallel(points, NPOINTS, hull, ConvexHullAlgorithm::GRAHAM_SCAN, &timer);
  

  // if (rank == 0) {
    timer.stop("final");
    printf("========================================\n");
    timer.printTimer("calculation");
    timer.printTimer("communication");
    timer.printTimer("final");
    // timer.printTimer("points");
    printf("size: %lu\n", hull.size());
  // }

  // save_points_binary(points, "results/points.bin");
  // save_points_ascii(hull, "results/hull.txt");
  // MPI_Finalize();

  return 0;
}