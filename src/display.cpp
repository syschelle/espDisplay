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

  // A short all-segments self-test makes wiring/pin problems immediately
  // visible after every boot. It is intentionally bounded to keep boot fast.
  if (cfg.displayEnabled) {
    display_->setBrightness(cfg.displayBrightness, true);
    display_->showNumberDec(8888, true, 4, 0);
    copyText(lastRenderedText_, "8888");
    delay(DISPLAY_BOOT_SELF_TEST_MS);
    yield();
    display_->clear();
  }

  lastUpdateMs_ = 0;
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

  if (lastUpdateMs_ != 0 &&
      static_cast<uint32_t>(millis() - lastUpdateMs_) < cfg.displayUpdateMs) {
    return;
  }
  lastUpdateMs_ = millis();

  switch (cfg.displayMode) {
    case DisplayMode::Clock:
      renderClock();
      break;

    case DisplayMode::Alternate: {
      const uint32_t phaseMs = static_cast<uint32_t>(cfg.alternateSeconds) * 1000UL;
      const bool showClock = phaseMs > 0 && ((millis() / phaseMs) % 2UL) == 1UL;
      if (showClock) {
        renderClock();
      } else {
        renderMetric(cfg.selectedMetric, values);
      }
      break;
    }

    case DisplayMode::Metric:
    default:
      renderMetric(cfg.selectedMetric, values);
      break;
  }
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

void DisplayService::renderClock() {
  if (!display_) return;

  struct tm now;
  if (!timeService.getLocalTm(now)) {
    renderError();
    return;
  }

  const int hhmm = now.tm_hour * 100 + now.tm_min;
  display_->showNumberDecEx(hhmm, TM1637_COLON_MASK, true, 4, 0);

  snprintf(lastRenderedText_, sizeof(lastRenderedText_), "%02d:%02d", now.tm_hour, now.tm_min);
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
