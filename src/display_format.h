#pragma once

#include <stdint.h>
#include "models.h"

struct DisplayFrame {
  char chars[5] = "----";   // Up to four display characters, no decimal point.
  int8_t decimalAfter = -1;  // Character index in chars after which the decimal point is lit.
  bool valid = false;
  bool scaledThousands = false;
};

bool formatMetricFrame(MetricId metric, const NumericValue& value, DisplayFrame& out);

struct ClockFrame {
  char chars[5] = "----";  // Four physical positions, a space means blank.
  bool colonVisible = false;
};

// Format 24-hour time for the physical four-digit display. Single-digit hours
// occupy only the second digit so the complete time stays right-aligned.
bool formatClockFrame(uint8_t hour, uint8_t minute, uint8_t second, ClockFrame& out);

// The TM1637 center colon is lit on even seconds and dark on odd seconds.
bool clockColonVisible(uint8_t second);
