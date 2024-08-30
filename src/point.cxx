#include "point.hxx"

using namespace std;



string Point::toString() {
  stringstream ss;
  ss << "(" << x << ", " << y << ")";
  return ss.str();
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