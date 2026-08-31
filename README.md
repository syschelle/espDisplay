# espDisplay

espDisplay is a small, independent ESP8266 network display. It periodically reads energy and optional air-quality values from an external HTTP endpoint, keeps the latest valid values in RAM, shows a selected value on a four-digit TM1637 display, and exposes a responsive local web interface.

This repository is **not** a GrowTent firmware variant. It has its own PlatformIO environment, version series, OTA channel and release workflow. GrowTent was used only as a design and architecture reference for the web UI, Wi-Fi fallback concept, NTP/OTA concepts and code organization.

## Main features

- ESP8266 / Arduino Framework / PlatformIO
- TM1637 four-digit display
- External API polling at `http://<host>/api/current-values`
- Last-known-good RAM cache with stale/error state
- No sensor, relay, pump, Shelly, VPD, irrigation, tank or HomeKit logic
- Responsive GrowTent-inspired web UI with light/dark theme
- German and English UI with centralized browser-side i18n
- Wi-Fi station mode with fallback access point
- NTP with POSIX timezone support and bounded boot synchronization
- Compact EEPROM-backed settings persistence with magic/schema/CRC validation
- Read-only local `/api/state` for the web UI
- GitHub Release based OTA update channel dedicated to this project
- Factory reset that deletes configuration and reboots
- Compact in-memory system log

## Hardware

Default PlatformIO target:

- Wemos/Lolin D1 mini compatible ESP8266 (`d1_mini`)
- TM1637 4-digit display

### Configurable TM1637 pins

Starting with v0.1.1, CLK and DIO are **persistent numeric GPIO settings** in the web interface. They are no longer fixed firmware constants. Open **Display** in the web interface, enter the numeric ESP8266 GPIO numbers for CLK and DIO, then save. A pin change schedules an automatic restart because the TM1637 driver is constructed with its pins at boot.

Factory defaults and migrated v0.1.0 settings use:

| TM1637 | D1 mini | GPIO |
|---|---:|---:|
| CLK | D5 | 14 |
| DIO | D6 | 12 |
| GND | GND | - |
| VCC | according to the module | - |

Recommended general-purpose choices are D1/GPIO5, D2/GPIO4, D5/GPIO14, D6/GPIO12 and D7/GPIO13. D3/GPIO0, D4/GPIO2 and D8/GPIO15 are ESP8266 boot-strap pins; RX/GPIO3 and TX/GPIO1 share the serial interface. They are available in the UI with a warning for existing installations, but should be used deliberately. GPIO6 through GPIO11 are connected to the ESP8266 flash and are not selectable.

At boot, an enabled display shows `8888` briefly as a wiring/pin self-test. If `8888` is not visible, verify power, CLK/DIO order and the configured GPIOs.

**Electrical note:** ESP8266 GPIOs are 3.3 V only. Some TM1637 breakout boards contain pull-ups to their supply voltage. Verify the specific module before powering it from 5 V; do not expose ESP8266 GPIOs to 5 V. Use a 3.3 V-compatible module or suitable level shifting when required.

## Build

Requirements:

- VS Code + PlatformIO IDE, or PlatformIO Core
- USB cable suitable for the target board

```bash
pio run -e d1_mini
```

Dependencies are pinned in `platformio.ini`:

- PlatformIO Espressif8266 platform 4.2.1
- ArduinoJson 7.4.3
- avishorp/TM1637 1.2.0

HTTP, Wi-Fi, web server, EEPROM emulation, NTP and ESP8266 update support come from the ESP8266 Arduino framework and are not added as redundant libraries.

## USB flash

```bash
pio run -e d1_mini -t upload
pio device monitor -b 115200
```

## First boot and Wi-Fi

1. Stored Wi-Fi credentials are loaded from the ESP8266 EEPROM-backed settings record.
2. If credentials exist, the ESP8266 attempts station-mode connection for a bounded period.
3. If no credentials exist or the station connection is unavailable, the fallback AP starts.
4. The web interface remains available while reconnect attempts continue at a low rate.
5. Once station Wi-Fi is stable, the fallback AP is stopped.

Default fallback AP:

- SSID: `espDisplay-Setup`
- Password: `ED-Setup-8266`

Change the default AP password in `src/config.h` before public deployment if required.

The stored Wi-Fi password is never returned by the settings API and is never written to the log. A blank password field in the web UI preserves the currently stored password.

## External API

Configure only the host/IP in the web UI, for example:

```text
192.168.178.50
```

The firmware constructs exactly:

```text
http://192.168.178.50/api/current-values
```

Schemes, ports embedded in arbitrary URLs, paths and redirects supplied by API data are not accepted. The endpoint path is fixed in `src/config.h`.

Default poll interval: **10 seconds**. Supported range: **5–300 seconds**.

The ESP8266 performs polling itself. Browser requests never trigger a remote API request:

```text
External API -> periodic ESP8266 poll -> RAM last-known-good cache -> web UI / TM1637
```

### Supported top-level fields

- `timestamp_utc`
- `local_date`
- `timezone`
- `last_measurement_at`
- `current_solar_production_w`
- `current_grid_power_w`
- `current_grid_import_w`
- `current_grid_export_w`
- `current_total_consumption_w`
- `daily_solar_production_kwh`
- `daily_grid_import_kwh`
- `daily_grid_export_kwh`
- `total_solar_production_kwh`
- `total_grid_import_kwh`
- `total_grid_export_kwh`

### Supported `air_sensor` fields

- `enabled`, `configured`, `ok`, `cached`
- `temperature_c`, `humidity_percent`, `dew_point_c`
- `pressure_hpa`, `pressure_sea_level_hpa`
- `sds_p1` (PM10), `sds_p2` (PM2.5)
- `age_seconds`, `software_version`, `last_success_at`, `last_error`
- Weather Underground status metadata included by the source API

Unknown fields are ignored. Missing optional fields remain unavailable rather than causing a crash. A response is accepted only if valid JSON contains at least one supported numeric measurement.

### Failure and cache behavior

- HTTP timeout is bounded.
- Response size is limited.
- Invalid JSON is rejected.
- Non-200 responses are rejected.
- A failed request does **not** erase the last valid measurements.
- Cached values are marked stale after an appropriate age threshold.
- Failures never deliberately reboot the ESP8266.
- Successful normal polls are not logged continuously; state transitions are logged instead.

No measurement history is stored persistently.

## TM1637 display

Selectable metrics:

- Current solar production
- Current grid power
- Current grid import
- Current grid export
- Current total consumption
- Daily solar / import / export
- Total solar / import / export
- External air temperature
- External humidity
- External dew point
- PM10 / PM2.5

Persistent display settings:

- On/off
- Brightness 0–7
- Selected metric
- Refresh interval
- Metric-only mode
- Clock-only mode
- Alternating metric/clock mode
- Alternation interval

### Four-digit formatting

The formatter deliberately fits values to four digits:

- Watt values up to the available digit/sign capacity are shown as integers without leading zeros.
- Larger Watt values are scaled by 1000 and shown with a useful decimal when possible, e.g. `12500 W -> 12.5`.
- kWh values use a dynamic decimal representation such as `6.31`, `12.5`, or `123.4` where the display permits it.
- Negative values preserve the minus sign where they can be represented.
- Missing/invalid values show `----`.

When the internal formatter scales a value by 1000, the web status page marks the rendered value with `×1000`. A four-digit TM1637 cannot display a textual `kW` suffix.

## NTP and timezone

Defaults:

- NTP server: `de.pool.ntp.org`
- POSIX timezone: `CET-1CEST,M3.5.0,M10.5.0/3`

Boot sequence:

1. Start/connect Wi-Fi.
2. Start NTP.
3. Wait for a real SNTP time update, but only for the configured boot timeout.
4. Log success or timeout.
5. Continue boot even if NTP is unavailable.
6. Retry later and periodically resynchronize.

The POSIX timezone handles CET/CEST daylight-saving transitions without hardcoded date tables.

## Web interface

The UI is served locally from PROGMEM. HTML, CSS and JavaScript are static resources; the firmware does **not** build the page with large chains of `String.replace()` calls.

Pages:

- Status
- Measurements
- Display
- System Settings
- System Log
- OTA Update
- Factory Reset

The browser polls local `/api/state` every five seconds. Visible text is translated centrally in `src/java_script.h`; new UI strings should be added to the DE/EN i18n object instead of being scattered through event handlers.

### Local web endpoints

Read-only status endpoints:

- `GET /api/state`
- `GET /api/settings` (never returns the Wi-Fi password)
- `GET /api/log`
- `GET /api/ota/check`

Configuration actions used by the local UI:

- `POST /api/settings/display`
- `POST /api/settings/system`
- `POST /api/log/clear`
- `POST /api/ota/update`
- `POST /api/factory-reset`

There is no external actuator API and no Shelly/relay control API.

## Persistent settings

Configuration is stored in a compact binary record using the ESP8266 Arduino EEPROM emulation. The record contains a magic value, schema version, fixed-size payload and CRC32. Corrupt or incompatible records are rejected and defaults are used instead. Settings are written only when the user changes configuration; live API measurements are never written to flash.

Stored:

- Device name
- Language and theme
- Wi-Fi SSID/password
- External API host and poll interval
- NTP server and POSIX timezone
- Display settings

Not stored:

- External measurements
- API history
- Measurement history
- Logs

There is no LittleFS partition and no filesystem format step. The firmware validates persisted settings with magic/schema/CRC before using them.

## Factory reset

The web UI uses two browser confirmations and then sends an explicit `RESET` token. The firmware clears the EEPROM-backed settings area, commits the change and reboots.

## OTA update

OTA is dedicated to this project and cannot intentionally use GrowTent or GrowTent-S3 firmware.

Configured channel in `src/config.h`:

```text
syschelle/espDisplay
```

The firmware checks the repository's latest GitHub Release, requires an exact `firmware.bin` asset, compares semantic versions, checks the advertised firmware size against available OTA sketch space, downloads only from the configured repository's release asset path, and reboots only after the updater reports success.

**Transition from the development v0.1.0 build:** that firmware still points to `syschelle/EnergyDisplay8266`. Because the final repository is `syschelle/espDisplay`, v0.1.1 must be installed once by USB/serial unless a transition release is also created in the old repository. From v0.1.1 onward, OTA uses `syschelle/espDisplay`.

### OTA security note

Version 0.1.1 restricts metadata and firmware to the configured GitHub project but uses BearSSL `setInsecure()` for GitHub HTTPS to avoid embedding a certificate chain that may expire or change. This protects against accidentally using another configured firmware channel but is **not equivalent to cryptographic firmware authenticity**. A future hardening step should use ESP8266 signed binaries or another maintained trust mechanism before deploying OTA across an untrusted network.

## GitHub Release workflow

`.github/workflows/release-firmware.yml` builds the `d1_mini` ESP8266 environment on pushes and pull requests to `main`. Tags matching `v*` additionally create/update the GitHub Release.

For a tag such as `v0.1.1`, the workflow:

1. Validates that the tag exactly matches `FW_VERSION` in `src/version.h`.
2. Installs PlatformIO.
3. Runs `pio run -e d1_mini`.
4. Verifies `.pio/build/d1_mini/firmware.bin` exists and is non-empty.
5. Produces `firmware.bin` and `firmware.bin.sha256`.
6. Creates the GitHub Release or updates its assets.

No ESP32 or ESP32-S3 environment exists in this repository.

## Versioning

The project starts at **v0.1.0**.

`src/version.h` is the firmware's version source of truth:

```cpp
#define FW_VERSION "v0.1.1"
```

The tag validation step prevents a GitHub release whose tag differs from the compiled firmware version. Every published functional change must receive a new version; never replace behavior under an already published version.

## Project structure

```text
src/
  main.cpp             Startup order and cooperative main loop
  config.h             Hardware pins, limits, OTA identity and defaults
  version.h            Firmware version
  models.h             Settings and cached API data structures
  settings.*           EEPROM-backed preferences-style persistence and validation
  network.*            Wi-Fi and fallback AP
  time_service.*       NTP / local time
  external_api.*       Remote HTTP polling and RAM cache
  display_format.*     Four-digit formatting logic
  display.*            TM1637 rendering and clock/alternate modes
  logging.*            Fixed-size RAM log
  ota.*                GitHub release check and ESP8266 OTA
  web.*                Local web server and JSON endpoints
  index_html.h         Static PROGMEM HTML
  style_css.h          Static PROGMEM CSS
  java_script.h        Static PROGMEM JavaScript and DE/EN i18n
```

## Troubleshooting

### Web UI not reachable

- If station Wi-Fi is unavailable, connect to `espDisplay-Setup`.
- Check the serial log at 115200 baud.
- Verify that the configured SSID/password are correct.

### API unavailable

- Enter only a host/IP, not `http://` and not `/api/current-values`.
- Confirm the server exposes `/api/current-values` over HTTP.
- Check the HTTP status and last error on the Status page.
- Last-known-good values remain visible but are marked stale.

### Display shows `----`

- No successful API value has been received for the selected metric, or the selected optional field is absent.
- Check TM1637 CLK/DIO wiring and the pin mapping under **Display** in the web interface.
- After boot, `8888` should appear briefly when the display is enabled.
- Check API status and the Measurements page.

### Time not synchronized

- The ESP8266 continues operating after the bounded boot timeout.
- Verify internet/DNS access and the configured NTP server.
- Verify the POSIX timezone string.

### Settings are not retained

- Check the serial log for EEPROM commit or CRC validation errors.
- A corrupt/incompatible record is intentionally rejected and defaults are used.
- Factory reset clears only the configured EEPROM-backed settings area.

### OTA fails

- Verify Wi-Fi/internet access.
- Verify the latest release has a `firmware.bin` asset.
- Verify the release tag is newer than the installed semantic version.
- Verify enough OTA sketch space is available.

## License

Apache License 2.0. See `LICENSE`.


### Clock-only operation
In **Clock only** mode, the TM1637 shows only the NTP-synchronized local time. The external API is optional and the API host may be left empty. The metric and alternation controls are not required in this mode.


### Browser cache after firmware updates

The embedded web assets are served with no-cache headers so the browser always uses the JavaScript/CSS matching the running firmware after USB or OTA updates.

### Browser reports `ERR_CONTENT_LENGTH_MISMATCH`

v0.1.1 transfers the embedded web assets from PROGMEM in small HTTP/1.1 chunks. This is intentionally used instead of a single large `send_P()` response because a short ESP8266 TCP send could otherwise advertise more bytes than the browser actually receives. After installing v0.1.1, reload the page once; the assets are served with `no-store`/`no-cache` headers.
