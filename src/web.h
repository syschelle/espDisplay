#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

class WebService {
 public:
  WebService();

  void begin();
  void tick();
  void scheduleRestart(uint32_t delayMs = 1500UL);

 private:
  ESP8266WebServer server_;
  bool restartScheduled_ = false;
  uint32_t restartAtMs_ = 0;

  void registerRoutes();
  void sendJson(int status, const String& json);
  void sendProgmemAsset(const char* contentType, PGM_P content, size_t contentLength);
  void sendError(int status, const char* message);
  bool parseBody(JsonDocument& doc, size_t maxBytes = 2048);

  void handleSettingsGet();
  void handleStateGet();
  void handleLogGet();
  void handleDisplaySettingsPost();
  void handleSystemSettingsPost();
  void handleLogClearPost();
  void handleOtaCheck();
  void handleOtaUpdate();
  void handleFactoryReset();
};

extern WebService webService;
