#include "QuickHull.hxx"

using namespace std;

void QuickHullOpenMPInit(
  Point *points,
  size_t numPoints,
  vector<Point> &hull)
{
  clock_t start = clock();

  int batches = 8;
  size_t batchSize = numPoints / batches;
  cout << "Batch size: " << batchSize << endl;

  #pragma omp parallel
  #pragma omp single
  {
    for (int i = 0; i < batches; i++) {
      size_t start = i * batchSize;
      #pragma omp task
      {
        vector<Point> hullBatch;
        QuickHullInit(points + start, batchSize, hullBatch);
        #pragma omp critical
        {
            hull.insert(hull.end(), hullBatch.begin(), hullBatch.end());
        }
      }
    }
    #pragma omp taskwait
  }

  clock_t end = clock();
  double elapsed = double(end - start) / CLOCKS_PER_SEC;

  cout << "QuickHull took: " << elapsed * 1000 << "ms" << endl;
}

void QuickHullInit(
  Point *points,
  size_t numPoints,
  vector<Point> &hull)
{
#pragma region 2. extremes
  //================= 2. Find the 2 extreme points =================
  clock_t start = clock();

  // find the local extreme points
  Point left = points[0], right = points[0];

  // cout << "Starting with " << numPoints << " points" << endl;

  for (size_t i = 1; i < numPoints; i++)
  {
    if (points[i].x < left.x)
    {
      left = points[i];
    }
    else if (points[i].x > right.x)
    {
      right = points[i];
    }
  }

  hull.push_back(right);
  hull.push_back(left);

  clock_t end = clock();
  double elapsed = double(end - start) / CLOCKS_PER_SEC;

  // cout << "Finding the 2 extreme points took: " << elapsed * 1000 << "ms" << endl;
//================================================================
#pragma endregion

#pragma region 3. Split points
  //================= 3. Split points into two halves =================
  start = clock();

  vector<Point> upperHalf, lowerHalf = vector<Point>();
  upperHalf.reserve(numPoints / 2);
  lowerHalf.reserve(numPoints / 2);

  double dx = right.x - left.x;
  double dy = right.y - left.y;

  for (size_t i = 0; i < numPoints; i++) {
    // if its the left or right point, skip it
    if (points[i].x == left.x && points[i].y == left.y) { continue; }
    if (points[i].x == right.x && points[i].y == right.y) { continue; }

    double line = dy * (points[i].x - left.x) - dx * (points[i].y - left.y);

    if (line > 0) {
      upperHalf.push_back(points[i]);
    } else if (line < 0) {
      lowerHalf.push_back(points[i]);
    }
  }

  end = clock();
  elapsed = double(end - start) / CLOCKS_PER_SEC;

  // cout << "Splitting the points took: " << elapsed * 1000 << "ms" << endl;
//================================================================
#pragma endregion

#pragma region 4. Recursion
  //================= 4. Recurse on the two halves =================
  start = clock();

  QuickHull(upperHalf, left, right, hull, 0);
  QuickHull(lowerHalf, right, left, hull, 0);

  end = clock();
  elapsed = double(end - start) / CLOCKS_PER_SEC;

  // cout << "Recursing on the two halves took: " << elapsed * 1000 << "ms" << endl;
//================================================================
#pragma endregion
}

void QuickHull(
    vector<Point> &points,
    Point a, Point b,
    vector<Point> &hull,
    int iteration)
{
#pragma region 0. Termination
  //================= 0. If there are no points left, return =================
  int pointsLeft = points.size();

  if (pointsLeft == 0)
  {
    return;
  }
//================================================================
#pragma endregion

#pragma region 1. Furthest point
  //================= 1. Find the point with the maximum distance =================
  float maxDistance = -1;
  Point maxPoint = Point();

  LineDistanceCalculator lineDistanceCalculator(a, b);

  for (size_t i = 0; i < points.size(); i++)
  {
    double distance = lineDistanceCalculator.distanceFromLine(points[i]);

    if (distance > maxDistance)
    {
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
  size_t removedPoints = 0;

  for (size_t i = 0; i < points.size(); i++)
  {
    // if the point is the same as the max point, skip it
    if (points[i].x == maxPoint.x && points[i].y == maxPoint.y)
    {
      continue;
    }

    if (!insideTriangle(points[i], a, b, maxPoint))
    {
      newPoints.push_back(points[i]);
    }
    else
    {
      removedPoints++;
    }
  }

  points = newPoints;
//================================================================
#pragma endregion

#pragma region 4. Split points
  //================= 4. Split points into two halves =================
  vector<Point> upperHalf, lowerHalf;

  for (size_t i = 0; i < points.size(); i++)
  {
    // Skip the endpoints
    if ((points[i].x == a.x && points[i].y == a.y) ||
        (points[i].x == maxPoint.x && points[i].y == maxPoint.y))
    {
      continue;
    }

    // Use long long for intermediate calculations
    long long dx1 = (long long)maxPoint.x - a.x;
    long long dy1 = (long long)maxPoint.y - a.y;
    long long dx2 = (long long)points[i].x - a.x;
    long long dy2 = (long long)points[i].y - a.y;

    // Calculate the cross product
    long long crossProduct = dx1 * dy2 - dx2 * dy1;

    if (crossProduct > 0)
    {
      upperHalf.push_back(points[i]);
    }
    else if (crossProduct < 0)
    {
      lowerHalf.push_back(points[i]);
    }
    // Points exactly on the line (crossProduct == 0) are ignored
  }
//================================================================
#pragma endregion

#pragma region 5. Recursion
  //================= 4. Recurse on the two halves =================
  QuickHull(upperHalf, maxPoint, b, hull, iteration + 1);
  QuickHull(lowerHalf, a, maxPoint, hull, iteration + 1);
// ================================================================
#pragma endregion
}