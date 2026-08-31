#include <Arduino.h>

#include "display.h"
#include "external_api.h"
#include "logging.h"
#include "network.h"
#include "ota.h"
#include "settings.h"
#include "time_service.h"
#include "version.h"
#include "web.h"

void setup() {
  Serial.begin(115200);
  delay(50);

  appLog.begin();
  char bootMessage[64];
  snprintf(bootMessage, sizeof(bootMessage), "Firmware %s", FW_VERSION);
  appLog.info("BOOT", bootMessage);

  settingsManager.begin();
  settingsManager.load(settings);

  // Initialize the physical display before any potentially slow network work.
  // While local time is still invalid, the display shows only "Conn".
  displayService.begin(settings);

  // Required boot order for time acquisition: establish Wi-Fi first, then make
  // a bounded NTP synchronization attempt before normal polling begins.
  networkService.begin(settings);
  timeService.begin();
  timeService.bootSynchronize(settings, networkService.connected());

  externalApiService.begin();
  otaService.begin();
  webService.begin();

  appLog.info("BOOT", "Startup complete");
}

void loop() {
  networkService.tick(settings);
  timeService.tick(settings, networkService.connected());
  externalApiService.tick(settings, networkService.connected());
  displayService.tick(settings, externalApiService.values());
  webService.tick();

  // Keep the ESP8266 watchdog and Wi-Fi stack serviced.
  yield();
  delay(1);
}
