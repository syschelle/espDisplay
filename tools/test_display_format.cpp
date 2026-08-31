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

static void expect(MetricId metric, float value, const char* chars, int decimalAfter, bool scaled) {
  DisplayFrame out;
  assert(formatMetricFrame(metric, numeric(value), out));
  assert(out.valid);
  assert(strcmp(out.chars, chars) == 0);
  assert(out.decimalAfter == decimalAfter);
  assert(out.scaledThousands == scaled);
}

int main() {
  expect(MetricId::GridImportW, 614.61f, "615", -1, false);
  expect(MetricId::GridImportW, 1250.0f, "1250", -1, false);
  expect(MetricId::GridImportW, 9999.0f, "9999", -1, false);
  expect(MetricId::GridImportW, 12500.0f, "125", 1, true);   // 12.5 kW
  expect(MetricId::GridW, -614.0f, "-614", -1, false);
  expect(MetricId::GridW, -1250.0f, "-125", 1, true);       // -1.25 kW

  expect(MetricId::DailyGridImportKwh, 6.3176f, "632", 0, false); // 6.32 after rounding
  expect(MetricId::DailyGridImportKwh, 12.5f, "125", 1, false);  // 12.5
  expect(MetricId::TotalGridImportKwh, 123.4f, "1234", 2, false); // 123.4
  expect(MetricId::TotalGridImportKwh, 12345.0f, "1235", 1, true); // 12.35 x1000

  expect(MetricId::AirTemperatureC, 22.85f, "229", 1, false); // 22.9
  expect(MetricId::AirHumidityPercent, 76.88f, "769", 1, false);

  DisplayFrame invalid;
  NumericValue missing;
  assert(!formatMetricFrame(MetricId::GridImportW, missing, invalid));
  assert(!invalid.valid);


  // Clock colon: visible on even seconds, hidden on odd seconds.
  assert(clockColonVisible(0));
  assert(!clockColonVisible(1));
  assert(clockColonVisible(58));
  assert(!clockColonVisible(59));

  // Clock digits are right-aligned. Single-digit hours use only one hour
  // position, while minutes always keep both digits.
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

  puts("display_format tests passed");
  return 0;
}
