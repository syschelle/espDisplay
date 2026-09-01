# Build / validation status — espDisplay v0.1.20

## Completed in the artifact environment

- project static/API/web checks: PASS
- JavaScript syntax (`node --check`): PASS
- browser SHA-256 regression vector: PASS (part of project checks)
- display formatter C++ tests with `-Wall -Wextra -Werror -Wformat-truncation`: PASS
- host validation C++ tests with `-Wall -Wextra -Werror`: PASS
- temperature-only API parser architecture guards: PASS
- Measurements-page removal guard: PASS
- metric-selector removal guard: PASS
- EEPROM legacy-layout compatibility guards: PASS
- browser-assisted OTA architecture guards: PASS

## Resource-oriented host check

Approximate host struct sizes:

- v0.1.19 `ExternalValues`: 712 bytes
- v0.1.20 `ExternalValues`: 224 bytes
- reduction: 488 bytes in the host ABI used for this check

This is an architecture comparison, not a substitute for the ESP8266 linker memory report.

## Not available in this environment

PlatformIO (`pio`) is not installed, so the real ESP8266 firmware build was not executed here.

Before tagging the release run:

```text
pio run -e d1_mini
```

The PlatformIO build and GitHub Actions result are the authoritative ESP8266 compile/link checks.
