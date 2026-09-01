#include "display_format.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "text_utils.h"

bool formatTemperatureFrame(const NumericValue& input, DisplayFrame& out) {
  out = DisplayFrame();
  if (!input.valid || !isfinite(input.value)) return false;

  const long rounded = lroundf(input.value);
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%ld", rounded);

  // One of the four TM1637 digits is reserved for the degree symbol, leaving
  // three positions for the signed whole-degree temperature.
  if (strlen(buffer) > 3) return false;

  copyText(out.chars, buffer);
  out.decimalAfter = -1;
  out.valid = true;
  out.degreeSuffix = true;
  return true;
}

bool formatClockFrame(uint8_t hour, uint8_t minute, uint8_t second, ClockFrame& out) {
  out = ClockFrame();
  if (hour > 23U || minute > 59U || second > 59U) return false;

  if (hour < 10U) {
    out.chars[0] = ' ';
    out.chars[1] = static_cast<char>('0' + hour);
  } else {
    out.chars[0] = static_cast<char>('0' + (hour / 10U));
    out.chars[1] = static_cast<char>('0' + (hour % 10U));
  }
  out.chars[2] = static_cast<char>('0' + (minute / 10U));
  out.chars[3] = static_cast<char>('0' + (minute % 10U));
  out.chars[4] = '\0';
  out.colonVisible = clockColonVisible(second);
  return true;
}

bool clockColonVisible(uint8_t second) {
  return (second % 2U) == 0U;
}

bool alternateClockVisible(uint32_t elapsedMs, uint16_t clockSeconds,
                           uint32_t apiValueDisplayMs) {
  if (clockSeconds == 0U || apiValueDisplayMs == 0U) return false;

  const uint32_t clockMs = static_cast<uint32_t>(clockSeconds) * 1000UL;
  const uint32_t cycleMs = clockMs + apiValueDisplayMs;
  return (elapsedMs % cycleMs) < clockMs;
}

bool alternateDisplayShowsClock(uint32_t elapsedMs, uint16_t clockSeconds,
                                uint32_t apiValueDisplayMs, bool apiValueAvailable) {
  if (!apiValueAvailable) return true;
  return alternateClockVisible(elapsedMs, clockSeconds, apiValueDisplayMs);
}

uint8_t degreeSymbolSegment(bool staleWarning) {
  static constexpr uint8_t kDegree = 0x63;        // A + B + F + G
  static constexpr uint8_t kBottomSegment = 0x08; // D
  return staleWarning ? static_cast<uint8_t>(kDegree | kBottomSegment) : kDegree;
}

bool apiDataStaleForDisplay(uint32_t ageMs, uint16_t pollSeconds) {
  const uint32_t calculatedStaleMs = static_cast<uint32_t>(pollSeconds) * 2000UL;
  const uint32_t staleAfterMs = calculatedStaleMs > 30000UL ? calculatedStaleMs : 30000UL;
  return ageMs > staleAfterMs;
}

void connectingSegments(uint8_t out[4]) {
  if (!out) return;
  out[0] = 0x39;  // C
  out[1] = 0x5C;  // o
  out[2] = 0x54;  // n
  out[3] = 0x54;  // n
}
