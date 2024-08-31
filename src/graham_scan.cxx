#include "graham_scan.hxx"
using namespace std;

Point nextToTop(std::stack<Point> &stack) {
  Point p = stack.top();
  stack.pop();
  Point res = stack.top();
  stack.push(p);
  return res;
}

void graham_scan(
    Point *points,
    size_t numPoints,
    std::vector<Point> &hull,
    Timer *timer) {
  if (timer == nullptr) {
    timer = new Timer();
  }

  // find the bottommost point
  long ymin = points[0].y, min = 0;

  for (size_t i = 1; i < numPoints; i++) {
    long y = points[i].y;

    // pick the bottom-most or chose the left most point in case of tie
    if ((y < ymin) || (ymin == y && points[i].x < points[min].x)) {
      ymin = points[i].y, min = i;
    }
  }

  // place the bottom-most point at first position
  swap(points[0], points[min]);

  Point pivot = points[0];

  // sort n-1 points with respect to the first point. A point p1 comes before p2 in sorted output if p2 has larger polar angle (in counterclockwise direction) than p1
  // sort(points + 1, points + numPoints, comparator);
  sort(points, points + numPoints, [&pivot](const Point &a, const Point &b) {
    int o = orientation(pivot, a, b);
    if (o == 0) {
      return (pivot.x - a.x) * (pivot.x - a.x) + (pivot.y - a.y) * (pivot.y - a.y) < (pivot.x - b.x) * (pivot.x - b.x) + (pivot.y - b.y) * (pivot.y - b.y);
    }
    return o < 0;
  });

  
  hull = vector<Point>();

  for (int i = 0; i < numPoints; i++) {
    while (hull.size() > 1 && orientation(hull[hull.size()-2], hull.back(), points[i]) != -1)
      hull.pop_back();
    hull.push_back(points[i]);
  }
}