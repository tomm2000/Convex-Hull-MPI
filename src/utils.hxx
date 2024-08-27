#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include "hpc_helpers.hpp" 
#include "point.hxx"

using namespace std;

#define TIMEPOINT std::chrono::time_point<std::chrono::system_clock>
#define TAKETIME std::chrono::system_clock::now()

class Timer {
 private:
  // hashmap of timers
  unordered_map<string, double> timers;
  unordered_map<string, TIMEPOINT> startTimes;

 public:
  Timer();
  void start(string timerName);
  void stop(string timerName);
  unordered_map<string, double> getTimers();
  double getTimer(string timerName);
  void printTimer(string timerName);
  void printTimers();
};

string readArg(int argc, char* argv[], string argName, string defaultValue = "");

bool comparePoints(Point a, Point b, Point pivot);

void merge(Point *points, size_t l, size_t m, size_t r, Point pivot);

void mergeSort(Point *points, size_t l, size_t r, Point pivot);
#endif