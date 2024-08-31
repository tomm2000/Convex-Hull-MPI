#ifndef POINT_H
#define POINT_H

#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#include "mpi.h"

struct Point {
  long x, y;
  std::string toString();
};



void swap(Point &p1, Point &p2);

MPI_Datatype registerPointType();

int estimateMemoryUsage(size_t numPoints);

#endif