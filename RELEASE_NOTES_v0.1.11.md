# espDisplay v0.1.11

espDisplay v0.1.11 adds a configurable TCP port for the external API and replaces blocking external HTTP polling with a cooperative ESP8266 state machine designed for slow API servers.

## Added

- Configurable external API TCP port in the web interface.
- Default API port: `80`.
- Valid port range: `1-65535`.
- Persistent API port setting with EEPROM schema migration from v0.1.10.
- API request-in-progress status in `/api/state` and the web interface.

## Improved external API polling

The external API client no longer uses the blocking `ESP8266HTTPClient::GET()` / `getString()` path.

After the short TCP/DNS connection phase, the request is handled cooperatively from the normal ESP8266 `loop()`:

- HTTP headers are read incrementally.
- Response bodies are read with a fixed per-loop byte budget.
- Slow responses may remain open for up to 60 seconds without intentionally blocking the local web server or TM1637 update loop.
- `Content-Length`, connection-close response bodies and HTTP chunked transfer encoding are supported.
- Only one external API request may be active at a time.
- The next polling interval starts after the previous request finishes, preventing request pile-ups when the source server is slow.

## Last-known-good cache

The last successfully parsed external values remain in RAM while a new request is running or after a request fails.

A temporary timeout, connection failure, malformed response or server outage therefore does not erase the value currently used by the TM1637 display.

Old data is explicitly marked stale in the local state API and web interface.

Measurements are still not written persistently to flash. After a reboot, the first successful external API response establishes the new RAM cache.

## API endpoint

The host and port are configured separately.

Example:

- Host: `192.168.178.50`
- Port: `8080`

Resulting endpoint:

`http://192.168.178.50:8080/api/current-values`

The API path remains fixed and cannot be supplied by remote JSON data.

## Settings migration

The persistent settings schema is updated from schema 2 to schema 3.

Existing settings are preserved, including:

- Wi-Fi credentials
- Device name
- API host
- API polling interval
- NTP settings
- Display GPIOs and display preferences

Migrated installations receive API port `80` as the default.

## Existing functionality retained

- Verified redirect-free OTA channel through branch `ota`
- `Conn` during startup until valid local time is available
- Runtime-configurable TM1637 CLK and DIO GPIOs
- Right-aligned clock without a leading hour zero
- One-second blinking center colon
- Clock-only mode without external API dependency
- Metric-only and metric/clock alternation modes
- NTP synchronization and POSIX timezone handling
- EEPROM-backed settings with CRC32
- German/English responsive web interface
- Light/dark theme
- Wi-Fi fallback AP
- GitHub Actions ESP8266 firmware build/release workflow

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
