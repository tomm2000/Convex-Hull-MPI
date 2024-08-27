#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include <vector>
#include <math.h>
#include <omp.h>
#include "mpi.h"
#include "point.hxx"
#include "graham_scan.hxx"
#include "hpc_helpers.hpp" 
#include "utils.hxx"

// enum for convex hull algorithm
enum class ConvexHullAlgorithm {
  GRAHAM_SCAN,
};

void convex_hull(
  Point points[],
  size_t numPoints,
  std::vector<Point> &hull,
  ConvexHullAlgorithm algorithm = ConvexHullAlgorithm::GRAHAM_SCAN,
  Timer *timer = nullptr
);

void convex_hull_parallel(
  Point points[],
  size_t numPoints,
  std::vector<Point> &hull,
  ConvexHullAlgorithm algorithm = ConvexHullAlgorithm::GRAHAM_SCAN,
  Timer *timer = nullptr
);

void convex_hull_distributed(
  MPI_Datatype PointType,
  MPI_Comm comm,
  Point points[], // points is only used by the master process
  size_t numPoints,
  std::vector<Point> &hull,
  ConvexHullAlgorithm algorithm = ConvexHullAlgorithm::GRAHAM_SCAN,
  Timer *timer = nullptr,
  bool hybrid = false
);

void convex_hull_predistributed(
  MPI_Datatype PointType,
  MPI_Comm comm,
  Point points[],
  size_t numPoints,
  std::vector<Point> &hull,
  ConvexHullAlgorithm algorithm = ConvexHullAlgorithm::GRAHAM_SCAN,
  Timer *timer = nullptr,
  bool hybrid = false
);
#endif