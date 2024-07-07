#ifndef QUICKHULLDISTRIBUTED_HXX
#define QUICKHULLDISTRIBUTED_HXX

#include "lib.hxx" // Include the previously created header

// Declaration of the QuickHull function
std::vector<Point> QuickHullDistributed(
  MPI_Datatype PointType, int numP, int rank,
  std::vector<Point> &points,
  Point a, Point b,
  std::vector<Point> &hull,
  int iteration
);

#endif // QUICKHULLDISTRIBUTED_HXX
