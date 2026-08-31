# espDisplay v0.1.1

First release under the final project and repository name **espDisplay**.

## Added

- Runtime-configurable TM1637 CLK and DIO pins in the web interface using free numeric GPIO input fields (no fixed dropdown list).
- D1 mini pin labels (`D0`…`D8`, `RX`, `TX`) together with the real GPIO number.
- Warnings in the UI for ESP8266 boot-strap and UART pins.
- Automatic restart when the TM1637 pin mapping changes.
- TM1637 boot self-test showing `8888` for about one second.
- Display pin mapping in the local status API and Status page.
- Settings schema v2 with migration from the v0.1.0 EEPROM layout.

## Changed

- Project identity changed from the development name `EnergyDisplay8266` to `espDisplay`.
- OTA release channel changed to `syschelle/espDisplay`.
- Default PlatformIO target changed to the Wemos/Lolin D1 mini compatible `d1_mini` board.
- GitHub Actions now builds on pushes and pull requests to `main`; tagged builds additionally publish `firmware.bin` and `firmware.bin.sha256` as GitHub Release assets.
- The old factory-default device name is migrated to `espDisplay`; custom device names are preserved.

## Preserved during migration

An upgrade from the v0.1.0 EEPROM settings layout preserves:

- Wi-Fi SSID and password
- API host and polling interval
- NTP server and timezone
- language and theme
- selected metric and display mode
- display enable state, brightness and refresh intervals

The new TM1637 pin settings start with the previous defaults `D5/GPIO14` (CLK) and `D6/GPIO12` (DIO) until changed in the web interface.

## Important OTA transition note

The already-built v0.1.0 development firmware points to the old OTA repository name `syschelle/EnergyDisplay8266`. It therefore cannot discover a release hosted only in `syschelle/espDisplay`.

**v0.1.1 must be installed once by USB/serial (or through a transition release in the old repository).** After v0.1.1 is installed, future versions can update normally through the `syschelle/espDisplay` OTA channel.

## Previous v0.1.0 functionality retained

- Periodic `/api/current-values` polling with RAM last-known-good cache and stale state.
- Responsive German/English light/dark web interface.
- NTP synchronization with Europe/Berlin POSIX timezone default.
- Fallback access point and persistent EEPROM settings.
- Four-digit formatting for W, kWh and optional external air-sensor values.
- GitHub-based OTA version check and update mechanism.

- Improved clock-only mode: external API is optional, metric controls are hidden, and the status hero shows local NTP time.

### Web asset cache reliability

- Embedded CSS and JavaScript are now served with `no-store`/`no-cache` headers.
- This prevents an older browser-cached frontend from being mixed with a newer firmware API after USB or OTA updates.
- Fixes display-settings saves failing with `Missing or invalid display settings` after upgrading from an older firmware.


### Web-transfer and settings reliability fix

- Serve embedded HTML, CSS and JavaScript in small HTTP/1.1 chunks to avoid incomplete ESP8266 `send_P()` transfers and browser `ERR_CONTENT_LENGTH_MISMATCH` errors.
- Keep static web assets explicitly non-cacheable across firmware changes.
- Make the display settings endpoint backward-compatible when an older cached UI omits the new CLK/DIO fields.
- Clock-only mode no longer requires a display metric in the POST payload.
- Return precise field/range validation messages for display settings instead of a generic HTTP 400 error.
