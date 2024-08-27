#include "graham_scan.hxx"
using namespace std;

Point nextToTop(std::stack<Point> &stack) {
  Point p = stack.top();
  stack.pop();
  Point res = stack.top();
  stack.push(p);
  return res;
}

void graham_scan(Point points[], int numPoints, std::vector<Point> &hull) {
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

  auto comp = [pivot](Point &a, Point &b) {
    // Find orientation
    Orientation o = orientation(pivot, a, b);
    if (o == Orientation::COLLINEAR) {
      return distSq(pivot, a) > distSq(pivot, b);
    }

    return o == Orientation::COUNTERCLOCKWISE;
  };

  // sort n-1 points with respect to the first point. A point p1 comes before p2 in sorted output if p2 has larger polar angle (in counterclockwise direction) than p1
  sort(points + 1, points + numPoints, comp);

  // If two or more points make same angle with p0,
  // Remove all but the one that is farthest from p0
  // Remember that, in above sorting, our criteria was
  // to keep the farthest point at the end when more than
  // one points have same angle.
  size_t m = 1; // Initialize size of modified array
  for (size_t i=1; i < numPoints; i++) {
    // Keep removing i while angle of i and i+1 is same
    // with respect to p0
    // while (i < n-1 && orientation(p0, points[i],
    //                             points[i+1]) == 0)
    //   i++;
    while (i < numPoints-1 && orientation(points[0], points[i], points[i+1]) == Orientation::COLLINEAR) {
      i++;
    }
    
    points[m] = points[i];
    m++;  // Update size of modified array
  }

  // If modified array of points has less than 3 points, convex hull is not possible
  if (m < 3) { return; }

  // Create an empty stack and push first three points to it
  std::stack<Point> s;
  s.push(points[0]);
  s.push(points[1]);
  s.push(points[2]);

  // Process remaining n-3 points
  for (size_t i = 3; i < m; i++) {
    // Keep removing top while the angle formed by points next-to-top, top, and points[i] makes a non-left turn
    while (orientation(nextToTop(s), s.top(), points[i]) != Orientation::COUNTERCLOCKWISE) {
      s.pop();
    }
    s.push(points[i]);
  }

  // Now stack has the output points, copy contents of stack to hull
  while (!s.empty()) {
    hull.push_back(s.top());
    s.pop();
  }
}