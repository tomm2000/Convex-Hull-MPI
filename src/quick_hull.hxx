#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include "point.hxx"
#include "utils.hxx"
#include <cmath>
#include <vector>

using namespace std;


// Class for calculating distance from a line
class LineDistanceCalculator {
private:
  long long dx, dy;
  Point a, b;
  double denominator;

public:
  LineDistanceCalculator(const Point& a, const Point& b);
  double distanceFromLine(const Point& p) const;
};

// Function to check if a point is inside a triangle
bool insideTriangle(Point &s, Point &a, Point &b, Point &c);

void quick_hull(
  Point *points,
  size_t numPoints,
  vector<Point> &hull,
  Timer *timer = nullptr
);

void quick_hull_recursive(
  vector<Point> &points,
  Point a, Point b,
  vector<Point> &hull,
  int iteration,
  Timer *timer = nullptr
);

#endif // QUICK_HULL_H