# espDisplay v0.1.8 build status

v0.1.8 is a deliberately minimal OTA validation release based on v0.1.7.

## Purpose

- Trigger the tagged GitHub Actions release workflow.
- Create/update branch `ota`.
- Generate `manifest.json` from the actual PlatformIO `firmware.bin`.
- Publish the same `firmware.bin` to the `ota` branch.
- Validate an OTA update from installed v0.1.7 to v0.1.8.

The only firmware behavior change is `FW_VERSION` from `v0.1.7` to `v0.1.8`.

## Checks performed in this package

- Static project/API/web checks: PASS
- JavaScript syntax: PASS
- Native display-format tests: PASS
- Native host-validation tests: PASS
- Strict clock-format warning test: PASS
- GitHub workflow YAML parse: PASS
- Release workflow contains OTA manifest generation: PASS
- Release workflow contains `ota` branch publication: PASS

## Build note

A real PlatformIO ESP8266 build must still complete successfully locally or in GitHub Actions before considering the release build verified.
