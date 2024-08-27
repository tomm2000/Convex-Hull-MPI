#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <unordered_map>
#include <chrono>
#include <string>
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

string readArg(int argc, char *argv[], string argName, string defaultValue = "");
#endif