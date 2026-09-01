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

bool buildRoundedTemperatureFrame(float value, DisplayFrame& out) {
  if (!isfinite(value)) return false;

  const long rounded = lroundf(value);
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%ld", rounded);

  // One of the four TM1637 digits is reserved for the degree symbol.
  // This leaves up to three characters for the signed integer temperature.
  if (strlen(buffer) > 3) return false;

  copyText(out.chars, buffer);
  out.decimalAfter = -1;
  out.valid = true;
  out.degreeSuffix = true;
  return true;
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
      return buildRoundedTemperatureFrame(value, out);

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

  // Build the four physical clock positions explicitly. This keeps the
  // representation warning-free and guarantees a terminating NUL byte.
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
  // Small raised circle: segments A + B + F + G. When cached API data has
  // become genuinely stale, also light segment D as an underline in the same
  // physical digit. This keeps the numeric temperature itself unchanged.
  static constexpr uint8_t kDegree = 0x63;
  static constexpr uint8_t kBottomSegment = 0x08;
  return staleWarning ? static_cast<uint8_t>(kDegree | kBottomSegment) : kDegree;
}

bool apiDataStaleForDisplay(uint32_t ageMs, uint16_t pollSeconds) {
  const uint32_t calculatedStaleMs = static_cast<uint32_t>(pollSeconds) * 2000UL;
  const uint32_t staleAfterMs = calculatedStaleMs > 30000UL ? calculatedStaleMs : 30000UL;
  return ageMs > staleAfterMs;
}

void connectingSegments(uint8_t out[4]) {
  if (!out) return;

  // Seven-segment approximations: C, o, n, n.
  // Segment bits use the TM1637 convention A..G = bit 0..6.
  out[0] = 0x39;  // C: A + D + E + F
  out[1] = 0x5C;  // o: C + D + E + G
  out[2] = 0x54;  // n: C + E + G
  out[3] = 0x54;  // n
}
