// Example usage
#include <iostream>
#include <vector>
#include "src/convex_hull.hxx"
#include "src/point.hxx"
#include "src/graham_scan.hxx"
#include "src/point.hxx"
#include "src/point_generator.hxx"

int main(int argc, char *argv[]) {
  #pragma region Initialization
  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Get the number of processes
  int numP, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    printf("Number of processes: %d\n", numP);
  }

  MPI_Datatype PointType = registerPointType();
  #pragma endregion

  const size_t NPOINTS = 50000000;

  int memUsage = estimateMemoryUsage(NPOINTS);
  if (rank == 0) {
    printf("Estimated memory usage: %d MB\n", memUsage);
  }

  vector<Point> points = generate_points(NPOINTS, PointGeneratorType::CIRCLE, 1);

  std::vector<Point> hull;

  time_t start = clock();

  TIMERSTART(convex_hull);
  
  convex_hull(points.data(), points.size(), hull, ConvexHullAlgorithm::GRAHAM_SCAN);
  // convex_hull_parallel(points.data(), points.size(), hull, ConvexHullAlgorithm::GRAHAM_SCAN);
  // convex_hull_distributed(PointType, MPI_COMM_WORLD, points.data(), points.size(), hull, ConvexHullAlgorithm::GRAHAM_SCAN);
  if (rank == 0) {
    TIMERSTOP(convex_hull);
    printf("Size of hull: %lu\n", hull.size());
  }

  // save_points_binary(points, "results/points.bin");
  // save_points_ascii(hull, "results/hull.txt");
  MPI_Finalize();

  return 0;
}