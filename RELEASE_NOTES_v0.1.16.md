# espDisplay v0.1.16

espDisplay v0.1.16 increases the configurable TM1637 display refresh interval maximum from 10 seconds to 200 seconds.

## Changed

- Increased **Display refresh / Aktualisierung (ms)** maximum from `10000` ms to `200000` ms.
- The web interface now accepts values from `250` ms through `200000` ms.
- Firmware-side validation accepts the same range.
- `displayUpdateMs` is now a 32-bit value so 200 seconds can be represented without integer overflow.

## Settings migration

The persistent settings schema is updated from schema 3 to schema 4.

The previous `displayUpdateMs` field used `uint16_t`, whose maximum value is 65,535 ms. v0.1.16 stores this field as `uint32_t`, allowing the requested 200,000 ms maximum.

Existing schema 3 settings are migrated automatically on startup. Existing Wi-Fi, API, NTP, display GPIO, brightness, metric, mode and timing settings are retained.

Older schema 1 and schema 2 records remain supported by the migration path.

## Effective range

- Minimum: `250 ms`
- Maximum: `200000 ms` / `200 seconds`

The unit in the web interface remains milliseconds.

## Existing functionality retained

- Redirect-free OTA channel through the generated `ota` branch
- OTA-exclusive hold protecting ESP8266 heap during update check/install
- Configurable external API host and TCP port
- Cooperative slow external API polling
- Last-known-good measurement cache in RAM
- Readable localized last-measurement date and time
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
