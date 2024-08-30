#include "utils.hxx"

using namespace std;

Timer::Timer() {
  timers = unordered_map<string, double>();
  startTimes = unordered_map<string, TIMEPOINT>();
}

void Timer::start(string timerName) {
  startTimes[timerName] = TAKETIME;
}

void Timer::stop(string timerName) {
  TIMEPOINT endTime = TAKETIME;
  double delta = chrono::duration<double>(endTime - startTimes[timerName]).count();
  timers[timerName] += delta * 1000;
}

unordered_map<string, double> Timer::getTimers() {
  return timers;
}

double Timer::getTimer(string timerName) {
  return timers[timerName];
}

void Timer::printTimer(string timerName) {
  printf("%s: %fms\n", timerName.c_str(), timers[timerName]);
}

void Timer::printTimers() {
  for (auto const& kv : timers) {
    printf("%s: %fms\n", kv.first.c_str(), kv.second);
  }
}

string readArg(int argc, char *argv[], string argName, string defaultValue) {
  // [argname]=[value]
  string arg = argName + "=";
  for (int i = 1; i < argc; i++) {
    string argStr = argv[i];
    if (argStr.find(arg) == 0) {
      return argStr.substr(arg.size());
    }
  }

  return defaultValue;  
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(Point &p, Point &q, Point &r) {
  long val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (val == 0) return 0;  // collinear
  return (val > 0)? 1 : 2; // clock or counterclock wise
}

bool comparePoints(Point &a, Point &b, Point &pivot) {
  int o = orientation(pivot, a, b);
  if (o == 0) {
    return distSq(pivot, a) < distSq(pivot, b);
  }

  return o == 2;
}

long distSq(Point &p1, Point &p2) {
  return (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);
}

void merge(Point points[], size_t l, size_t m, size_t r, Point &pivot, Point L[], Point R[]) {
  size_t i, j, k;
  size_t n1 = m - l + 1;
  size_t n2 = r - m;

  // Copy data to temp arrays L[] and R[]
  for (i = 0; i < n1; i++) {
    L[i] = points[l + i];
  }
  for (j = 0; j < n2; j++) {
    R[j] = points[m + 1 + j];
  }

  // Merge the temp arrays back into points[l..r]
  i = 0;  // Initial index of first subarray
  j = 0;  // Initial index of second subarray
  k = l;  // Initial index of merged subarray
  while (i < n1 && j < n2) {
    // if (comp(L[i], R[j])) {
    if (comparePoints(L[i], R[j], pivot)) {
      points[k] = L[i];
      i++;
    } else {
      points[k] = R[j];
      j++;
    }
    k++;
  }

  // Copy the remaining elements of L[], if there are any
  while (i < n1) {
    points[k] = L[i];
    i++;
    k++;
  }

  // Copy the remaining elements of R[], if there are any
  while (j < n2) {
    points[k] = R[j];
    j++;
    k++;
  }
}

void mergeSort(Point points[], size_t l, size_t r, Point &pivot, Point L[], Point R[]) {
  if (l < r) {
    // Same as (l+r)/2, but avoids overflow for large l and r
    size_t m = l + (r - l) / 2;

    // Sort first and second halves
    mergeSort(points, l, m, pivot, L, R);
    mergeSort(points, m + 1, r, pivot, L, R);

    merge(points, l, m, r, pivot, L, R);
  }
}

void mergeSort(Point points[], size_t numPoints, Point &pivot) {
  Point *L = new Point[numPoints];
  Point *R = new Point[numPoints];
  
  mergeSort(points, 0, numPoints - 1, pivot, L, R);
}