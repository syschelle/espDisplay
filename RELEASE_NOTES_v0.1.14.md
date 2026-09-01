# espDisplay v0.1.14

espDisplay v0.1.14 fixes an ESP8266 heap/resource regression that could prevent the firmware TLS connection from starting after a successful OTA update check.

## Fixed

- Fixed OTA firmware downloads failing with ESP8266 HTTP client error `-1` after an update had already been detected.
- The external API is no longer resumed between a successful OTA manifest check and the firmware installation step.
- A pending OTA update now receives an exclusive network/heap window while waiting for the user to start installation.
- The install endpoint refreshes the OTA manifest while the external API is still suspended, then proceeds directly to the firmware TLS connection.
- Any active slow external API request is aborted before OTA TLS work begins.
- Temporary external API response memory is released before OTA TLS work begins.
- The OTA hold automatically expires after five minutes if the user does not start the update, so normal API polling is not left disabled indefinitely.

## Improved diagnostics

OTA log entries now include:

- total free heap
- largest contiguous free heap block
- heap fragmentation percentage

Example:

`Firmware update starting (free 28640, max block 24816, frag 13%)`

Negative TLS connection errors also report both the remaining free heap and largest available block.

This is more useful than total free heap alone because BearSSL requires comparatively large allocations when a TLS connection is established.

## OTA / external API coordination

The OTA sequence is now:

1. Suspend external API polling.
2. Abort any active external API socket.
3. Release any temporary API response buffer.
4. Allow the ESP8266 network stack to settle.
5. Download and validate `ota/manifest.json`.
6. If no update is available, resume normal API polling immediately.
7. If an update is available, keep external API polling suspended while the update is pending.
8. When installation is requested, refresh the manifest again while the API remains suspended.
9. Start the `ota/firmware.bin` TLS connection immediately afterwards.
10. Stream the firmware directly into the ESP8266 updater.
11. Reboot after a successful flash.
12. Resume external API polling after a failed OTA attempt.

## Five-minute safety timeout

If an update is detected but installation is not started, the exclusive OTA hold expires automatically after five minutes.

Normal external API polling then resumes.

Starting an installation always creates a fresh OTA hold again, so installation does not depend on the earlier hold still being active.

## External API behavior retained

- Configurable API host
- Configurable API TCP port
- Fixed `/api/current-values` path
- Cooperative slow-response polling
- No overlapping API requests
- Last-known-good measurement cache in RAM
- Stale-value reporting
- Up to 60 seconds response time
- Incremental response processing

## Display behavior retained

- `Conn` while valid local time is unavailable
- Rounded whole-degree air temperature
- Degree symbol on the TM1637 temperature display
- One-second metric phase in alternate mode
- Configurable clock display duration
- Right-aligned clock without leading hour zero
- One-second blinking clock colon

## Settings

There is no persistent settings schema change in v0.1.14.

Existing v0.1.13 settings are retained.

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
- TM1637 4-digit seven-segment display

## License

Apache License 2.0
