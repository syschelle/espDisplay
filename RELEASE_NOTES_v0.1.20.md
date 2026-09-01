# espDisplay v0.1.20

espDisplay v0.1.20 simplifies the external API integration to a single value: **air temperature**.

## Changed

- The external API parser now consumes only `air_sensor.temperature_c` as measurement data.
- `air_sensor.last_success_at`, root `last_measurement_at` as fallback, and `timezone` are retained only for the readable **Last measurement** status timestamp.
- Power, energy, humidity, dew point, pressure, PM10, PM2.5 and other API data are ignored.
- Removed the complete **Measurements** page from the web interface.
- Removed the display metric selector because temperature is now the only API value.
- Renamed display mode wording from generic API value to **Temperature**.
- The local `/api/state` response now exposes only the external temperature plus timestamp metadata instead of the former large measurement tree.

## Runtime simplification

The in-memory `ExternalValues` cache was reduced from approximately 712 bytes to approximately 224 bytes in the host layout used by the project checks.

The simplification also removes substantial source/UI code associated with unused measurements:

- models: about 1.1 KB less source
- external API parser: about 3.9 KB less source
- HTML: about 3.1 KB less source
- JavaScript: about 5.2 KB less source
- web/API implementation: about 3.5 KB less source

Actual firmware flash/RAM deltas depend on the ESP8266 compiler/linker and should be confirmed by the PlatformIO build output.

## External API

The endpoint remains unchanged:

`http://<host>:<port>/api/current-values`

The parser now filters for only:

```json
{
  "timezone": "Europe/Berlin",
  "last_measurement_at": "2026-09-01T12:00:00+00:00",
  "air_sensor": {
    "temperature_c": 23.6,
    "last_success_at": "2026-09-01T12:00:00Z"
  }
}
```

`air_sensor.last_success_at` is preferred for the Status page because it represents the temperature source. Root `last_measurement_at` is used as a compatibility fallback.

A response without a valid `air_sensor.temperature_c` is treated as an unsuccessful API refresh. The existing last-known-good temperature remains in RAM and becomes stale according to the existing stale-data policy.

## Display behavior

The physical TM1637 continues to support:

- Temperature only
- Clock only
- Temperature and clock alternating

Temperature remains:

- rounded to a whole degree
- displayed with a degree symbol
- marked with the lower-segment stale indicator after a prolonged API outage

After reboot, alternating mode remains on the clock until the first valid temperature has been received.

## Display timing

Existing independent timing settings remain available:

- clock display duration in seconds
- temperature display duration in milliseconds
- general TM1637 refresh interval up to 200 seconds

## Web interface

Current pages are:

- Status
- Display
- System Settings
- System Log
- OTA Update
- Factory Reset

The former **Measurements** page is no longer present.

The Status page still shows:

- API connection state
- endpoint
- last request
- readable date/time of the last temperature measurement
- data age
- HTTP status
- last API error
- current temperature status / freshness

## Settings compatibility

There is **no EEPROM schema change** in v0.1.20.

Existing schema 5 settings remain readable. The historical selected-metric byte is retained only in the frozen persisted record layout for backward compatibility and is ignored by the runtime.

When settings are saved again, that legacy byte is normalized to the historical air-temperature selection value.

All other settings remain unchanged.

## OTA

v0.1.20 retains the browser-assisted OTA process introduced previously.

The browser performs GitHub HTTPS access, validates the firmware and uploads it locally to the ESP8266. The ESP8266 itself does not open a GitHub TLS connection.

OTA channel:

`syschelle/espDisplay`, branch `ota`

## Existing functionality retained

- configurable API host and port
- cooperative slow-API polling
- RAM-only last-known-good temperature cache
- stale-data warning segment
- `Conn` startup display until valid time exists
- NTP synchronization
- configurable TM1637 GPIOs
- configurable display brightness
- configurable display refresh interval
- independent clock and temperature phase timing
- EEPROM settings with CRC32
- German and English UI
- light and dark themes
- Wi-Fi fallback access point
- browser-assisted OTA
- reset-reason logging

## Platform

- ESP8266
- Arduino Framework
- PlatformIO
- Wemos/Lolin D1 mini compatible target
- TM1637 four-digit seven-segment display

## License

Apache License 2.0
