# espDisplay v0.1.6

espDisplay v0.1.6 removes the remaining GCC clock-formatting warning while retaining the new diagnostic OTA implementation introduced in v0.1.5.

## Fixed

- Removed the `snprintf()` based clock frame formatter that could trigger `-Wformat-truncation`.
- Clock digits are now written directly into the four physical TM1637 positions.
- The output buffer is explicitly NUL-terminated.
- Added a regression guard preventing the warning-prone clock `snprintf()` implementation from returning.
- Native clock formatter tests are compiled with warnings treated as errors.

## Clock display retained

- Single-digit hours have no leading zero.
- Time remains right-aligned.
- Minutes always use two digits.
- The center colon continues to blink once per second.

Examples:

- `09:05` -> `[blank]9:05`
- `00:05` -> `[blank]0:05`
- `12:34` -> `12:34`

## OTA implementation retained from v0.1.5

- `ESPhttpUpdate` is not used for firmware installation.
- GitHub release-asset redirects are resolved explicitly.
- The signed CDN target is restricted to `release-assets.githubusercontent.com`.
- Actual HTTP response codes are logged.
- Firmware Content-Length is compared with GitHub asset metadata.
- ESP8266 firmware magic byte `0xE9` is verified before flashing.
- Firmware is streamed directly through `Update.writeStream()`.
- Browser `/api/state` polling is paused during OTA.

## Settings

There is no settings schema change in v0.1.6. Existing EEPROM settings are retained.

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
