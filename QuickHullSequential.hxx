#ifndef QUICKHULLSEQUENTIAL_HXX
#define QUICKHULLSEQUENTIAL_HXX

#include "lib.hxx"

// Declaration of the QuickHull_sequential function
void QuickHull_sequential(
    std::vector<Point> &points,
    Point a, Point b,
    std::vector<Point> &hull,
    int iteration
);

#endif // QUICKHULLSEQUENTIAL_HXX
