# espDisplay v0.1.19

espDisplay v0.1.19 improves the physical TM1637 behavior when external API data is not yet available or becomes stale during operation.

## Changed

- Alternating clock/API mode no longer switches to an empty or invalid API-value phase after reboot.
- Until the selected API metric has been received successfully at least once, the TM1637 remains on the clock.
- Once a valid selected metric becomes available, the normal alternating cycle begins with a complete clock phase.
- Last-known-good API values continue to be retained in RAM during slow responses or API outages.
- Stale temperature data now receives a physical warning indicator on the TM1637.

## No API data after reboot

Measurement values are intentionally not persisted across restarts.

After a reboot there is therefore no last-known-good measurement until the first successful external API response arrives.

In alternating mode, v0.1.19 keeps showing the clock during this period and does not switch to `----` or an empty API-value phase.

As soon as the selected API metric is available, the alternating cycle starts from a complete clock phase and then follows the configured clock/API durations.

Metric-only mode keeps its existing behavior for a missing selected metric.

## Stale temperature indicator

When a valid temperature was received previously but no fresh API data has arrived for a sufficiently long period, the existing last-known-good rounded temperature remains visible.

The final TM1637 digit normally renders the degree symbol using segments:

`A + B + F + G`

When that cached API data becomes stale, segment `D` is added to the same degree digit.

This creates an additional lower horizontal line below the degree symbol without changing the numeric temperature.

Example conceptually:

`24°` -> fresh

`24°` with lower line in the degree digit -> stale

The warning line is removed automatically as soon as a successful fresh API response arrives.

## Stale threshold

The physical warning does not appear immediately after a single failed request.

The stale threshold is the greater of:

- 30 seconds
- two configured external API polling intervals

Examples:

- 10 second polling -> warning after more than 30 seconds without a successful refresh
- 20 second polling -> warning after more than 40 seconds
- 300 second polling -> warning after more than 600 seconds

This avoids visually flagging brief transient API failures while still making longer outages obvious.

## Immediate recovery indication

A fresh/stale state transition forces an immediate redraw of the API value.

This means the warning line disappears as soon as fresh data arrives even when the normal TM1637 refresh interval is configured to a very large value.

## Existing functionality retained

- Browser-assisted OTA without GitHub TLS on the ESP8266
- Configurable external API host and TCP port
- Cooperative slow external API polling
- Last-known-good measurement cache in RAM
- Localized last-measurement date and time
- `Conn` startup display until valid local time exists
- Rounded whole-degree temperature with degree symbol
- Independently configurable clock and API-value phase durations
- Configurable TM1637 refresh interval up to 200 seconds
- Right-aligned clock without a leading hour zero
- One-second blinking center colon
- Runtime-configurable TM1637 GPIOs
- NTP synchronization and configurable POSIX timezone
- EEPROM-backed settings with CRC32
- German and English responsive web UI
- Light and dark themes
- Wi-Fi fallback access point
- GitHub Actions firmware build and release workflow

## Settings

There is no EEPROM schema change in v0.1.19.

Existing v0.1.18 settings are retained unchanged.

## OTA channel

Repository: `syschelle/espDisplay`

Branch: `ota`

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
