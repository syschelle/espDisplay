#pragma once

#include <Arduino.h>
#include "models.h"

class ExternalApiService {
 public:
  void begin();
  void tick(const AppSettings& settings, bool wifiConnected);
  void forcePoll();
  void clearCache();

  const ExternalValues& values() const { return values_; }
  bool configured(const AppSettings& settings) const { return settings.apiHost[0] != '\0'; }

 private:
  ExternalValues values_;
  bool forcePoll_ = true;

  bool poll(const AppSettings& settings);
  bool parsePayload(const String& payload, ExternalValues& next, char* error, size_t errorLen);
  void markFailure(int httpStatus, const char* message);
  void updateStale(uint16_t pollSeconds);
};

extern ExternalApiService externalApiService;
