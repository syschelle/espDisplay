#pragma once

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Hardware
// -----------------------------------------------------------------------------
// Default target: Wemos/Lolin D1 mini compatible ESP8266 board.
// TM1637 pins are runtime settings and can be changed in the web interface.
// The defaults remain D5/GPIO14 and D6/GPIO12 for migration compatibility.
static constexpr uint8_t DEFAULT_TM1637_CLK_GPIO = 14;  // D5
static constexpr uint8_t DEFAULT_TM1637_DIO_GPIO = 12;  // D6

// -----------------------------------------------------------------------------
// Project identity / OTA channel
// -----------------------------------------------------------------------------
static constexpr char PROJECT_NAME[] = "espDisplay";
static constexpr char OTA_GITHUB_OWNER[] = "syschelle";
static constexpr char OTA_GITHUB_REPO[] = "espDisplay";
static constexpr char OTA_ASSET_NAME[] = "firmware.bin";
static constexpr char OTA_BRANCH[] = "ota";
static constexpr char OTA_MANIFEST_NAME[] = "manifest.json";

// -----------------------------------------------------------------------------
// Network defaults
// -----------------------------------------------------------------------------
static constexpr char DEFAULT_DEVICE_NAME[] = "espDisplay";
static constexpr char LEGACY_DEFAULT_DEVICE_NAME[] = "EnergyDisplay8266";
static constexpr char FALLBACK_AP_SSID[] = "espDisplay-Setup";
static constexpr char FALLBACK_AP_PASSWORD[] = "ED-Setup-8266";

static constexpr uint32_t WIFI_BOOT_CONNECT_TIMEOUT_MS = 10000UL;
static constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 15000UL;
static constexpr uint32_t WIFI_FALLBACK_AP_AFTER_MS = 30000UL;
static constexpr uint32_t WIFI_AP_STOP_DELAY_MS = 10000UL;

// -----------------------------------------------------------------------------
// External API
// -----------------------------------------------------------------------------
static constexpr char EXTERNAL_API_PATH[] = "/api/current-values";
static constexpr uint16_t DEFAULT_API_PORT = 80;
static constexpr uint16_t DEFAULT_API_POLL_SECONDS = 10;
static constexpr uint16_t MIN_API_POLL_SECONDS = 5;
static constexpr uint16_t MAX_API_POLL_SECONDS = 300;
static constexpr uint32_t API_CONNECT_TIMEOUT_MS = 1200UL;
static constexpr uint32_t API_RESPONSE_TIMEOUT_MS = 60000UL;
static constexpr uint32_t API_INACTIVITY_TIMEOUT_MS = 60000UL;
static constexpr size_t API_READ_BUDGET_BYTES = 384;
static constexpr size_t MAX_EXTERNAL_API_BYTES = 8192;

// -----------------------------------------------------------------------------
// NTP / timezone
// -----------------------------------------------------------------------------
static constexpr char DEFAULT_NTP_SERVER[] = "de.pool.ntp.org";
// POSIX TZ for Europe/Berlin including CET/CEST transitions.
static constexpr char DEFAULT_TIMEZONE[] = "CET-1CEST,M3.5.0,M10.5.0/3";
static constexpr uint32_t NTP_BOOT_SYNC_TIMEOUT_MS = 10000UL;
static constexpr uint32_t NTP_RETRY_INTERVAL_MS = 300000UL;
static constexpr uint32_t NTP_RESYNC_INTERVAL_MS = 21600000UL;  // 6 hours

// -----------------------------------------------------------------------------
// Display
// -----------------------------------------------------------------------------
static constexpr uint8_t DEFAULT_DISPLAY_BRIGHTNESS = 5;
static constexpr uint16_t DEFAULT_DISPLAY_UPDATE_MS = 1000;
static constexpr uint16_t MIN_DISPLAY_UPDATE_MS = 250;
static constexpr uint16_t MAX_DISPLAY_UPDATE_MS = 10000;
static constexpr uint16_t DEFAULT_ALTERNATE_SECONDS = 10;
static constexpr uint16_t MIN_ALTERNATE_SECONDS = 2;
static constexpr uint16_t MAX_ALTERNATE_SECONDS = 120;

// On common four-digit TM1637 clock modules, segment DP on digit 2 drives
// the center colon when raw segments are written with setSegments().
static constexpr uint8_t TM1637_CENTER_COLON_SEGMENT = 0x80;

// ESP8266 GPIO6..GPIO11 are connected to the SPI flash and must never be used.
// GPIO0, GPIO2 and GPIO15 are boot strap pins; GPIO1/GPIO3 are UART pins. They
// remain selectable in the UI with warnings because existing installations can
// legitimately use them, but recommended choices are GPIO4/5/12/13/14.
static constexpr bool isSelectableDisplayGpio(uint8_t gpio) {
  return gpio <= 16 && !(gpio >= 6 && gpio <= 11);
}

static constexpr bool isRecommendedDisplayGpio(uint8_t gpio) {
  return gpio == 4 || gpio == 5 || gpio == 12 || gpio == 13 || gpio == 14;
}

// -----------------------------------------------------------------------------
// Persistence / logging
// -----------------------------------------------------------------------------
static constexpr size_t EEPROM_SETTINGS_BYTES = 512;
static constexpr uint32_t EEPROM_SETTINGS_MAGIC = 0x45443832UL;  // "ED82"
static constexpr uint16_t LEGACY_SETTINGS_SCHEMA_VERSION = 1;
static constexpr uint16_t PREVIOUS_SETTINGS_SCHEMA_VERSION = 2;
static constexpr uint16_t SETTINGS_SCHEMA_VERSION = 3;

static constexpr size_t LOG_LINE_COUNT = 30;
static constexpr size_t LOG_LINE_LENGTH = 128;

// Web UI state polling is deliberately slower than the display refresh.
static constexpr uint32_t WEB_STATE_POLL_MS = 5000UL;
