#include <Arduino.h>

#define COUNTER_SIZE        2

class TimeBasedCounter {
private:
  unsigned long times[COUNTER_SIZE];
  unsigned long withinTime;

public:
  TimeBasedCounter(unsigned long window = 5000) : times{}, withinTime(window) {}

  // Add current time and check if all three are within the window
  bool addTimeAndCheck(unsigned long currentTime) {
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      if ((unsigned long)(currentTime - times[i]) > withinTime) {
        times[i] = currentTime;
        return false;
      }
    }

    reset();
    return true; // All timestamps are within the window
  }

  // Reset all stored timestamps
  void reset() {
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      times[i] = 0;
    }
  }

  // Count how many events occurred within the window
  uint8_t getCurrentShakeCounter(unsigned long currentTime) const {
    uint8_t counter = 0;
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      if ((unsigned long)(currentTime - times[i]) <= withinTime) {
        counter++;
      }
    }
    return counter;
  }

  // Get the latest timestamp from the stored values
  unsigned long getLatestTime() const {
    unsigned long newest = 0;
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      // For explanation, google
      // "Modular (or Unsigned) Time Comparison"
      // "Half-range unsigned comparison"
      if ((unsigned long)(times[i] - newest) < 0x80000000UL) {
        newest = times[i];
      }
    }
    return newest;
  }
};