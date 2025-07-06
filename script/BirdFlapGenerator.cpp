#include "BirdFlapGenerator.h"

// --- Constants ---
const uint16_t TOTAL_DURATION_MS = 15000;
const int MAX_FLAPS = 170; //never above 254

const uint16_t FLAP_MIN = 50;
const uint16_t FLAP_MAX = 200;
const uint16_t BREAK_MIN = 50;
const uint16_t BREAK_MAX = 100;

const uint16_t PAUSE_BREAK_MIN = 250;
const uint16_t PAUSE_BREAK_MAX = 600;

const int BURST_MIN_FLAPS = 2;
const int BURST_MAX_FLAPS = 6;

// --- Buffers ---
static uint16_t flapBuffer[MAX_FLAPS];
static uint16_t breakBuffer[MAX_FLAPS];

// --- Internal Utility ---
static uint16_t randomBetween(uint16_t minVal, uint16_t maxVal) {
  return random(minVal, maxVal + 1);
}

// --- Generator Implementation ---
void generateSpeechLikeFlappingPattern(SoundParams& params) {
  uint16_t timeSoFar = 0;
  int index = 0;

  while (timeSoFar < TOTAL_DURATION_MS && index < MAX_FLAPS) {
    int flapsInBurst = random(BURST_MIN_FLAPS, BURST_MAX_FLAPS + 1);

    for (int j = 0; j < flapsInBurst && timeSoFar < TOTAL_DURATION_MS; j++) {
      uint16_t flapTime = randomBetween(FLAP_MIN, FLAP_MAX);
      uint16_t breakTime = randomBetween(BREAK_MIN, BREAK_MAX);

      if (timeSoFar + flapTime + breakTime > TOTAL_DURATION_MS) break;

      flapBuffer[index] = flapTime;
      breakBuffer[index] = breakTime;
      timeSoFar += flapTime + breakTime;
      index++;
    }

    // Add longer pause (like a speech pause)
    if (timeSoFar + PAUSE_BREAK_MIN < TOTAL_DURATION_MS && index < MAX_FLAPS) {
      flapBuffer[index] = 0;
      breakBuffer[index] = randomBetween(PAUSE_BREAK_MIN, PAUSE_BREAK_MAX);
      timeSoFar += breakBuffer[index];
      index++;
    }
  }

  // Assign generated data to the struct
  params.flapPattern = flapBuffer;
  params.flapBreakPattern = breakBuffer;
  params.flapPatternSize = index;
  params.flapBreakPatternSize = index;
}