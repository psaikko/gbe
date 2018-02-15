#pragma once

#include <chrono>

using namespace std;
using namespace std::chrono;

class SyncTimer {
public:

  static inline SyncTimer& get() {
    static SyncTimer timer;
    return timer;
  }

  long long elapsed_ms() {
  	auto current_time = high_resolution_clock::now();
  	auto time_difference = current_time - start_time;
  	return duration_cast<milliseconds>(time_difference).count();
  }

  void start() {
  	start_time = high_resolution_clock::now();
  }

  time_point<high_resolution_clock> start_time;

private:
  SyncTimer() {};
  SyncTimer(SyncTimer const&);
  void operator=(SyncTimer const&);
};