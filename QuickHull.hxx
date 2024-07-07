#ifndef QUICKHULL_HXX
#define QUICKHULL_HXX

#include "lib.hxx"

using namespace std;

void QuickHullInit(
  Point* points,
  int numPoints,
  vector<Point> &hull
);

void QuickHull(
    std::vector<Point> &points,
    Point a, Point b,
    std::vector<Point> &hull,
    int iteration
);

#endif // QUICKHULL_HXX
