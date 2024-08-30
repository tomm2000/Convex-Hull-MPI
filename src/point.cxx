#include "point.hxx"

using namespace std;

long distSq(Point p1, Point p2) {
  return (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);
}

string Point::toString() {
  stringstream ss;
  ss << "(" << x << ", " << y << ")";
  return ss.str();
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(Point p, Point q, Point r) {
  long val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (val == 0) return 0;  // collinear
  return (val > 0)? 1 : 2; // clock or counterclock wise
}

void swap(Point &p1, Point &p2) {
  Point temp = p1;
  p1 = p2;
  p2 = temp;
}

MPI_Datatype registerPointType() {
  // Create a new MPI datatype to represent a Point
  MPI_Datatype PointType;
  MPI_Datatype type[2] = {MPI_LONG, MPI_LONG};
  int blocklen[2] = {1, 1};
  MPI_Aint displacements[2];
  displacements[0] = offsetof(Point, x);
  displacements[1] = offsetof(Point, y);
  MPI_Type_create_struct(2, blocklen, displacements, type, &PointType);
  MPI_Type_commit(&PointType);

  return PointType;
}

int estimateMemoryUsage(size_t numPoints) {
  // in mb
  return sizeof(Point) * numPoints / 1024 / 1024;
}