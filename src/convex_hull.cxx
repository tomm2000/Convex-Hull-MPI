#include "convex_hull.hxx"

using namespace std;


void convex_hull(Point points[], size_t numPoints, std::vector<Point> &hull, ConvexHullAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexHullAlgorithm::GRAHAM_SCAN:
      graham_scan(points, numPoints, hull);
      break;
  }
}

void convex_hull_parallel(Point points[], size_t numPoints, std::vector<Point> &hull, ConvexHullAlgorithm algorithm) {
  int threadCount = 1;
  int pointsPerThread = 0;
  vector<Point> mergedHull;

#pragma omp parallel
  {
#pragma omp single
    {
#if defined(_OPENMP)
      threadCount = omp_get_num_threads();
#endif

      pointsPerThread = round(numPoints / threadCount);

      printf("Thread count=%d, pointsPerThread=%d\n", threadCount, pointsPerThread);
    }

    int threadId = 0;
#if defined(_OPENMP)
    threadId = omp_get_thread_num();
#endif

    size_t start = threadId * pointsPerThread;
    size_t end = min(start + pointsPerThread, numPoints);

    if (threadId == threadCount - 1) { end = numPoints; }

    vector<Point> localHull;

    TIMERSTART(parallel_convex_hull);
    convex_hull(points + start, end - start, localHull, algorithm);
    TIMERSTOP(parallel_convex_hull);

#pragma omp critical
    {
      mergedHull.insert(mergedHull.end(), localHull.begin(), localHull.end());
    }
  }

  printf("Merged hull size=%lu\n", mergedHull.size());

  convex_hull(mergedHull.data(), mergedHull.size(), hull, ConvexHullAlgorithm::GRAHAM_SCAN);
}

void convex_hull_distributed(
  MPI_Datatype PointType,
  MPI_Comm comm,
  Point points[], // points is only used by the master process
  size_t numPoints,
  std::vector<Point> &hull,
  ConvexHullAlgorithm algorithm
) {
  int numP, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  Point *localPoints = nullptr;
  size_t localNumPoints = 0;

  // 1. send the respective points to each process
  if (rank == 0) {
    for (int i = 0; i < numP; i++) {
      size_t start = i * numPoints / numP;
      size_t end = (i + 1) * numPoints / numP;
      if (i == numP - 1) { end = numPoints; }
      size_t count = end - start;

      if (i == 0) {
        localNumPoints = count;
        localPoints = new Point[localNumPoints];
        localPoints = points + start;
      } else {
        MPI_Send(&count, 1, MPI_UNSIGNED_LONG_LONG, i, 0, comm);
        MPI_Send(points + start, count, PointType, i, 0, comm);
      }
    }
  } else {
    MPI_Recv(&localNumPoints, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, comm, MPI_STATUS_IGNORE);

    localPoints = new Point[localNumPoints];
    MPI_Recv(localPoints, localNumPoints, PointType, 0, 0, comm, MPI_STATUS_IGNORE);
  }

  // 2. compute the convex hull of the local points
  vector<Point> localHull;
  convex_hull(localPoints, localNumPoints, localHull, algorithm);
  size_t localHullSize = localHull.size();

  printf("Rank=%d, localHullSize=%lu\n", rank, localHullSize);

  // 3. gather the local hulls to the master process
  if (rank == 0) {
    vector<Point> mergedHull;
    mergedHull.insert(mergedHull.end(), localHull.begin(), localHull.end());

    for (int i = 1; i < numP; i++) {
      size_t count;
      MPI_Recv(&count, 1, MPI_UNSIGNED_LONG_LONG, i, 0, comm, MPI_STATUS_IGNORE);

      Point *remoteHull = new Point[count];
      MPI_Recv(remoteHull, count, PointType, i, 0, comm, MPI_STATUS_IGNORE);

      mergedHull.insert(mergedHull.end(), remoteHull, remoteHull + count);
    }

    convex_hull(mergedHull.data(), mergedHull.size(), hull, algorithm);
  } else {
    MPI_Send(&localHullSize, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, comm);
    MPI_Send(localHull.data(), localHullSize, PointType, 0, 0, comm);
  }

  MPI_Barrier(comm);
}