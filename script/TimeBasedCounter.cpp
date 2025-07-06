#include <Arduino.h>

class TimeBasedCounter {
private:
  uint16_t times[3];            // Only stores ms within 60s
  const uint16_t withinTime = 5000; // 5 seconds

public:
  TimeBasedCounter() : times{0, 0, 0} {}

  bool addTimeAndCheck(uint16_t currentTime) {
    for (uint8_t i = 0; i < 3; i++) {
      if ((uint16_t)(currentTime - times[i]) > withinTime) {
        times[i] = currentTime;
        return false;
      }
    }

    reset();
    return true; // All slots are within window
  }

  void reset() {
    for (uint8_t i = 0; i < 3; i++) {
      times[i] = 0;
    }
  }

  uint8_t getCurrentShakeCounter(uint16_t currentTime) {
    uint8_t counter = 0;
    for (uint8_t i = 0; i < 3; i++) {
      if ((uint16_t)(currentTime - times[i]) <= withinTime) {
        counter++;
      }
    }
    return counter;
  }

  uint16_t getLatestTime() {
    uint16_t newest = 0;
    for (uint8_t i = 0; i < 3; i++) {
      if (times[i] > newest) {
        newest = times[i];
      }
    }
    return newest;
  }
};