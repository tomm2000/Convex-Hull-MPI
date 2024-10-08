#include "point_generator.hxx"

using namespace std;

#define TWO_PI 6.28318530718

void generate_points(
    size_t numPoints,
    Point *points,
    PointGeneratorType type,
    int seed,
    Point corner1,
    Point corner2) {
  
  srand(seed);

  int radius = min(abs(corner1.x - corner2.x), abs(corner1.y - corner2.y)) / 2;
  for (size_t i = 0; i < numPoints; i++) {
    nextPoint(type, &points[i], radius);
  }
}

void nextPoint(PointGeneratorType type, Point *point, int radius) {
  if (type == PointGeneratorType::CIRCLE) {
    double r = std::sqrt(rand() / (double)RAND_MAX) * radius;
    double angle = (rand() / (double)RAND_MAX) * TWO_PI;

    point->x = r * std::cos(angle);
    point->y = r * std::sin(angle);
  } else {
    point->x = 0;
    point->y = 0;
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
  file.write(reinterpret_cast<char *>(&size_t_bytes), sizeof(int));

  // second (|int| bytes = 4) is the size of each point
  int point_size = sizeof(long) * 2;
  file.write(reinterpret_cast<char *>(&point_size), sizeof(int));

  // write the number of points (|size_t| bytes)
  file.write(reinterpret_cast<char *>(&numPoints), sizeof(size_t));

  long buffer[BUFFER_SIZE * 2];

  for (size_t i = 0; i < numPoints; i += BUFFER_SIZE) {
    size_t pointsToWrite = BUFFER_SIZE;
    if (i + pointsToWrite > numPoints) {
      pointsToWrite = numPoints - i;
    }

    for (size_t j = 0; j < pointsToWrite; j++) {
      buffer[j * 2] = points[i + j].x;
      buffer[j * 2 + 1] = points[i + j].y;
    }

    file.write(reinterpret_cast<char *>(buffer), pointsToWrite * 2 * sizeof(long));
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