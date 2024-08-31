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



int orientation(Point a, Point b, Point c) {
    double v = a.x*(b.y-c.y)+b.x*(c.y-a.y)+c.x*(a.y-b.y);
    if (v < 0) return -1; // clockwise
    if (v > 0) return +1; // counter-clockwise
    return 0;
}

