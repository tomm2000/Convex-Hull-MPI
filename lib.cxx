#include "lib.hxx"

using namespace std;

MPI_Datatype registerPointType() {
  // Create a new MPI datatype to represent a Point
  MPI_Datatype PointType;
  MPI_Datatype type[2] = {MPI_INT, MPI_INT};
  int blocklen[2] = {1, 1};
  MPI_Aint displacements[2];
  displacements[0] = offsetof(Point, x);
  displacements[1] = offsetof(Point, y);
  MPI_Type_create_struct(2, blocklen, displacements, type, &PointType);
  MPI_Type_commit(&PointType);

  return PointType;
}

LineDistanceCalculator::LineDistanceCalculator(const Point& a, const Point& b) {
  dx = b.x - a.x;
  dy = b.y - a.y;
  this->a = a;
  this->b = b;
  denominator = std::sqrt(dx * dx + dy * dy);
}

float LineDistanceCalculator::distanceFromLine(const Point& p) const {
  return std::abs(dy * p.x - dx * p.y + (b.x * a.y - b.y * a.x)) / denominator;
}

string Point::toString() {
  stringstream ss;
  ss << "(" << x << ", " << y << ")";
  return ss.str();
}

bool insideTriangle(Point &s, Point &a, Point &b, Point &c) {
  int as_x = s.x - a.x;
  int as_y = s.y - a.y;

  bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x > 0;

  if ((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0 == s_ab) 
      return false;
  if ((c.x - b.x) * (s.y - b.y) - (c.y - b.y)*(s.x - b.x) > 0 != s_ab) 
      return false;
  return true;
}

void savePointsToFile(vector<Point> points, string filename) {
  ofstream outputFile(filename);

  if (!outputFile.is_open())
  {
    cerr << "Error opening file: " << filename
         << std::endl;

    // Check for specific error conditions
    if (outputFile.bad())
    {
      cerr << "Fatal error: badbit is set." << endl;
    }

    if (outputFile.fail())
    {
      // Print a more detailed error message using strerror
      cerr << "Error details: " << strerror(errno)
           << endl;
    }

    // Handle the error or exit the program
    return;
  }

  for (int i = 0; i < points.size(); i++)
  {
    outputFile << points[i].x << " " << points[i].y << endl;
  }
}
