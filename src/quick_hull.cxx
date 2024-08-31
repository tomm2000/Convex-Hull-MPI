#include "quick_hull.hxx"
#include <vector>

#pragma region Utility
LineDistanceCalculator::LineDistanceCalculator(const Point& a, const Point& b) : a(a), b(b) {
  dx = static_cast<long long>(b.x) - a.x;
  dy = static_cast<long long>(b.y) - a.y;
  denominator = std::sqrt(static_cast<double>(dx) * dx + static_cast<double>(dy) * dy);
}

double LineDistanceCalculator::distanceFromLine(const Point& p) const {
    if (denominator == 0) {
        return std::numeric_limits<double>::infinity();
    }
    long long numerator = std::abs(dy * static_cast<long long>(p.x) - 
                                   dx * static_cast<long long>(p.y) + 
                                   static_cast<long long>(b.x) * a.y - 
                                   static_cast<long long>(b.y) * a.x);
    return static_cast<double>(numerator) / denominator;
}

bool insideTriangle(Point &s, Point &a, Point &b, Point &c) {
  long long as_x = (long long)s.x - a.x;
  long long as_y = (long long)s.y - a.y;
  
  bool s_ab = ((long long)b.x - a.x) * as_y - ((long long)b.y - a.y) * as_x > 0;
  if (((long long)c.x - a.x) * as_y - ((long long)c.y - a.y) * as_x > 0 == s_ab)
      return false;

  if (((long long)c.x - b.x) * (s.y - b.y) - ((long long)c.y - b.y) * (s.x - b.x) > 0 != s_ab)
      return false;
  return true;
}
#pragma endregion

#pragma region QuickHull
void quick_hull(
  Point *points,
  size_t numPoints,
  vector<Point> &hull,
  Timer *timer
) {
  if (timer == nullptr) { timer = new Timer(); }

  // 1. Find the leftmost and rightmost points
  Point leftmost, rightmost = points[0];

  for (size_t i = 1; i < numPoints; i++) {
    if (points[i].x < leftmost.x) {
      leftmost = points[i];
    } else if (points[i].x > rightmost.x) {
      rightmost = points[i];
    }
  }

  // add the leftmost and rightmost points to the hull
  hull.push_back(leftmost);
  hull.push_back(rightmost);

  // 2. Split the points into two groups
  vector<Point> upperGroup, lowerGroup = vector<Point>();
  upperGroup.reserve(numPoints / 2);
  lowerGroup.reserve(numPoints / 2);

  LineDistanceCalculator line(leftmost, rightmost);

  for (size_t i = 0; i < numPoints; i++) {
    if (points[i].x == leftmost.x && points[i].y == leftmost.y) { continue; }
    if (points[i].x == rightmost.x && points[i].y == rightmost.y) { continue; }

    double distance = line.distanceFromLine(points[i]);
    if (distance > 0) {
      upperGroup.push_back(points[i]);
    } else if (distance < 0) {
      lowerGroup.push_back(points[i]);
    }
  }

  // 3. Recursively find the hulls
  quick_hull_recursive(upperGroup, leftmost, rightmost, hull, 0, timer);
  quick_hull_recursive(lowerGroup, rightmost, leftmost, hull, 0, timer);
}

void quick_hull_recursive(
  vector<Point> &points,
  Point a, Point b,
  vector<Point> &hull,
  int iteration,
  Timer *timer
) {
  if (timer == nullptr) { timer = new Timer(); }
  
  // 0. if there are no points, return
  if (points.size() == 0) { return; }

  // 1. find the point with the maximum distance from the line ab
  double maxDistance = 0;
  Point maxPoint;

  LineDistanceCalculator line(a, b);

  for (size_t i = 0; i < points.size(); i++) {
    double distance = line.distanceFromLine(points[i]);
    if (distance > maxDistance) {
      maxDistance = distance;
      maxPoint = points[i];
    }
  }

  hull.push_back(maxPoint);

  // 2. Remove points inside the triangle formed by a, b, and maxPoint
  vector<Point> newPoints;
  size_t removed = 0;

  for (size_t i = 0; i < points.size(); i++) {
    if (points[i].x == maxPoint.x && points[i].y == maxPoint.y) { continue; }

    if (insideTriangle(points[i], a, b, maxPoint)) {
      removed++;
    } else {
      newPoints.push_back(points[i]);
    }
  }

  points = newPoints;

  // 3. split the remaining points into two groups
  vector<Point> upperGroup, lowerGroup = vector<Point>();
  upperGroup.reserve(points.size() / 2);
  lowerGroup.reserve(points.size() / 2);

  for (size_t i = 0; i < points.size(); i++) {
    if (points[i].x == a.x && points[i].y == a.y) { continue; }
    if (points[i].x == b.x && points[i].y == b.y) { continue; }

    // Use long long for intermediate calculations
    long long dx1 = (long long) maxPoint.x - a.x;
    long long dy1 = (long long) maxPoint.y - a.y;
    long long dx2 = (long long) points[i].x - a.x;
    long long dy2 = (long long) points[i].y - a.y;

    // calculate the cross product
    long long crossProduct = dx1 * dy2 - dx2 * dy1;

    if (crossProduct > 0) {
      upperGroup.push_back(points[i]);
    } else if (crossProduct < 0) {
      lowerGroup.push_back(points[i]);
    }
  }

  // 4. Recursively find the hulls
  quick_hull_recursive(upperGroup, a, maxPoint, hull, iteration + 1, timer);
  quick_hull_recursive(lowerGroup, maxPoint, b, hull, iteration + 1, timer);
}
#pragma endregion