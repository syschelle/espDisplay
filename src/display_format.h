#pragma once

#include <stdint.h>
#include "models.h"

struct DisplayFrame {
  char chars[5] = "----";   // Up to four display characters, no decimal point.
  int8_t decimalAfter = -1;  // Character index in chars after which the decimal point is lit.
  bool valid = false;
  bool scaledThousands = false;
  bool degreeSuffix = false;  // Render a dedicated seven-segment degree symbol in the last digit.
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

// In alternate mode the clock phase is configured in seconds and the
// API-value phase is configured independently in milliseconds.
bool alternateClockVisible(uint32_t elapsedMs, uint16_t clockSeconds,
                           uint32_t apiValueDisplayMs);

// Alternate mode must remain on the clock until the selected API value has
// actually been received after boot. Once available, normal timing applies.
bool alternateDisplayShowsClock(uint32_t elapsedMs, uint16_t clockSeconds,
                                uint32_t apiValueDisplayMs, bool apiValueAvailable);

// Raw TM1637 seven-segment approximation of the degree symbol. When staleWarning
// is true, segment D (the lower horizontal segment) is added as a visible stale-data indicator.
uint8_t degreeSymbolSegment(bool staleWarning = false);

// Physical-display stale policy for last-known-good API data. The value becomes
// stale after the greater of 30 seconds or two configured polling intervals.
bool apiDataStaleForDisplay(uint32_t ageMs, uint16_t pollSeconds);

// TM1637 segment pattern for the boot/wait state "Conn".
void connectingSegments(uint8_t out[4]);
