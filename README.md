# espDisplay

espDisplay is a small, independent ESP8266 network display. It periodically reads energy and optional air-quality values from an external HTTP endpoint, keeps the latest valid values in RAM, shows a selected value on a four-digit TM1637 display, and exposes a responsive local web interface.

This repository is **not** a GrowTent firmware variant. It has its own PlatformIO environment, version series, OTA channel and release workflow. GrowTent was used only as a design and architecture reference for the web UI, Wi-Fi fallback concept, NTP/OTA concepts and code organization.

## Main features

- ESP8266 / Arduino Framework / PlatformIO
- TM1637 four-digit display
- External API polling at `http://<host>:<port>/api/current-values`
- Last-known-good RAM cache with stale/error state
- No sensor, relay, pump, Shelly, VPD, irrigation, tank or HomeKit logic
- Responsive GrowTent-inspired web UI with light/dark theme
- German and English UI with centralized browser-side i18n
- Wi-Fi station mode with fallback access point
- NTP with POSIX timezone support and bounded boot synchronization
- Compact EEPROM-backed settings persistence with magic/schema/CRC validation
- Read-only local `/api/state` for the web UI
- Redirect-free GitHub `ota` branch used as the ESP8266 OTA channel, while tagged releases still publish normal GitHub Release assets
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

At boot, an enabled display immediately shows `Conn` while no valid local time is available. `Conn` remains the only physical display output during Wi-Fi/NTP startup. As soon as a valid local time exists, the normal configured display mode takes over.

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

Configure the host/IP and TCP port separately in the web UI, for example:

```text
Host: 192.168.178.50
Port: 8080
```

The firmware constructs exactly:

```text
http://192.168.178.50:8080/api/current-values
```

The default port is **80**. Schemes, embedded paths and arbitrary URLs are not accepted in the host field. The endpoint path remains fixed in `src/config.h`.

Default poll interval: **10 seconds**. Supported range: **5–300 seconds**.

The external API client is implemented as a cooperative state machine rather than a FreeRTOS task. The ESP8266 only performs the short TCP/DNS connection phase synchronously with a tightly bounded timeout. Waiting for slow HTTP headers/body and receiving the JSON response are handled incrementally from the normal `loop()`, with a fixed per-loop read budget. This keeps the local web server, TM1637 clock, NTP, Wi-Fi maintenance and OTA responsive while a slow API request is still in progress.

```text
External API -> cooperative ESP8266 background poll -> RAM last-known-good cache -> web UI / TM1637
```

Only one external API request may be active at a time. The next polling interval starts after the previous request finishes, so a slow server cannot create overlapping requests.

The API response body buffer is allocated dynamically only while a response is being received and is released immediately after success, failure, timeout or cancellation. This avoids permanently reserving roughly 8 KB of scarce ESP8266 DRAM. Starting with v0.1.17 the ESP8266 no longer opens GitHub TLS connections for OTA at all. The browser downloads and verifies release artifacts, and the external API is suspended only while the already-verified firmware is streamed locally into flash.

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

### Slow API, failure and cache behavior

- TCP/DNS connection establishment is bounded to a short timeout.
- Waiting for the HTTP response is cooperative and does not block the main loop.
- A request may remain open for up to 60 seconds without freezing the local UI/display.
- Response reading is limited to a small number of bytes per main-loop pass.
- `Content-Length`, connection-close bodies and HTTP chunked transfer encoding are supported.
- Response size is limited to 8192 bytes.
- Invalid JSON is rejected.
- Non-200 responses are rejected.
- A failed or still-running request does **not** erase the last valid measurements.
- The last-known-good measurements remain in RAM and continue to be available to the TM1637 and web interface.
- Old values are explicitly marked stale; they are never presented as fresh data.
- No overlapping API requests are started.
- Failures never deliberately reboot the ESP8266.
- Successful normal polls are not logged continuously; state transitions are logged instead.

No measurement history is stored persistently. After a reboot there is intentionally no persisted measurement cache; the first valid response establishes the new RAM cache.

## TM1637 display

Selectable metrics:

- Current solar production
- Current grid power
- Current grid import
- Current grid export
- Current total consumption
- Daily solar / import / export
- Total solar / import / export
- External air temperature (rounded to a whole degree with `°` on the TM1637)
- External humidity
- External dew point
- PM10 / PM2.5

Persistent display settings:

- On/off
- Brightness 0–7
- Selected metric
- Refresh interval (250–200,000 ms / up to 200 seconds)
- Metric-only mode
- Clock-only mode
- Alternating metric/clock mode
- Clock display duration in alternating mode (seconds)
- API-value display duration in alternating mode (100–200,000 ms)

### Four-digit formatting

The formatter deliberately fits values to four digits:

- Watt values up to the available digit/sign capacity are shown as integers without leading zeros.
- Larger Watt values are scaled by 1000 and shown with a useful decimal when possible, e.g. `12500 W -> 12.5`.
- kWh values use a dynamic decimal representation such as `6.31`, `12.5`, or `123.4` where the display permits it.
- Negative values preserve the minus sign where they can be represented.
- External air temperature is rounded to the nearest whole degree and uses the final TM1637 digit as a seven-segment degree symbol, e.g. `23.6 °C -> 24°`.
- Missing/invalid values show `----`.

In alternating mode, the clock and API-value phases are configured independently. The clock duration remains in seconds, while the API-value duration is configured in milliseconds. For example, `clock = 10 s` and `API value = 500 ms` produces `clock 10 s -> metric 500 ms -> clock 10 s -> metric 500 ms`. The API-value duration defaults to 1000 ms so existing behavior is preserved after migration.

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

On the **Status** page, `last_measurement_at` is rendered as a localized absolute date and time (for example `01.09.2026, 08:03:27`) using the timezone reported by the external API when it is a valid browser-supported IANA timezone. If no measurement timestamp is available yet, the UI shows a localized “No measurement yet” message. The separate data-age field remains available for relative freshness.

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
- External API host, port and poll interval
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

Repository:

```text
syschelle/espDisplay
```

Tagged releases continue to publish a dedicated `ota` branch containing exactly:

```text
manifest.json
firmware.bin
firmware.bin.sha256
README.md
```

### Browser-assisted OTA (v0.1.17+)

Starting with **v0.1.17**, GitHub HTTPS is handled by the browser instead of the ESP8266. This removes BearSSL/GitHub TLS handshakes from the resource-constrained device and prevents an update check from consuming enough ESP8266 heap to trigger a reset.

The browser uses the pinned channel:

```text
https://raw.githubusercontent.com/syschelle/espDisplay/ota/manifest.json
https://raw.githubusercontent.com/syschelle/espDisplay/ota/firmware.bin
```

If a direct raw-content browser fetch fails, the UI can fall back to the GitHub Contents API for the same fixed repository and branch. The configured external measurement API is never involved in OTA discovery.

The manifest contains release metadata, for example:

```json
{
  "version": "v0.1.17",
  "firmware": "firmware.bin",
  "size": 412345,
  "sha256": "..."
}
```

The **Check for update** path is browser-only:

1. The browser downloads `ota/manifest.json` over normal browser HTTPS.
2. It validates the semantic version, fixed `firmware.bin` name, positive size and SHA-256 format.
3. It compares the manifest version with the installed firmware version obtained from the ESP's local `/api/state`.
4. The ESP8266 does not contact GitHub, does not allocate BearSSL state, does not touch flash and does not reboot.

The **Install update** path is:

1. The browser downloads the fixed `ota/firmware.bin`.
2. The browser checks the exact manifest size.
3. The browser checks the ESP8266 image magic byte `0xE9`.
4. The browser computes SHA-256 locally and requires an exact match with the manifest.
5. The browser requests a short-lived one-time OTA upload token from the ESP over the same local origin.
6. The browser sends the verified binary to the ESP over the existing local HTTP connection using multipart upload and that token.
7. At upload start the ESP aborts/suspends the slow external API client to free sockets and temporary buffers.
8. `ESP8266WebServer` supplies the upload in small chunks; those chunks are written directly through `Update.write()`.
9. The ESP also validates the first firmware byte, exact expected size and that the target semantic version is newer than the installed version.
10. The ESP calls `Update.end()` only after the exact expected byte count has been received.
11. A reboot is scheduled only after the local HTTP upload has returned success.

The browser pauses `/api/state` polling only while firmware installation is running. Update checking itself does not pause the ESP or external API.

This design uses the desktop/mobile browser's mature TLS stack and certificate validation while keeping the ESP8266 OTA side to a local streaming flash operation.

### Upgrade transition

- v0.1.7 introduced the redirect-free `ota` branch.
- v0.1.9 fixed the original firmware-stream header handoff bug.
- v0.1.10 successfully validated device-side redirect-free OTA on real hardware.
- v0.1.11 added the cooperative slow external API poller and configurable API port.
- v0.1.12/v0.1.14 added increasingly strict heap coordination between the external API and BearSSL OTA.
- v0.1.16 can still reboot during **Check for update** on affected devices because the manifest TLS handshake itself still runs on the ESP8266.
- **v0.1.17 removes GitHub TLS from the ESP8266 entirely and replaces it with browser-assisted OTA.**

Because the new local firmware-upload endpoint does not exist in v0.1.16, an affected v0.1.16 device that reboots during update checks must install v0.1.17 once via USB/serial. Do not erase flash if EEPROM settings should be retained. Future releases can then use the browser-assisted OTA path.

### OTA security note

The browser obtains the manifest and firmware over normal HTTPS and validates the firmware SHA-256 before it is sent to the ESP8266. The local upload additionally requires a short-lived one-time token generated by the ESP. The ESP accepts only a semantic version newer than the installed version, requires the expected size, and validates the ESP8266 firmware magic byte.

The SHA-256 is still published beside the firmware in the same repository, so it is an integrity check rather than an independent cryptographic release signature. Signed firmware remains a possible future hardening step.

## GitHub Release workflow

`.github/workflows/release-firmware.yml` builds the `d1_mini` ESP8266 environment on pushes and pull requests to `main`. Tags matching `v*` perform both the normal GitHub Release and the dedicated ESP8266 OTA publication.

For a tag such as `v0.1.17`, the workflow:

1. Validates that the tag exactly matches `FW_VERSION` in `src/version.h`.
2. Installs PlatformIO and runs the project checks.
3. Runs `pio run -e d1_mini`.
4. Verifies `.pio/build/d1_mini/firmware.bin` exists and is non-empty.
5. Produces `firmware.bin` and `firmware.bin.sha256`.
6. Generates `manifest.json` from the tag, real firmware size and SHA-256.
7. Creates or updates the normal GitHub Release with `firmware.bin` and `firmware.bin.sha256`.
8. Creates or updates branch `ota` and publishes `manifest.json`, `firmware.bin`, `firmware.bin.sha256` and its generated README.

No ESP32 or ESP32-S3 environment exists in this repository.

## Versioning

The project starts at **v0.1.0**.

`src/version.h` is the firmware's version source of truth:

```cpp
#define FW_VERSION "v0.1.17"
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
  external_api.*       Cooperative remote HTTP state machine and RAM cache
  display_format.*     Four-digit formatting logic
  display.*            TM1637 rendering and clock/alternate modes
  logging.*            Fixed-size RAM log
  ota.*                Local streaming firmware validation and flash writer
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
- Immediately after boot, `Conn` should appear when the display is enabled.
- `Conn` remains visible until a valid local time is available. If it is not visible, verify power, CLK/DIO order and the configured GPIOs.
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

- Verify the browser has internet access to GitHub.
- Verify the tagged workflow completed successfully.
- Verify branch `ota` contains `manifest.json`, `firmware.bin` and `firmware.bin.sha256`.
- Open the raw `manifest.json` and confirm its version is newer than the installed semantic version.
- During installation, keep the browser tab open until the local upload reaches 100%.
- Check the system log for local upload/flash errors and the recorded reset reason after any unexpected reboot.
- Verify enough OTA sketch space is available.

## License

Apache License 2.0. See `LICENSE`.


### Clock-only operation
In **Clock only** mode, the TM1637 shows only the NTP-synchronized local time. The external API is optional and the API host may be left empty. The metric and alternation controls are not required in this mode.

Starting with **v0.1.2**, the TM1637 center colon blinks once per second while the clock is visible. The colon is lit on even seconds and dark on odd seconds. This timing is independent of the configured metric/display refresh interval, so a slow value refresh such as 4000 ms does not slow down the clock colon. The same behavior is used during the clock phase of the alternating metric/clock mode.

Starting with **v0.1.4**, single-digit hours are shown without a leading zero while the complete time remains right-aligned. Examples: `9:05` is rendered in the four physical positions as `[blank][9][0][5]`, while `12:34` uses all four digits. Midnight `0:05` is rendered as `[blank][0][0][5]`. Minutes always keep two digits.


### Browser cache after firmware updates

The embedded web assets are served with no-cache headers so the browser always uses the JavaScript/CSS matching the running firmware after USB or OTA updates.

### Browser reports `ERR_CONTENT_LENGTH_MISMATCH`

v0.1.1 transfers the embedded web assets from PROGMEM in small HTTP/1.1 chunks. This is intentionally used instead of a single large `send_P()` response because a short ESP8266 TCP send could otherwise advertise more bytes than the browser actually receives. After installing v0.1.1, reload the page once; the assets are served with `no-store`/`no-cache` headers.
