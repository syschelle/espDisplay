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
