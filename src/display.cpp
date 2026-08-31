#include "display.h"

#include <new>
#include <string.h>
#include <time.h>

#include "config.h"
#include "logging.h"
#include "text_utils.h"
#include "time_service.h"

DisplayService displayService;

void DisplayService::begin(const AppSettings& cfg) {
  clkGpio_ = cfg.displayClkGpio;
  dioGpio_ = cfg.displayDioGpio;
  display_ = new (displayStorage_) TM1637Display(clkGpio_, dioGpio_);

  char message[80];
  snprintf(message, sizeof(message), "TM1637 initialized: CLK GPIO%u, DIO GPIO%u",
           clkGpio_, dioGpio_);
  appLog.info("DISPLAY", message);

  display_->clear();
  applySettings(cfg);

  // Show exactly one deterministic boot state while the clock is invalid.
  // No segment self-test or placeholder value is displayed before NTP time.
  if (cfg.displayEnabled) {
    display_->setBrightness(cfg.displayBrightness, true);
    renderConnecting();
  }

  lastUpdateMs_ = 0;
  lastClockPollMs_ = 0;
  lastClockSecond_ = -1;
  lastRenderedWasClock_ = false;
}

void DisplayService::applySettings(const AppSettings& cfg) {
  if (!display_) return;

  if (lastBrightness_ != cfg.displayBrightness || lastEnabled_ != cfg.displayEnabled) {
    display_->setBrightness(cfg.displayBrightness, cfg.displayEnabled);
    lastBrightness_ = cfg.displayBrightness;
    lastEnabled_ = cfg.displayEnabled;
  }

  if (!cfg.displayEnabled) {
    display_->clear();
    copyText(lastRenderedText_, "OFF");
  }

  lastUpdateMs_ = 0;
  lastClockPollMs_ = 0;
  lastClockSecond_ = -1;
  lastRenderedWasClock_ = false;
  lastRenderedWasConnecting_ = false;
}

void DisplayService::tick(const AppSettings& cfg, const ExternalValues& values) {
  if (!display_) return;

  if (!cfg.displayEnabled) {
    if (lastEnabled_) applySettings(cfg);
    return;
  }

  if (lastBrightness_ != cfg.displayBrightness || !lastEnabled_) {
    applySettings(cfg);
  }

  // Until a valid local clock exists, the display must show only "Conn".
  // This intentionally overrides metric and clock modes during startup.
  struct tm validTimeProbe;
  if (!timeService.getLocalTm(validTimeProbe)) {
    if (!lastRenderedWasConnecting_) {
      renderConnecting();
    }
    lastRenderedWasClock_ = false;
    lastClockSecond_ = -1;
    return;
  }
  lastRenderedWasConnecting_ = false;

  bool showClock = cfg.displayMode == DisplayMode::Clock;
  if (cfg.displayMode == DisplayMode::Alternate) {
    const uint32_t phaseMs = static_cast<uint32_t>(cfg.alternateSeconds) * 1000UL;
    showClock = phaseMs > 0 && ((millis() / phaseMs) % 2UL) == 1UL;
  }

  // Clock rendering is deliberately independent from displayUpdateMs. A user may
  // choose a slow metric refresh (for example 4000 ms), but the center colon
  // still has to toggle exactly once per second while the clock is visible.
  if (showClock) {
    const uint32_t nowMs = millis();
    if (lastClockPollMs_ != 0 &&
        static_cast<uint32_t>(nowMs - lastClockPollMs_) < 200UL) {
      return;
    }
    lastClockPollMs_ = nowMs;

    struct tm now;
    if (!timeService.getLocalTm(now)) {
      if (lastUpdateMs_ == 0 ||
          static_cast<uint32_t>(nowMs - lastUpdateMs_) >= 1000UL) {
        renderError();
        lastUpdateMs_ = nowMs;
      }
      lastRenderedWasClock_ = false;
      lastClockSecond_ = -1;
      return;
    }

    if (lastRenderedWasClock_ && lastClockSecond_ == now.tm_sec) {
      return;
    }

    renderClock(now);
    lastClockSecond_ = static_cast<int8_t>(now.tm_sec);
    lastRenderedWasClock_ = true;
    lastUpdateMs_ = nowMs;
    return;
  }

  if (lastRenderedWasClock_) {
    // Change back to the metric immediately when an alternate clock phase ends.
    lastUpdateMs_ = 0;
  }
  lastRenderedWasClock_ = false;
  lastClockSecond_ = -1;
  lastClockPollMs_ = 0;

  if (lastUpdateMs_ != 0 &&
      static_cast<uint32_t>(millis() - lastUpdateMs_) < cfg.displayUpdateMs) {
    return;
  }
  lastUpdateMs_ = millis();
  renderMetric(cfg.selectedMetric, values);
}

const NumericValue* DisplayService::metricValue(
    MetricId metric,
    const ExternalValues& values) const {
  switch (metric) {
    case MetricId::SolarW: return &values.currentSolarProductionW;
    case MetricId::GridW: return &values.currentGridPowerW;
    case MetricId::GridImportW: return &values.currentGridImportW;
    case MetricId::GridExportW: return &values.currentGridExportW;
    case MetricId::ConsumptionW: return &values.currentTotalConsumptionW;
    case MetricId::DailySolarKwh: return &values.dailySolarProductionKwh;
    case MetricId::DailyGridImportKwh: return &values.dailyGridImportKwh;
    case MetricId::DailyGridExportKwh: return &values.dailyGridExportKwh;
    case MetricId::TotalSolarKwh: return &values.totalSolarProductionKwh;
    case MetricId::TotalGridImportKwh: return &values.totalGridImportKwh;
    case MetricId::TotalGridExportKwh: return &values.totalGridExportKwh;
    case MetricId::AirTemperatureC: return &values.air.temperatureC;
    case MetricId::AirHumidityPercent: return &values.air.humidityPercent;
    case MetricId::AirDewPointC: return &values.air.dewPointC;
    case MetricId::AirPm10: return &values.air.pm10;
    case MetricId::AirPm25: return &values.air.pm25;
    default: return nullptr;
  }
}

void DisplayService::renderMetric(MetricId metric, const ExternalValues& values) {
  const NumericValue* value = metricValue(metric, values);
  if (!value) {
    renderError();
    return;
  }

  DisplayFrame frame;
  if (!formatMetricFrame(metric, *value, frame)) {
    renderError();
    return;
  }

  renderFrame(frame);
  rememberFrame(frame);
}

void DisplayService::renderClock(const struct tm& now) {
  if (!display_) return;

  ClockFrame frame;
  if (!formatClockFrame(static_cast<uint8_t>(now.tm_hour),
                        static_cast<uint8_t>(now.tm_min),
                        static_cast<uint8_t>(now.tm_sec), frame)) {
    renderError();
    return;
  }

  uint8_t segments[4] = {0, 0, 0, 0};
  for (uint8_t i = 0; i < 4; ++i) {
    const char c = frame.chars[i];
    if (c >= '0' && c <= '9') {
      segments[i] = display_->encodeDigit(static_cast<uint8_t>(c - '0'));
    }
  }

  // On common four-digit TM1637 clock modules the decimal-point segment of
  // digit 2 drives the center colon. Keep it blinking once per second.
  if (frame.colonVisible) {
    segments[1] |= TM1637_CENTER_COLON_SEGMENT;
  }
  display_->setSegments(segments);

  // The web status mirrors the no-leading-zero clock representation.
  snprintf(lastRenderedText_, sizeof(lastRenderedText_), "%d:%02d", now.tm_hour, now.tm_min);
  lastScaledThousands_ = false;
}

void DisplayService::renderFrame(const DisplayFrame& frame) {
  if (!display_) return;

  uint8_t segments[4] = {0, 0, 0, 0};
  const size_t len = strlen(frame.chars);
  const uint8_t offset = len < 4 ? static_cast<uint8_t>(4 - len) : 0;

  for (size_t i = 0; i < len && i < 4; ++i) {
    const char c = frame.chars[i];
    uint8_t encoded = 0;

    if (c >= '0' && c <= '9') {
      encoded = display_->encodeDigit(c - '0');
    } else if (c == '-') {
      encoded = 0x40;
    }

    const uint8_t displayIndex = offset + static_cast<uint8_t>(i);
    if (frame.decimalAfter == static_cast<int8_t>(i)) {
      encoded |= 0x80;
    }
    segments[displayIndex] = encoded;
  }

  display_->setSegments(segments);
}

void DisplayService::renderConnecting() {
  if (!display_) return;

  uint8_t segments[4] = {0, 0, 0, 0};
  connectingSegments(segments);
  display_->setSegments(segments);
  copyText(lastRenderedText_, "Conn");
  lastScaledThousands_ = false;
  lastRenderedWasConnecting_ = true;
}

void DisplayService::renderError() {
  if (!display_) return;
  const uint8_t dash[4] = {0x40, 0x40, 0x40, 0x40};
  display_->setSegments(dash);
  copyText(lastRenderedText_, "----");
  lastScaledThousands_ = false;
}

void DisplayService::rememberFrame(const DisplayFrame& frame) {
  size_t out = 0;
  for (size_t i = 0; frame.chars[i] != '\0' && out < sizeof(lastRenderedText_) - 1; ++i) {
    lastRenderedText_[out++] = frame.chars[i];
    if (frame.decimalAfter == static_cast<int8_t>(i) &&
        out < sizeof(lastRenderedText_) - 1) {
      lastRenderedText_[out++] = '.';
    }
  }
  lastRenderedText_[out] = '\0';
  lastScaledThousands_ = frame.scaledThousands;
}
