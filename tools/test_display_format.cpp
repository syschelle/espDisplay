#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "display_format.h"

static NumericValue numeric(float value) {
  NumericValue n;
  n.valid = true;
  n.value = value;
  return n;
}

static void expectTemperature(float value, const char* chars) {
  DisplayFrame out;
  assert(formatTemperatureFrame(numeric(value), out));
  assert(out.valid);
  assert(strcmp(out.chars, chars) == 0);
  assert(out.decimalAfter == -1);
  assert(out.degreeSuffix);
}

int main() {
  expectTemperature(22.49f, "22");
  expectTemperature(22.50f, "23");
  expectTemperature(-4.6f, "-5");

  DisplayFrame invalid;
  NumericValue missing;
  assert(!formatTemperatureFrame(missing, invalid));
  assert(!invalid.valid);

  // One digit is reserved for the degree symbol, so signed temperatures need
  // to fit in the remaining three character positions.
  assert(!formatTemperatureFrame(numeric(-100.0f), invalid));
  assert(!formatTemperatureFrame(numeric(1000.0f), invalid));

  assert(alternateClockVisible(0, 10, 500));
  assert(alternateClockVisible(9999, 10, 500));
  assert(!alternateClockVisible(10000, 10, 500));
  assert(!alternateClockVisible(10499, 10, 500));
  assert(alternateClockVisible(10500, 10, 500));
  assert(!alternateClockVisible(10000, 10, 1250));
  assert(!alternateClockVisible(11249, 10, 1250));
  assert(alternateClockVisible(11250, 10, 1250));

  // After reboot, alternate mode stays on the clock until temperature exists.
  assert(alternateDisplayShowsClock(10000, 10, 500, false));
  assert(alternateDisplayShowsClock(10499, 10, 500, false));
  assert(!alternateDisplayShowsClock(10000, 10, 500, true));
  assert(alternateDisplayShowsClock(10500, 10, 500, true));

  assert(degreeSymbolSegment() == 0x63);
  assert(degreeSymbolSegment(false) == 0x63);
  assert(degreeSymbolSegment(true) == 0x6B);

  assert(!apiDataStaleForDisplay(30000, 10));
  assert(apiDataStaleForDisplay(30001, 10));
  assert(!apiDataStaleForDisplay(40000, 20));
  assert(apiDataStaleForDisplay(40001, 20));
  assert(!apiDataStaleForDisplay(600000, 300));
  assert(apiDataStaleForDisplay(600001, 300));

  assert(clockColonVisible(0));
  assert(!clockColonVisible(1));
  assert(clockColonVisible(58));
  assert(!clockColonVisible(59));

  ClockFrame clock;
  assert(formatClockFrame(9, 5, 0, clock));
  assert(strcmp(clock.chars, " 905") == 0);
  assert(clock.colonVisible);

  assert(formatClockFrame(9, 5, 1, clock));
  assert(strcmp(clock.chars, " 905") == 0);
  assert(!clock.colonVisible);

  assert(formatClockFrame(0, 5, 2, clock));
  assert(strcmp(clock.chars, " 005") == 0);

  assert(formatClockFrame(12, 34, 3, clock));
  assert(strcmp(clock.chars, "1234") == 0);

  assert(!formatClockFrame(24, 0, 0, clock));
  assert(!formatClockFrame(12, 60, 0, clock));
  assert(!formatClockFrame(12, 0, 60, clock));

  uint8_t conn[4] = {0, 0, 0, 0};
  connectingSegments(conn);
  assert(conn[0] == 0x39);
  assert(conn[1] == 0x5C);
  assert(conn[2] == 0x54);
  assert(conn[3] == 0x54);

  puts("display_format tests passed");
  return 0;
}
