# espDisplay v0.1.5

espDisplay v0.1.5 replaces the previous ESP8266 GitHub OTA download path after v0.1.3 still failed with `-104 / Wrong HTTP Code` when downloading a release asset.

## Fixed

- Removed `ESPhttpUpdate` from the firmware download/install path.
- GitHub release asset redirects are now resolved explicitly by espDisplay.
- The initial GitHub asset request runs with automatic redirects disabled so the firmware can inspect the actual response code and `Location` header.
- The signed GitHub CDN URL is accepted only when it points to `release-assets.githubusercontent.com`.
- The firmware download uses a new TLS connection to the resolved CDN URL.
- The CDN response must be HTTP 200.
- The real HTTP status code is now written to the system log when a download fails instead of only reporting generic updater error `-104`.
- The CDN `Content-Length` must match the firmware size reported by the GitHub Release API.
- The downloaded image is checked for the ESP8266 firmware magic byte before flashing.
- Firmware data is streamed directly to the ESP8266 `Update` API.
- Incomplete writes and update finalization errors now report their actual ESP8266 updater error code.
- Browser `/api/state` polling is paused while an OTA installation is running, avoiding expected connection timeouts and reducing ESP8266 socket/heap pressure during TLS download and flashing.

## Why this change was necessary

The previous v0.1.3 workaround disabled HTTP connection reuse and enabled forced redirect following in `ESPhttpUpdate`. Real-device testing showed that this did not solve the GitHub release download failure.

The ESP8266 `ESPhttpUpdate` implementation maps any unexpected HTTP status to the generic `HTTP_UE_SERVER_WRONG_HTTP_CODE` error (`-104`). That hid the actual response from GitHub or its release CDN.

v0.1.5 therefore handles the HTTP stages directly and only uses the lower-level ESP8266 `Update` API for writing the verified firmware stream.

## OTA flow in v0.1.5

1. Read the latest GitHub Release metadata.
2. Select the exact `firmware.bin` asset.
3. Request its `browser_download_url` with redirects disabled.
4. Log the actual GitHub HTTP status.
5. Read the `Location` header from the GitHub redirect.
6. Require the redirect target to use GitHub's release asset CDN.
7. Open a fresh TLS connection to the CDN.
8. Require HTTP 200 and a positive Content-Length.
9. Compare Content-Length with the Release API asset size.
10. Verify that the image starts with the ESP8266 firmware magic byte (`0xE9`).
11. Stream the image directly to `Update.writeStream()`.
12. Finalize the update and reboot only after a successful web response.

## Diagnostics

If a future OTA request fails, the system log now reports messages such as:

- `GitHub asset response HTTP 302`
- `Firmware download returned HTTP <code>`
- `Firmware size mismatch: expected <n>, received <n>`
- `Downloaded file is not a valid ESP8266 firmware image`
- `Firmware write incomplete: <written>/<expected> bytes (error <code>)`
- `Firmware finalization failed: <code>`

This makes the OTA path diagnosable without guessing which HTTP response caused the failure.

## Existing functionality retained

- Right-aligned clock display without a leading zero for single-digit hours
- One-second blinking TM1637 center colon
- Clock-only mode without an external API
- Runtime-configurable TM1637 CLK and DIO GPIOs
- External `/api/current-values` polling and RAM last-known-good cache
- NTP synchronization and POSIX timezone handling
- EEPROM-backed persistent settings
- German and English responsive web UI
- GitHub Actions ESP8266 build and release workflow

## Upgrade note

Because the OTA downloader itself is being replaced, a device running v0.1.3 cannot reliably install v0.1.5 through the currently failing OTA path. Install v0.1.5 once by USB/serial without erasing the flash so EEPROM settings remain intact.

After v0.1.5 is installed, use the next release to validate this new OTA implementation end to end.

## Platform

- ESP8266
- Arduino Framework
- PlatformIO
- Wemos/Lolin D1 mini compatible target
- TM1637 4-digit display

## OTA repository

`syschelle/espDisplay`

The tagged GitHub Actions workflow builds and publishes:

- `firmware.bin`
- `firmware.bin.sha256`

## License

Apache License 2.0
