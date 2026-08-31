# espDisplay v0.1.2 – Release validation status

This source tree is the espDisplay v0.1.2 release source.

## Checks completed in the packaging environment

- Static project/API/web checks: **passed**
- JavaScript syntax check (`node --check` via `tools/check_project.py`): **passed**
- HTML duplicate-ID check: **passed**
- I18N key coverage check: **passed**
- Host C++ display-format tests with warnings as errors: **passed**
- Host C++ validation tests with warnings as errors: **passed**
- Clock-colon even/odd second regression tests: **passed**
- Release metadata consistency (`FW_VERSION = v0.1.2`, repository `syschelle/espDisplay`): **passed**

## ESP8266 build gate

PlatformIO is not installed in the packaging environment, so an ESP8266 firmware build was not claimed here. The GitHub Actions workflow builds `pio run -e d1_mini` on every push to `main` and on the `v0.1.2` tag. The tagged release publishes `firmware.bin` only after that build succeeds.

Recommended release sequence:

1. Push the final source to `main`.
2. Confirm the GitHub Actions build is green.
3. Create and push the annotated `v0.1.2` tag.
4. The tagged workflow builds and publishes `firmware.bin` and `firmware.bin.sha256`.
