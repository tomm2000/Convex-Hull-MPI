#include "lib.hxx"

using namespace std;

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

LineDistanceCalculator::LineDistanceCalculator(const Point& a, const Point& b) : a(a), b(b) {
  dx = static_cast<long long>(b.x) - a.x;
  dy = static_cast<long long>(b.y) - a.y;
  denominator = std::sqrt(static_cast<double>(dx) * dx + static_cast<double>(dy) * dy);
}

double LineDistanceCalculator::distanceFromLine(const Point& p) const {
    if (denominator == 0) {
        return std::numeric_limits<double>::infinity();
    }
    long long numerator = std::abs(dy * static_cast<long long>(p.x) - 
                                   dx * static_cast<long long>(p.y) + 
                                   static_cast<long long>(b.x) * a.y - 
                                   static_cast<long long>(b.y) * a.x);
    return static_cast<double>(numerator) / denominator;
}

string Point::toString() {
  stringstream ss;
  ss << "(" << x << ", " << y << ")";
  return ss.str();
}

bool insideTriangle(Point &s, Point &a, Point &b, Point &c) {
  long long as_x = (long long)s.x - a.x;
  long long as_y = (long long)s.y - a.y;
  
  bool s_ab = ((long long)b.x - a.x) * as_y - ((long long)b.y - a.y) * as_x > 0;
  if (((long long)c.x - a.x) * as_y - ((long long)c.y - a.y) * as_x > 0 == s_ab)
      return false;

  if (((long long)c.x - b.x) * (s.y - b.y) - ((long long)c.y - b.y) * (s.x - b.x) > 0 != s_ab)
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


void savePointsToBinary(std::vector<Point> points, std::string filename) {
  ofstream outputFile(filename, ios::binary);

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

  size_t size = points.size();
  outputFile.write(reinterpret_cast<char*>(&size), sizeof(size_t));

  for (size_t i = 0; i < size; i++) {
    outputFile.write(reinterpret_cast<char*>(&points[i].x), sizeof(long));
    outputFile.write(reinterpret_cast<char*>(&points[i].y), sizeof(long));
  }
}
