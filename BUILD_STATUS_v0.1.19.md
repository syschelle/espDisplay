# espDisplay v0.1.19 - Build / verification status

## Purpose

v0.1.19 improves alternating display behavior when API data is initially unavailable and adds a physical stale-data indicator to the temperature degree digit.

## Verification performed in this environment

- `python tools/check_project.py` - PASS
- JavaScript syntax (`node --check`, invoked by the project check) - PASS
- Browser SHA-256 known-vector test (`abc`) - PASS
- display format C++ host tests with `-Wall -Wextra -Werror -Wformat-truncation` - PASS
- validation C++ host tests with `-Wall -Wextra -Werror` - PASS
- Static regression checks verify:
  - alternate mode remains on the clock until the selected API metric is available after boot
  - the alternate cycle starts cleanly only after a usable API metric exists
  - stale indication uses the greater of 30 seconds or two API polling intervals
  - fresh/stale transitions force an immediate metric refresh
  - the normal degree symbol remains `0x63`
  - the stale degree symbol adds TM1637 segment D and becomes `0x6B`
  - no EEPROM schema change is required

## PlatformIO

PlatformIO is not installed in this execution environment, therefore the real ESP8266 firmware compile was not executed here.

Run before tagging:

```text
pio run -e d1_mini
```

The GitHub Actions workflow also performs the real `d1_mini` PlatformIO build.
