#pragma once

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <time.h>

struct NumericValue {
  bool valid = false;
  float value = NAN;
};

struct AirSensorValues {
  bool present = false;
  bool enabled = false;
  bool configured = false;
  bool ok = false;
  bool cached = false;

  NumericValue temperatureC;
  NumericValue humidityPercent;
  NumericValue dewPointC;
  NumericValue pressureHpa;
  NumericValue pressureSeaLevelHpa;
  NumericValue pm10;
  NumericValue pm25;

  int32_t ageSeconds = -1;

  char softwareVersion[40] = "";
  char lastSuccessAt[40] = "";
  char lastError[96] = "";

  bool weatherUndergroundLastOk = false;
  int16_t weatherUndergroundLastStatus = 0;
  char weatherUndergroundLastResponse[64] = "";
  char weatherUndergroundLastAt[40] = "";
};

struct ExternalValues {
  bool valid = false;
  bool lastRequestOk = false;
  bool stale = true;

  uint32_t lastAttemptMs = 0;
  uint32_t lastSuccessMs = 0;
  time_t lastSuccessEpoch = 0;
  int16_t lastHttpStatus = 0;
  char lastError[96] = "";

  char timestampUtc[40] = "";
  char localDate[16] = "";
  char remoteTimezone[48] = "";
  char lastMeasurementAt[40] = "";

  NumericValue currentSolarProductionW;
  NumericValue currentGridPowerW;
  NumericValue currentGridImportW;
  NumericValue currentGridExportW;
  NumericValue currentTotalConsumptionW;

  NumericValue dailySolarProductionKwh;
  NumericValue dailyGridImportKwh;
  NumericValue dailyGridExportKwh;

  NumericValue totalSolarProductionKwh;
  NumericValue totalGridImportKwh;
  NumericValue totalGridExportKwh;

  AirSensorValues air;
};

enum class DisplayMode : uint8_t {
  Metric = 0,
  Clock = 1,
  Alternate = 2
};

enum class MetricId : uint8_t {
  SolarW = 0,
  GridW,
  GridImportW,
  GridExportW,
  ConsumptionW,
  DailySolarKwh,
  DailyGridImportKwh,
  DailyGridExportKwh,
  TotalSolarKwh,
  TotalGridImportKwh,
  TotalGridExportKwh,
  AirTemperatureC,
  AirHumidityPercent,
  AirDewPointC,
  AirPm10,
  AirPm25
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
  MetricId selectedMetric = MetricId::GridImportW;
  DisplayMode displayMode = DisplayMode::Metric;
  uint32_t displayUpdateMs = 1000UL;
  uint16_t alternateSeconds = 10;
  uint32_t apiValueDisplayMs = 1000UL;
};
