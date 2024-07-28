#ifndef QUICKHULL_HXX
#define QUICKHULL_HXX

#include "lib.hxx"
#include "omp.h"

using namespace std;

void QuickHullOpenMPInit(
  Point *points,
  size_t numPoints,
  vector<Point> &hull
);

void QuickHullInit(
  Point* points,
  size_t numPoints,
  vector<Point> &hull
);

void QuickHull(
    std::vector<Point> &points,
    Point a, Point b,
    std::vector<Point> &hull,
    int iteration
);

#endif // QUICKHULL_HXX
