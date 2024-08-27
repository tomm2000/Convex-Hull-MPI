#include "timer.hxx"

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
  timers[timerName] += delta;
}

unordered_map<string, double> Timer::getTimers() {
  return timers;
}

double Timer::getTimer(string timerName) {
  return timers[timerName];
}

void Timer::printTimer(string timerName) {
  printf("%s: %fs\n", timerName.c_str(), timers[timerName]);
}

void Timer::printTimers() {
  for (auto const& [key, val] : timers) {
    printf("%s: %fs\n", key.c_str(), val);
  }
}