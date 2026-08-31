# espDisplay v0.1.10

espDisplay v0.1.10 changes the TM1637 boot display to a single deterministic connection/wait state.

## Changed

- The TM1637 is initialized immediately after persistent settings are loaded, before the potentially slow Wi-Fi and NTP boot operations begin.
- While no valid local time exists, the physical display shows exactly `Conn`.
- The previous `8888` startup self-test has been removed.
- No `----`, metric value, clock frame, or other startup placeholder is shown before valid local time is available.
- As soon as local time becomes valid, the configured normal display mode takes over automatically.

## `Conn` display

The four physical TM1637 digits use seven-segment approximations for:

`C o n n`

The state remains static; there is no blinking colon or alternating content while the device is waiting for valid time.

This applies regardless of whether the later configured operating mode is:

- Clock only
- Metric only
- Metric / Clock alternation

If the display is disabled in settings, it remains disabled and does not show `Conn`.

## Boot sequence

The relevant startup order is now:

1. Start logging.
2. Load persistent EEPROM settings.
3. Initialize TM1637 and show `Conn` when enabled.
4. Connect Wi-Fi or start the fallback access point.
5. Start the bounded NTP synchronization attempt.
6. Continue normal startup even when NTP times out.
7. Keep showing `Conn` until a valid local time becomes available.
8. Switch directly to the configured normal display mode.

## OTA validation

v0.1.10 retains the corrected redirect-free OTA implementation from v0.1.9.

If the device already runs v0.1.9, v0.1.10 is suitable for validating the full OTA path:

`v0.1.9 -> ota/manifest.json -> ota/firmware.bin -> v0.1.10`

The tagged GitHub Actions workflow updates the generated `ota` branch with:

- `manifest.json`
- `firmware.bin`
- `firmware.bin.sha256`
- `README.md`

## Existing functionality retained

- Redirect-free OTA channel through `raw.githubusercontent.com`
- Runtime-configurable TM1637 CLK and DIO GPIOs
- Numeric GPIO input fields
- Right-aligned clock without a leading hour zero
- One-second blinking center colon after time is valid
- Clock-only operation without an external measurement API
- Metric-only mode
- Metric / Clock alternation mode
- External `/api/current-values` polling
- RAM last-known-good API cache
- Stale-data handling
- NTP synchronization with configurable POSIX timezone
- EEPROM-backed persistent settings
- German and English responsive web interface
- Light and dark themes
- Wi-Fi fallback access point
- Chunked PROGMEM web asset delivery
- GitHub Actions ESP8266 firmware build and release workflow

## Settings

There is no settings schema change in v0.1.10. Existing EEPROM settings are retained.

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
