# espDisplay v0.1.8

espDisplay v0.1.8 is a deliberately minimal validation release for the redirect-free OTA channel introduced in v0.1.7.

## Purpose

This release intentionally contains no new user-facing functionality beyond the firmware version bump from v0.1.7 to v0.1.8.

Its purpose is to validate the complete OTA publication and installation path:

1. A `v0.1.8` tag starts GitHub Actions.
2. PlatformIO builds the `d1_mini` firmware.
3. The workflow creates `firmware.bin` and `firmware.bin.sha256`.
4. The workflow generates `manifest.json` from the actual built firmware.
5. The workflow creates/updates the normal GitHub Release.
6. The workflow creates/updates branch `ota` with the manifest and firmware.
7. An ESP8266 running v0.1.7 detects v0.1.8 from `ota/manifest.json`.
8. The device downloads `ota/firmware.bin` directly from `raw.githubusercontent.com` and installs it.

## Expected OTA branch contents

After the tagged workflow completes successfully, branch `ota` should contain:

- `manifest.json`
- `firmware.bin`
- `firmware.bin.sha256`
- `README.md`

The manifest should report version `v0.1.8` and the actual size and SHA-256 of the firmware built by GitHub Actions.

## OTA test

Recommended validation sequence:

1. Keep v0.1.7 installed on the ESP8266.
2. Push v0.1.8 to `main`.
3. Verify the normal GitHub Actions build is green.
4. Push tag `v0.1.8`.
5. Verify the tagged workflow is green.
6. Confirm branch `ota` now exists and contains `manifest.json` and `firmware.bin`.
7. Open the OTA page on the v0.1.7 device.
8. Check for an update.
9. Confirm v0.1.8 is detected.
10. Start the update.
11. Confirm the firmware downloads, flashes and reboots.
12. Confirm the web UI reports v0.1.8 afterward.

## Existing functionality retained

All v0.1.7 functionality remains unchanged, including:

- Redirect-free OTA channel using `raw.githubusercontent.com`
- Runtime-configurable TM1637 CLK and DIO GPIOs
- Right-aligned clock display without a leading hour zero
- One-second blinking center colon
- Clock-only mode without an external API
- External `/api/current-values` polling
- NTP synchronization
- EEPROM-backed persistent settings
- German and English responsive web interface
- GitHub Actions ESP8266 build/release workflow

## Settings

There is no settings schema change in v0.1.8. Existing EEPROM settings are retained.

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
