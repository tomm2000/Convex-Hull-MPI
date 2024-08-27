#ifndef GRAHAM_SCAN_H
#define GRAHAM_SCAN_H

#include <vector>
#include <iostream>
#include <stack>
#include <algorithm>
#include "point.hxx"

// enum for convex hull algorithm
void graham_scan(Point points[], int numPoints, std::vector<Point> &hull);

#endif