# Build status — espDisplay v0.1.6

## Completed in the generation environment

- Project static/API/web checks: PASS
- JavaScript syntax check: PASS
- HTML ID duplicate check: PASS
- DE/EN i18n key check: PASS
- External API fixture validation: PASS
- Display formatting native C++ tests: PASS
- Display formatting warnings-as-errors host build: PASS
- Host validation native C++ tests: PASS
- OTA architecture guards: PASS
  - explicit GitHub redirect handling retained
  - release-assets.githubusercontent.com host restriction retained
  - actual HTTP status diagnostics retained
  - direct Update.writeStream firmware transfer retained

## v0.1.6 compiler-warning fix

The clock renderer no longer uses `snprintf()` for four-digit time output. It writes the four validated digit positions explicitly and terminates the buffer manually. This removes GCC `-Wformat-truncation` warnings while preserving right-aligned single-digit hours and the one-second blinking colon.

## Required before tagging

A real PlatformIO ESP8266 build cannot be executed in this environment. Before creating tag `v0.1.6`, run:

```text
pio run -e d1_mini
```

Do not tag/release if that build reports warnings or errors.
