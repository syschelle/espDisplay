#pragma once

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <time.h>

struct NumericValue {
  bool valid = false;
  float value = NAN;
};

// Runtime cache for the external API. espDisplay v0.1.20 intentionally keeps
// only the one measurement the device actually displays: air temperature.
// Metadata needed by the Status page is retained separately.
struct ExternalValues {
  bool valid = false;
  bool lastRequestOk = false;
  bool stale = true;

  uint32_t lastAttemptMs = 0;
  uint32_t lastSuccessMs = 0;
  time_t lastSuccessEpoch = 0;
  int16_t lastHttpStatus = 0;
  char lastError[96] = "";

  char remoteTimezone[48] = "";
  char lastMeasurementAt[40] = "";

  NumericValue temperatureC;
};

enum class DisplayMode : uint8_t {
  Metric = 0,     // Temperature only. Kept as "metric" on the wire for compatibility.
  Clock = 1,
  Alternate = 2
};

struct AppSettings {
  char deviceName[33] = "espDisplay";
  char language[3] = "de";
  char theme[6] = "light";

  char wifiSsid[33] = "";
  char wifiPassword[65] = "";

  char apiHost[65] = "";
  uint16_t apiPort = 80;
  uint16_t apiPollSeconds = 10;

  char ntpServer[65] = "de.pool.ntp.org";
  char timezone[65] = "CET-1CEST,M3.5.0,M10.5.0/3";

  bool displayEnabled = true;
  uint8_t displayBrightness = 5;
  uint8_t displayClkGpio = 14;
  uint8_t displayDioGpio = 12;
  DisplayMode displayMode = DisplayMode::Metric;
  uint32_t displayUpdateMs = 1000UL;
  uint16_t alternateSeconds = 10;
  uint32_t apiValueDisplayMs = 1000UL;
};
