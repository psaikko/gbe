#pragma once

#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

class SyncTimer {
  public:
    static inline SyncTimer &get() {
        static SyncTimer timer;
        return timer;
    }

    long long elapsed_ms() const {
        auto current_time = high_resolution_clock::now();
        return duration_cast<milliseconds>(current_time - start_time).count() + offset;
    }

    void start() {
        start_time = high_resolution_clock::now();
    }

    time_point<high_resolution_clock> start_time;
    long long offset;

  private:
    SyncTimer() : offset(0){};
    SyncTimer(SyncTimer const &)      = delete;
    void operator=(SyncTimer const &) = delete;

    friend std::ostream &operator<<(std::ostream &out, const SyncTimer &st);

    friend std::istream &operator>>(std::istream &in, SyncTimer &st);
};