#pragma once

#include "Arduino.h"

struct SoundParams {
  uint8_t folderId;
  bool triggerBird;
  uint16_t* flapBreakPattern;  // Pointer to an array of uint16_t
  uint16_t* flapPattern;       // Pointer to an array of uint16_t
  uint8_t flapBreakPatternSize;    // Size of the flapBreakPattern array
  uint8_t flapPatternSize;         // Size of the flapPattern array
};

// Call this to generate 15s of speech-like flapping
void generateSpeechLikeFlappingPattern(SoundParams& params);