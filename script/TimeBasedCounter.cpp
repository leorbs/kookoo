#include <Arduino.h>

#define COUNTER_SIZE        2

class TimeBasedCounter {
private:
  unsigned long times[COUNTER_SIZE];
  unsigned long withinTime;

public:
  TimeBasedCounter(unsigned long window = 5000) : withinTime(window) {
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      times[i] = 0 - withinTime;  // initialize with offset
    }
  }

  // Add current time and check if all COUNTER_SIZE are within the window
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

  // Reset all stored by shifting them back timestamps
  void reset() {
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      times[i] = times[i] - withinTime;
    }
  }

  // Count how many events occurred within the window
  uint8_t getCurrentShakeCounter(unsigned long currentTime) const {
    uint8_t counter = 0;
    for (uint8_t i = 0; i < COUNTER_SIZE; i++) {
      if ((unsigned long)(currentTime - times[i]) <= withinTime) {
#ifdef DEBUG
        Serial.print("Current time ");
        Serial.print(currentTime);
        Serial.print(" minus times[i] ");
        Serial.print(times[i]);
        Serial.print(" is ");
        Serial.print(currentTime - times[i]);
        Serial.print(" and therefore smaller than ");
        Serial.println(withinTime);
#endif
        counter++;
      }
    }
    return counter;
  }

  // Get the latest timestamp from the stored values
  unsigned long getLatestTime() const {
    unsigned long newest = times[0];
    for (uint8_t i = 1; i < COUNTER_SIZE; i++) {
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