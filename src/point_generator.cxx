#include "point_generator.hxx"

using namespace std;

vector<Point> generate_points(
  size_t numPoints,
  PointGeneratorType type,
  int seed,
  Point corner1,
  Point corner2
) {
  srand(seed);
  vector<Point> points;
  points.reserve(numPoints);
  for (int i = 0; i < numPoints; i++) {
    points.push_back(nextPoint(type, corner1, corner2));
  }
  return points;
}

Point nextPoint(PointGeneratorType type, Point corner1, Point corner2) {
  // TODO: implement other point generation types, and make cleaner
  if (type == PointGeneratorType::CIRCLE) {
    int radius = min(abs(corner1.x - corner2.x), abs(corner1.y - corner2.y)) / 2;

    // generate points uniformly distributed in a circle
    double angle = (double) rand() / RAND_MAX * 2 * 3.14159265359;
    double r = sqrt((double) rand() / RAND_MAX) * radius;

    long x = r * cos(angle);
    long y = r * sin(angle);

    return {x, y};
  } else {
    return {0, 0};
  }
}

#define BUFFER_SIZE 1024

void save_points_binary(std::vector<Point> &points, std::string filename) {
  Point *pointsArray = points.data();
  save_points_binary(pointsArray, points.size(), filename);
}

void save_points_binary(Point *points, int numPoints, std::string filename) {
  ofstream file;
  file.open(filename);

  // first (|int| bytes = 4) bytes is the number of bytes used by size_t
  int size_t_bytes = sizeof(size_t);
  file.write(reinterpret_cast<char*>(&size_t_bytes), sizeof(int));

  // second (|int| bytes = 4) is the size of each point
  int point_size = sizeof(long) * 2;
  file.write(reinterpret_cast<char*>(&point_size), sizeof(int));

  // write the number of points (|size_t| bytes)
  file.write(reinterpret_cast<char*>(&numPoints), sizeof(size_t));

  long buffer[BUFFER_SIZE * 2];

  for (size_t i = 0; i < numPoints; i += BUFFER_SIZE) {
    size_t pointsToWrite = BUFFER_SIZE;
    if (i + pointsToWrite > numPoints) { pointsToWrite = numPoints - i; }

    for (size_t j = 0; j < pointsToWrite; j++) {
      buffer[j * 2] = points[i + j].x;
      buffer[j * 2 + 1] = points[i + j].y;
    }

    file.write(reinterpret_cast<char*>(buffer), pointsToWrite * 2 * sizeof(long));
  }

  file.close();
}



void save_points_ascii(std::vector<Point> &points, std::string filename) {
  Point *pointsArray = points.data();
  save_points_ascii(pointsArray, points.size(), filename);
}

void save_points_ascii(Point *points, int numPoints, std::string filename) {
  ofstream file;
  file.open(filename);

  for (int i = 0; i < numPoints; i++) {
    file << points[i].x << " " << points[i].y << endl;
  }

  file.close();
}