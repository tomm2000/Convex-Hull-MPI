#ifndef LIB_H
#define LIB_H

#include "mpi.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <sstream>

// Define enum for messages
enum messages {msg_tag, eos_tag, empty_points};

// Define the Point struct
struct Point {
    int x, y;

    std::string toString();
};

// Function to register MPI datatype for Point
MPI_Datatype registerPointType();

// Class for calculating distance from a line
class LineDistanceCalculator {
private:
    long long dx, dy;
    Point a, b;
    double denominator;

public:
    LineDistanceCalculator(const Point& a, const Point& b);
    double distanceFromLine(const Point& p) const;
};

// Function to check if a point is inside a triangle
bool insideTriangle(Point &s, Point &a, Point &b, Point &c);

// Function to save points to a file
void savePointsToFile(std::vector<Point> points, std::string filename);

#endif // LIB_H
