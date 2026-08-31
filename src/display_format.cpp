#include "display_format.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "text_utils.h"

namespace {

bool buildDecimalFrame(float value, uint8_t maxDecimals, DisplayFrame& out) {
  if (!isfinite(value)) return false;

  for (int decimals = static_cast<int>(maxDecimals); decimals >= 0; --decimals) {
    char formatted[24];
    snprintf(formatted, sizeof(formatted), "%.*f", decimals, value);

    // Remove insignificant trailing zeros and a trailing decimal separator.
    if (strchr(formatted, '.') != nullptr) {
      size_t len = strlen(formatted);
      while (len > 0 && formatted[len - 1] == '0') {
        formatted[--len] = '\0';
      }
      if (len > 0 && formatted[len - 1] == '.') {
        formatted[--len] = '\0';
      }
    }

    char compact[5] = "";
    int8_t decimalAfter = -1;
    uint8_t compactLen = 0;

    for (size_t i = 0; formatted[i] != '\0'; ++i) {
      if (formatted[i] == '.') {
        if (compactLen == 0) return false;
        decimalAfter = static_cast<int8_t>(compactLen - 1);
        continue;
      }

      if (compactLen >= 4) {
        compactLen = 5;
        break;
      }

      compact[compactLen++] = formatted[i];
    }

    if (compactLen <= 4 && compactLen > 0) {
      compact[compactLen] = '\0';
      copyText(out.chars, compact);
      out.decimalAfter = decimalAfter;
      out.valid = true;
      return true;
    }
  }

  return false;
}

bool buildIntegerFrame(float value, DisplayFrame& out) {
  if (!isfinite(value)) return false;
  const long rounded = lroundf(value);

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%ld", rounded);
  if (strlen(buffer) > 4) return false;

  copyText(out.chars, buffer);
  out.decimalAfter = -1;
  out.valid = true;
  return true;
}

}  // namespace

bool formatMetricFrame(MetricId metric, const NumericValue& input, DisplayFrame& out) {
  out = DisplayFrame();
  if (!input.valid || !isfinite(input.value)) return false;

  const float value = input.value;

  switch (metric) {
    case MetricId::SolarW:
    case MetricId::GridW:
    case MetricId::GridImportW:
    case MetricId::GridExportW:
    case MetricId::ConsumptionW:
      if (buildIntegerFrame(value, out)) {
        return true;
      }
      // When watts no longer fit, display kW implicitly, e.g. 12500 W -> 12.5.
      out = DisplayFrame();
      if (buildDecimalFrame(value / 1000.0f, 2, out)) {
        out.scaledThousands = true;
        return true;
      }
      return false;

    case MetricId::DailySolarKwh:
    case MetricId::DailyGridImportKwh:
    case MetricId::DailyGridExportKwh:
    case MetricId::TotalSolarKwh:
    case MetricId::TotalGridImportKwh:
    case MetricId::TotalGridExportKwh:
      if (buildDecimalFrame(value, 2, out)) {
        return true;
      }
      // Large accumulated kWh values are displayed in thousands (MWh-equivalent).
      out = DisplayFrame();
      if (buildDecimalFrame(value / 1000.0f, 2, out)) {
        out.scaledThousands = true;
        return true;
      }
      return false;

    case MetricId::AirTemperatureC:
    case MetricId::AirHumidityPercent:
    case MetricId::AirDewPointC:
      return buildDecimalFrame(value, 1, out);

    case MetricId::AirPm10:
    case MetricId::AirPm25:
      return buildDecimalFrame(value, 2, out);

    default:
      return false;
  }
}

bool formatClockFrame(uint8_t hour, uint8_t minute, uint8_t second, ClockFrame& out) {
  out = ClockFrame();
  if (hour > 23U || minute > 59U || second > 59U) return false;

  if (hour < 10U) {
    snprintf(out.chars, sizeof(out.chars), " %u%02u",
             static_cast<unsigned>(hour), static_cast<unsigned>(minute));
  } else {
    snprintf(out.chars, sizeof(out.chars), "%02u%02u",
             static_cast<unsigned>(hour), static_cast<unsigned>(minute));
  }
  out.colonVisible = clockColonVisible(second);
  return true;
}

bool clockColonVisible(uint8_t second) {
  return (second % 2U) == 0U;
}
