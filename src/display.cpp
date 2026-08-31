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
  alternateCycleStartedMs_ = 0;
  alternateCycleActive_ = false;
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
  alternateCycleStartedMs_ = 0;
  alternateCycleActive_ = false;
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
    alternateCycleActive_ = false;
    return;
  }
  lastRenderedWasConnecting_ = false;

  const uint32_t nowMs = millis();
  bool showClock = cfg.displayMode == DisplayMode::Clock;
  if (cfg.displayMode == DisplayMode::Alternate) {
    // Start every alternate cycle with a complete clock phase. The configured
    // interval now controls only how long the clock stays visible; the metric
    // is always shown for exactly one second between clock phases.
    if (!alternateCycleActive_) {
      alternateCycleStartedMs_ = nowMs;
      alternateCycleActive_ = true;
    }
    showClock = alternateClockVisible(
        static_cast<uint32_t>(nowMs - alternateCycleStartedMs_),
        cfg.alternateSeconds);
  } else {
    alternateCycleActive_ = false;
  }

  // Clock rendering is deliberately independent from displayUpdateMs. A user may
  // choose a slow metric refresh (for example 4000 ms), but the center colon
  // still has to toggle exactly once per second while the clock is visible.
  if (showClock) {
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
  const size_t suffixWidth = frame.degreeSuffix ? 1U : 0U;
  const size_t totalWidth = len + suffixWidth;
  const uint8_t offset = totalWidth < 4U ? static_cast<uint8_t>(4U - totalWidth) : 0U;

  for (size_t i = 0; i < len && (offset + i) < 4U; ++i) {
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

  if (frame.degreeSuffix && len < 4U && (offset + len) < 4U) {
    segments[offset + len] = degreeSymbolSegment();
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
  if (frame.degreeSuffix && out + 2U < sizeof(lastRenderedText_)) {
    // UTF-8 degree sign for the web/API mirror of the physical display.
    lastRenderedText_[out++] = static_cast<char>(0xC2);
    lastRenderedText_[out++] = static_cast<char>(0xB0);
  }
  lastRenderedText_[out] = '\0';
  lastScaledThousands_ = frame.scaledThousands;
}
