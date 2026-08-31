#pragma once

#include <Arduino.h>
#include <time.h>
#include <TM1637Display.h>

#include "display_format.h"
#include "models.h"

class DisplayService {
 public:
  DisplayService() = default;

  void begin(const AppSettings& settings);
  void applySettings(const AppSettings& settings);
  void tick(const AppSettings& settings, const ExternalValues& values);

  const char* lastRenderedText() const { return lastRenderedText_; }
  bool lastScaledThousands() const { return lastScaledThousands_; }
  bool initialized() const { return display_ != nullptr; }
  uint8_t clkGpio() const { return clkGpio_; }
  uint8_t dioGpio() const { return dioGpio_; }

 private:
  // Runtime-configurable TM1637 pins require constructing the driver only after
  // persisted settings have been loaded. Placement-new avoids heap allocation.
  alignas(TM1637Display) uint8_t displayStorage_[sizeof(TM1637Display)];
  TM1637Display* display_ = nullptr;
  uint8_t clkGpio_ = 255;
  uint8_t dioGpio_ = 255;
  uint32_t lastUpdateMs_ = 0;
  uint32_t lastClockPollMs_ = 0;
  int8_t lastClockSecond_ = -1;
  bool lastRenderedWasClock_ = false;
  bool lastRenderedWasConnecting_ = false;
  char lastRenderedText_[8] = "----";
  bool lastScaledThousands_ = false;
  bool lastEnabled_ = true;
  uint8_t lastBrightness_ = 255;

  const NumericValue* metricValue(MetricId metric, const ExternalValues& values) const;
  void renderMetric(MetricId metric, const ExternalValues& values);
  void renderClock(const struct tm& now);
  void renderConnecting();
  void renderFrame(const DisplayFrame& frame);
  void renderError();
  void rememberFrame(const DisplayFrame& frame);
};

extern DisplayService displayService;
