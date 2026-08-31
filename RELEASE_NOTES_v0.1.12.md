# espDisplay v0.1.12

espDisplay v0.1.12 fixes an OTA TLS resource regression introduced by the slow, cooperative external API poller in v0.1.11.

## Fixed

- Fixed OTA manifest checks failing with `OTA manifest returned HTTP -1` after v0.1.11.
- External API requests are now suspended before an OTA manifest check or firmware installation.
- Any currently active slow API TCP connection is explicitly aborted before BearSSL starts the OTA TLS connection.
- API response memory is released before OTA begins.
- API polling resumes automatically after an OTA check or failed OTA install.
- After a successful OTA flash, external API polling remains suspended until the scheduled reboot.

## Memory improvement

v0.1.11 used a fixed API response body array sized for the configured maximum JSON response.

That reserved more than 8 KB of ESP8266 DRAM for the API body even while no API response was being received.

v0.1.12 changes the API body buffer to request-scoped dynamic storage:

- no large API body buffer is permanently reserved while idle
- the buffer is allocated only when an HTTP response body is actually needed
- known `Content-Length` responses allocate enough capacity for the announced body
- unknown/chunked responses grow the buffer geometrically up to the configured maximum
- the buffer is freed after success, failure, timeout, cancellation or OTA suspension

The last-known-good measurement cache is a separate compact structure and remains available while the response buffer is released.

## OTA and slow API coordination

BearSSL needs significant ESP8266 heap when establishing an HTTPS/TLS connection.

The OTA web handlers now coordinate explicitly with the external API poller:

1. Suspend external API polling.
2. Abort an active external API TCP request.
3. Release its temporary response buffer.
4. Yield briefly so the ESP8266 network stack can release resources.
5. Start the OTA BearSSL connection.
6. Resume API polling after a normal OTA check or failed installation.

This prevents a very slow external HTTP API from competing with the OTA TLS connection for sockets and heap.

## Better OTA diagnostics

Negative ESP8266 HTTPClient return values are transport errors rather than HTTP status codes.

OTA failures now include the actual HTTPClient error string and current free heap.

Example:

`OTA manifest connection failed -1: connection failed (heap 28640)`

The OTA log also records free heap immediately before manifest and firmware TLS operations.

This makes future network-versus-memory failures much easier to diagnose.

## External API behavior retained

- Configurable API host and TCP port
- Fixed `/api/current-values` path
- Cooperative non-FreeRTOS request state machine
- Incremental header/body receive handling
- Bounded TCP connect timeout
- Up to 60 seconds for a slow response
- Fixed per-loop receive budget
- `Content-Length` support
- chunked transfer support
- no overlapping API requests
- last-known-good RAM cache
- stale-state reporting on timeout or failure

Aborting an in-progress refresh for OTA does not clear the previous valid measurement values.

## Existing functionality retained

- Verified redirect-free OTA channel via branch `ota`
- `Conn` startup display until valid local time exists
- Runtime-configurable TM1637 CLK/DIO GPIOs
- Numeric GPIO fields
- Right-aligned clock without leading hour zero
- One-second blinking center colon
- Clock-only mode without an external API
- Metric-only and metric/clock alternation modes
- NTP synchronization and configurable POSIX timezone
- EEPROM-backed settings with CRC32
- German/English responsive web UI
- Light/dark theme
- Wi-Fi fallback AP
- GitHub Actions ESP8266 build/release workflow

## Settings

There is no settings schema change in v0.1.12.

Existing v0.1.11 settings are retained, including the configured API port and display GPIOs.

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
