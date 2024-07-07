#include "QuickHullSequential.hxx"

using namespace std;

void QuickHull_sequential(
  vector<Point> &points,
  Point a, Point b,
  vector<Point> &hull,
  int iteration
) {
  #pragma region 0. Termination
  //================= 0. If there are no points left, return =================
  int pointsLeft = points.size();

  if (pointsLeft == 0) {
    return;
  }
  //================================================================
  #pragma endregion

  #pragma region 1. Furthest point
  //================= 1. Find the point with the maximum distance =================
  float maxDistance = -1;
  Point maxPoint = Point();

  LineDistanceCalculator lineDistanceCalculator(a, b);

  for (int i = 0; i < points.size(); i++) {
    float distance = lineDistanceCalculator.distanceFromLine(points[i]);

    if (distance > maxDistance) {
      maxDistance = distance;
      maxPoint = points[i];
    }
  }

  // add the max point to the hull
  hull.push_back(maxPoint);
  //================================================================
  #pragma endregion

  #pragma region 3. Triangle
  //================= 3. Remove points inside the triangle =================
  vector<Point> newPoints;
  int removedPoints = 0;

  for (int i = 0; i < points.size(); i++) {
    // if the point is the same as the max point, skip it
    if (points[i].x == maxPoint.x && points[i].y == maxPoint.y) {
      continue;
    }

    if (!insideTriangle(points[i], a, b, maxPoint)) {
      newPoints.push_back(points[i]);
    } else {
      removedPoints++;
    }
  }

  points = newPoints;
  //================================================================
  #pragma endregion

  #pragma region 4. Split points
  //================= 4. Split points into two halves =================
  vector<Point> upperHalf, lowerHalf;

  for (int i = 0; i < points.size(); i++) {
    float line = (maxPoint.y - a.y) * (points[i].x - a.x) - (maxPoint.x - a.x) * (points[i].y - a.y);

    // if its the left or right point, skip it
    if (points[i].x == a.x && points[i].y == a.y) {
      continue;
    }

    if (points[i].x == maxPoint.x && points[i].y == maxPoint.y) {
      continue;
    }

    if (line > 0) {
      upperHalf.push_back(points[i]);
    } else if (line < 0) {
      lowerHalf.push_back(points[i]);
    }
  }
  //================================================================
  #pragma endregion

  #pragma region 5. Recursion
  //================= 4. Recurse on the two halves =================
  QuickHull_sequential(upperHalf, a, maxPoint, hull, iteration + 1);
  QuickHull_sequential(lowerHalf, maxPoint, b, hull, iteration + 1);
  //================================================================
  #pragma endregion
}