#include "convex_hull.hxx"

using namespace std;

void convex_hull(
    Point *points,
    size_t numPoints,
    std::vector<Point> &hull,
    ConvexHullAlgorithm algorithm,
    Timer *timer) {
  if (timer == nullptr) {
    timer = new Timer();
  }

  switch (algorithm) {
    case ConvexHullAlgorithm::GRAHAM_SCAN:
      graham_scan(points, numPoints, hull);
      break;
  }
}

void convex_hull_parallel(
    Point *points,
    size_t numPoints,
    std::vector<Point> &hull,
    ConvexHullAlgorithm algorithm,
    Timer *timer) {
  if (timer == nullptr) {
    timer = new Timer();
  }

  vector<Point> mergedHull = vector<Point>();

  // run the algorithm in parallel using OpenMP
  #pragma omp parallel num_threads(4)
  {
    vector<Point> localHull = vector<Point>();

    #pragma omp single
    {
      timer->start("calculation");
    }

    int numThreads = omp_get_num_threads();
    int threadId = omp_get_thread_num();

    size_t start = threadId * numPoints / numThreads;
    size_t end = (threadId + 1) * numPoints / numThreads;

    if (threadId == numThreads - 1) { end = numPoints; }

    printf("Thread %d has points %s and %s\n", threadId, points[start].toString().c_str(), points[end - 1].toString().c_str());

    convex_hull(points + start, end - start, localHull, algorithm, timer);

    #pragma omp single
    {
      timer->stop("calculation");
    }

    #pragma omp critical
    {
      mergedHull.insert(mergedHull.end(), localHull.begin(), localHull.end());
    }
  }

  timer->printTimer("calculation");

  hull = vector<Point>();
}

// void convex_hull_distributed(
//     MPI_Datatype PointType,
//     MPI_Comm comm,
//     Point points[],  // points is only used by the master process
//     size_t numPoints,
//     std::vector<Point> &hull,
//     ConvexHullAlgorithm algorithm,
//     Timer *timer,
//     bool hybrid) {
//   if (timer == nullptr) {
//     timer = new Timer();
//   }

//   int numP, rank;
//   MPI_Comm_size(MPI_COMM_WORLD, &numP);
//   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//   Point *localPoints = nullptr;
//   size_t localNumPoints = 0;

//   // 1. send the respective points to each process
//   if (rank == 0) {
//     for (int i = 0; i < numP; i++) {
//       size_t start = i * numPoints / numP;
//       size_t end = (i + 1) * numPoints / numP;
//       if (i == numP - 1) {
//         end = numPoints;
//       }
//       size_t count = end - start;

//       if (i == 0) {
//         localNumPoints = count;
//         localPoints = new Point[localNumPoints];
//         localPoints = points + start;
//       } else {
//         timer->start("communication");
//         MPI_Send(&count, 1, MPI_UNSIGNED_LONG_LONG, i, 0, comm);
//         MPI_Send(points + start, count, PointType, i, 0, comm);
//         timer->stop("communication");
//       }
//     }
//   } else {
//     timer->start("communication");
//     MPI_Recv(&localNumPoints, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, comm, MPI_STATUS_IGNORE);

//     localPoints = new Point[localNumPoints];
//     MPI_Recv(localPoints, localNumPoints, PointType, 0, 0, comm, MPI_STATUS_IGNORE);
//     timer->stop("communication");
//   }

//   convex_hull_predistributed(PointType, comm, localPoints, localNumPoints, hull, algorithm, timer, hybrid);
// }

// void convex_hull_predistributed(
//     MPI_Datatype PointType,
//     MPI_Comm comm,
//     Point points[],
//     size_t numPoints,
//     std::vector<Point> &hull,
//     ConvexHullAlgorithm algorithm,
//     Timer *timer,
//     bool hybrid) {
//   if (timer == nullptr) {
//     printf("Timer is null\n");
//     timer = new Timer();
//   }

//   int numP, rank;
//   MPI_Comm_size(MPI_COMM_WORLD, &numP);
//   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//   vector<Point> localHull;

//   timer->start("calculation");
//   if (hybrid) {
//     convex_hull_parallel(points, numPoints, localHull, algorithm, timer);
//   } else {
//     convex_hull(points, numPoints, localHull, algorithm, timer);
//   }
//   size_t localHullSize = localHull.size();
//   timer->stop("calculation");

//   // 3. gather the local hulls to the master process
//   if (rank == 0) {
//     vector<Point> mergedHull;
//     mergedHull.insert(mergedHull.end(), localHull.begin(), localHull.end());

//     for (int i = 1; i < numP; i++) {
//       timer->start("communication");
//       size_t count;
//       MPI_Recv(&count, 1, MPI_UNSIGNED_LONG_LONG, i, 0, comm, MPI_STATUS_IGNORE);

//       Point *remoteHull = new Point[count];
//       MPI_Recv(remoteHull, count, PointType, i, 0, comm, MPI_STATUS_IGNORE);

//       mergedHull.insert(mergedHull.end(), remoteHull, remoteHull + count);
//       timer->stop("communication");
//     }

//     timer->start("calculation");
//     convex_hull(mergedHull.data(), mergedHull.size(), hull, algorithm, timer);
//     timer->stop("calculation");
//   } else {
//     timer->start("communication");
//     MPI_Send(&localHullSize, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, comm);
//     MPI_Send(localHull.data(), localHullSize, PointType, 0, 0, comm);
//     timer->stop("communication");
//   }

//   MPI_Barrier(comm);
// }