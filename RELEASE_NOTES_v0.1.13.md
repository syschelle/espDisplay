# espDisplay v0.1.13

espDisplay v0.1.13 improves the physical TM1637 temperature presentation and makes alternating clock/metric timing deterministic.

## Changed

- External air temperature is now always rounded to the nearest whole degree on the TM1637.
- The temperature display now uses a dedicated seven-segment degree symbol in the final digit.
- Examples:
  - `22.49 °C -> 22°`
  - `22.50 °C -> 23°`
  - `23.6 °C -> 24°`
  - `-4.6 °C -> -5°`
- Temperature decimals are no longer shown on the physical display.

## Alternate display mode

The configured alternation value now defines only how long the clock is displayed.

The selected API metric is always displayed for exactly one second.

Example with a configured value of 10 seconds:

`clock 10 s -> metric 1 s -> clock 10 s -> metric 1 s`

The alternate cycle starts with a complete clock phase after valid local time becomes available or after display settings are reapplied.

The web UI label has been updated accordingly:

- German: `Uhrzeit-Anzeigedauer (s)`
- English: `Clock display duration (s)`

## Temperature representation

The four-digit TM1637 reserves one digit for the degree symbol while displaying external air temperature. This leaves three character positions for the signed rounded number. Values that cannot be represented safely in that space fall back to the existing invalid-value display rather than truncating digits.

Only the selected external air temperature metric receives this special whole-degree `°` formatting. Other metrics retain their previous formatting behavior.

## Existing functionality retained

- Redirect-free OTA channel through the generated `ota` branch
- OTA/API resource coordination from v0.1.12
- Configurable external API host and TCP port
- Cooperative slow-API polling
- RAM last-known-good cache
- `Conn` startup display until valid local time exists
- Runtime-configurable TM1637 CLK and DIO GPIOs
- Right-aligned clock without a leading hour zero
- One-second blinking center colon
- Metric-only mode
- Clock-only mode
- NTP synchronization and configurable POSIX timezone
- EEPROM-backed persistent settings
- German and English responsive web interface
- Light and dark themes
- Wi-Fi fallback access point
- GitHub Actions ESP8266 firmware build/release workflow

## Settings

There is no persistent settings schema change in v0.1.13. Existing v0.1.12 settings are retained.

The existing `alternateSeconds` setting is preserved; only its behavior is clarified and changed so that it controls the clock phase duration instead of both phases.

## Platform

- ESP8266
- Arduino Framework
- PlatformIO
- Wemos/Lolin D1 mini compatible target
- TM1637 4-digit 7-segment display

## Repository

`syschelle/espDisplay`

## License

Apache License 2.0
