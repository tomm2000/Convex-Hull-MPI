#ifndef GRAHAM_SCAN_H
#define GRAHAM_SCAN_H

#include <vector>
#include <iostream>
#include <stack>
#include <algorithm>
#include "point.hxx"
#include "utils.hxx"

// enum for convex hull algorithm
void graham_scan(
  Point *points,
  size_t numPoints,
  std::vector<Point> &hull,
  Timer &timer = Timer()
);

#endif