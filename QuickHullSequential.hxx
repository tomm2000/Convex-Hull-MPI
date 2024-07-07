#ifndef QUICKHULLSEQUENTIAL_HXX
#define QUICKHULLSEQUENTIAL_HXX

#include "lib.hxx"

using namespace std;

void QuickHullInit_sequential(
  Point* points,
  int numPoints,
  vector<Point> &hull
);

void QuickHull_sequential(
    std::vector<Point> &points,
    Point a, Point b,
    std::vector<Point> &hull,
    int iteration
);

#endif // QUICKHULLSEQUENTIAL_HXX
