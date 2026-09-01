# espDisplay v0.1.18 - Build / verification status

## Purpose

v0.1.18 adds an independently configurable API-value display duration in milliseconds for alternating clock/API mode.

## Verification performed in this environment

- `python tools/check_project.py` - PASS
- JavaScript syntax (`node --check`, invoked by the project check) - PASS
- Browser SHA-256 known-vector test (`abc`) - PASS
- display format C++ host tests with `-Wall -Wextra -Werror -Wformat-truncation` - PASS
- validation C++ host tests with `-Wall -Wextra -Werror` - PASS
- `settings.cpp` host syntax compile with Arduino/EEPROM stubs and `-Wall -Wextra -Werror` - PASS
- Static regression checks verify:
  - web UI exposes `apiValueDisplayMs`
  - allowed range is `100..200000 ms`
  - alternating timing uses the configured millisecond API phase
  - default API phase remains `1000 ms`
  - EEPROM schema 5 stores the new 32-bit field
  - schema 4 migration remains available
  - legacy schema 1/2/3 migration paths remain available

## PlatformIO

PlatformIO is not installed in this execution environment, therefore the real ESP8266 firmware compile was not executed here.

Run before tagging:

```text
pio run -e d1_mini
```

The GitHub Actions workflow also performs the real `d1_mini` PlatformIO build.
