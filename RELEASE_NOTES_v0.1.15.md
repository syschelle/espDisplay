# espDisplay v0.1.15

espDisplay v0.1.15 improves the readability of the **Last measurement** field on the Status page.

## Changed

- The raw `last_measurement_at` timestamp from the external API is no longer displayed unchanged.
- The Status page now renders the timestamp as a localized date and time.
- German example: `01.09.2026, 08:03:27`
- English example: `01/09/2026, 08:03:27`
- Seconds are always included so the exact measurement time remains visible.
- The separate **Data age** field remains unchanged and continues to show relative freshness.

## Timezone handling

The external API already provides both:

- `last_measurement_at`
- `timezone`

When the API timezone is a browser-supported IANA timezone such as `Europe/Berlin`, the browser converts the measurement timestamp into that timezone before displaying it.

If the timezone is missing or not supported by the browser, espDisplay falls back to the browser's local timezone instead of failing to render the timestamp.

API timestamps containing microseconds are normalized to milliseconds before parsing for broad browser compatibility.

## No measurement yet

If the API has not supplied a valid measurement timestamp yet, the Status page now displays a localized message instead of a bare placeholder:

German:

`Noch keine Messung`

English:

`No measurement yet`

## Existing functionality retained

- Redirect-free OTA channel through the generated `ota` branch
- OTA-exclusive hold protecting ESP8266 heap during update check/install
- Configurable external API host and TCP port
- Cooperative slow external API polling
- Last-known-good measurement cache in RAM
- Stale-data reporting
- `Conn` startup display until valid local time is available
- Rounded whole-degree air temperature with degree symbol on the TM1637
- One-second metric phase in alternate mode
- Configurable clock display duration
- Right-aligned clock without a leading hour zero
- One-second blinking clock colon
- Runtime-configurable TM1637 CLK and DIO GPIOs
- NTP synchronization and configurable POSIX timezone
- EEPROM-backed persistent settings
- German and English responsive web interface
- Light and dark themes
- Wi-Fi fallback access point
- GitHub Actions ESP8266 firmware build and release workflow

## Settings

There is no persistent settings schema change in v0.1.15.

Existing v0.1.14 settings are retained.

## OTA channel

Repository:

`syschelle/espDisplay`

Branch:

`ota`

Files:

- `manifest.json`
- `firmware.bin`
- `firmware.bin.sha256`

## Platform

- ESP8266
- Arduino Framework
- PlatformIO
- Wemos/Lolin D1 mini compatible target
- TM1637 four-digit seven-segment display

## License

Apache License 2.0
