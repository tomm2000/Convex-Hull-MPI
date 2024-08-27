#ifndef POINT_GENERATOR_H
#define POINT_GENERATOR_H

#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include "point.hxx"

using namespace std;

enum class PointGeneratorType {
  CIRCLE,
  TORUS,
  GAUSSIAN,
  CIRCUMFERENCE,
  SQUARE,
  PERFECT_CIRCUMFERENCE,
};

vector<Point> generate_points(
  size_t numPoints,
  PointGeneratorType type,
  int seed = time(NULL),
  Point corner1 = {1000000, 1000000},
  Point corner2 = {-1000000, -1000000}
);

Point nextPoint(PointGeneratorType type, Point corner1, Point corner2);

void save_points_binary(std::vector<Point> &points, std::string filename);
void save_points_binary(Point *points, int numPoints, std::string filename);

void save_points_ascii(std::vector<Point> &points, std::string filename);
void save_points_ascii(Point *points, int numPoints, std::string filename);

#endif