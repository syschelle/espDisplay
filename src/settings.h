#pragma once

#include <Arduino.h>
#include "models.h"

class SettingsManager {
 public:
  bool begin();
  bool load(AppSettings& settings);
  bool save(const AppSettings& settings);
  bool factoryReset();
  bool ready() const { return ready_; }
  const char* lastError() const { return lastError_; }

  static bool validateAndNormalize(AppSettings& settings);
  static bool isValidHost(const char* value);
  static const char* metricToString(MetricId metric);
  static bool metricFromString(const char* value, MetricId& metric);
  static const char* modeToString(DisplayMode mode);
  static bool modeFromString(const char* value, DisplayMode& mode);

 private:
  bool ready_ = false;
  char lastError_[96] = "";
  void setError(const char* value);
};

extern SettingsManager settingsManager;
extern AppSettings settings;
