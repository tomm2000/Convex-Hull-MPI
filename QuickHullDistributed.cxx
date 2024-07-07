#include "QuickHullDistributed.hxx"

using namespace std;

std::vector<Point> QuickHullDistributed(
  MPI_Datatype PointType, int numP, int rank,
  vector<Point> &points,
  Point a, Point b,
  vector<Point> &hull,
  int iteration
) {
  #pragma region 0. Termination
  //================= 0. If there are no points left, return =================
  #ifdef TIMING
  clock_t start_time = clock();
  #endif

  int pointsLeft = points.size();
  int *pointsLeftPerProcess = new int[numP];

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, local points left: " << pointsLeft << endl;
  #endif

  MPI_Allgather(&pointsLeft, 1, MPI_INT, pointsLeftPerProcess, 1, MPI_INT, MPI_COMM_WORLD);

  pointsLeft = 0;
  for (int i = 0; i < numP; i++) {
    pointsLeft += pointsLeftPerProcess[i];
  }

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, global points left: " << pointsLeft << endl;
  #endif

  #ifdef TIMING
  clock_t end_time = clock();
  double elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to check points left: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  if (pointsLeft == 0) {
    return vector<Point>();
  }
  else if (pointsLeft < 5000) {
    return points;
  }
  //================================================================
  #pragma endregion

  #pragma region 1. Furthest point
  //================= 1. Find the point with the maximum distance =================
  #ifdef TIMING
  start_time = clock();
  #endif
  
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

  bool valid = maxDistance > 0;

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, local max point: " << maxPoint.toString() << endl;
  #endif

  Point *maxPoints = new Point[numP];
  bool *validPoints = new bool[numP];

  MPI_Allgather(&valid, 1, MPI_C_BOOL, validPoints, 1, MPI_C_BOOL, MPI_COMM_WORLD);
  MPI_Allgather(&maxPoint, 1, PointType, maxPoints, 1, PointType, MPI_COMM_WORLD);

  maxDistance = -1;
  maxPoint = Point();

  for (int i = 0; i < numP; i++) {
    if (!validPoints[i]) { continue; }

    float tmpDistance = lineDistanceCalculator.distanceFromLine(maxPoints[i]);

    if (tmpDistance > maxDistance) {
      maxDistance = tmpDistance;
      maxPoint = maxPoints[i];
    }
  }

  // add the max point to the hull
  hull.push_back(maxPoint);

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, global max point: " << maxPoint.toString() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to find max point: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  #pragma region 3. Triangle
  //================= 3. Remove points inside the triangle =================
  #ifdef TIMING
  start_time = clock();
  #endif

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

  // free(points.data());
  points = newPoints;

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, removed points: " << removedPoints << ", points left: " << newPoints.size() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to remove points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif

  //================================================================
  #pragma endregion

  #pragma region 4. Split points
  //================= 4. Split points into two halves =================
  #ifdef TIMING
  start_time = clock();
  #endif

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

  #ifdef DEBUG
  cout << "<process " << rank << " - " << iteration << ">, upper half: " << upperHalf.size() << ", lower half: " << lowerHalf.size() << endl;
  #endif

  #ifdef TIMING
  end_time = clock();
  elapsed = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  if (elapsed * 1000 > 1) {
    cout << "<process " << rank << " - " << iteration << "> Time to split points: " << elapsed * 1000.0 << "ms" << endl;
  }
  #endif
  //================================================================
  #pragma endregion

  #pragma region 5. Recursion
  //================= 4. Recurse on the two halves =================
  // add return values to pointsLeft
  vector<Point> pointsLeftUpper = QuickHullDistributed(PointType, numP, rank, upperHalf, a, maxPoint, hull, iteration + 1);
  vector<Point> pointsLeftLower = QuickHullDistributed(PointType, numP, rank, lowerHalf, maxPoint, b, hull, iteration + 1);

  pointsLeftUpper.insert(pointsLeftUpper.end(), pointsLeftLower.begin(), pointsLeftLower.end());

  return pointsLeftUpper;
  //================================================================
  #pragma endregion
}