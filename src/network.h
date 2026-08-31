#pragma once

#include <Arduino.h>
#include "models.h"

class NetworkService {
 public:
  void begin(const AppSettings& settings);
  void tick(const AppSettings& settings);

  bool connected() const;
  bool apActive() const { return apActive_; }
  int32_t rssi() const;
  String ipAddress() const;
  String apIpAddress() const;
  String hostname() const;

 private:
  bool apActive_ = false;
  bool lastConnected_ = false;
  uint32_t disconnectedSinceMs_ = 0;
  uint32_t lastReconnectAttemptMs_ = 0;
  uint32_t connectedSinceMs_ = 0;

  void startFallbackAp();
  void stopFallbackAp();
  void configureHostname(const char* deviceName);
};

extern NetworkService networkService;
