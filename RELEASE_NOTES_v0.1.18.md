# espDisplay v0.1.18

espDisplay v0.1.18 makes the API-value phase duration independently configurable in milliseconds when the TM1637 uses alternating clock/API mode.

## Added

- New display setting: **API value display duration (ms)**.
- German label: **API-Wert-Anzeigedauer (ms)**.
- Configurable range: `100` to `200000` ms.
- Default: `1000` ms, preserving the previous one-second API-value phase after upgrade.

## Alternating display timing

The two phases are now configured independently:

- Clock display duration: seconds
- API-value display duration: milliseconds

Example:

- Clock display duration: `10 s`
- API-value display duration: `500 ms`

Result:

`clock 10 s -> API value 500 ms -> clock 10 s -> API value 500 ms`

The alternating cycle still starts with a complete clock phase.

## Persistence / migration

v0.1.18 updates the EEPROM settings schema from version 4 to version 5.

The new `apiValueDisplayMs` setting is stored as a 32-bit unsigned value.

Existing schema 4 settings are migrated automatically. All existing Wi-Fi, API, NTP and display settings are retained. During migration, the new API-value duration receives the default value of `1000 ms`, matching the behavior of v0.1.17.

Legacy schema 1, 2 and 3 migration paths remain supported.

## Existing functionality retained

- Browser-assisted OTA without GitHub TLS on the ESP8266
- External API host and configurable TCP port
- Cooperative slow-API polling
- Last-known-good measurement cache in RAM
- Localized last-measurement date and time
- `Conn` startup display until valid local time exists
- Rounded whole-degree temperature with degree symbol
- Configurable TM1637 refresh interval up to 200 seconds
- Configurable clock phase duration
- Right-aligned clock without leading hour zero
- One-second blinking center colon
- Runtime-configurable TM1637 GPIOs
- NTP synchronization and configurable POSIX timezone
- EEPROM-backed settings with CRC32
- German and English responsive web UI
- Light and dark themes
- Wi-Fi fallback access point
- GitHub Actions firmware build and release workflow

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
