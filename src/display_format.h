#pragma once

#include <stdint.h>
#include "models.h"

struct DisplayFrame {
  char chars[5] = "----";   // Up to four display characters, no decimal point.
  int8_t decimalAfter = -1;  // Character index in chars after which the decimal point is lit.
  bool valid = false;
  bool degreeSuffix = false;  // Render a dedicated seven-segment degree symbol in the last digit.
};

// The only external value displayed by espDisplay is air temperature. It is
// rounded to a whole degree and rendered with a dedicated degree symbol.
bool formatTemperatureFrame(const NumericValue& value, DisplayFrame& out);

struct ClockFrame {
  char chars[5] = "----";  // Four physical positions, a space means blank.
  bool colonVisible = false;
};

bool formatClockFrame(uint8_t hour, uint8_t minute, uint8_t second, ClockFrame& out);
bool clockColonVisible(uint8_t second);

bool alternateClockVisible(uint32_t elapsedMs, uint16_t clockSeconds,
                           uint32_t apiValueDisplayMs);
bool alternateDisplayShowsClock(uint32_t elapsedMs, uint16_t clockSeconds,
                                uint32_t apiValueDisplayMs, bool apiValueAvailable);

uint8_t degreeSymbolSegment(bool staleWarning = false);
bool apiDataStaleForDisplay(uint32_t ageMs, uint16_t pollSeconds);
void connectingSegments(uint8_t out[4]);
