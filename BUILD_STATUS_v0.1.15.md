# espDisplay v0.1.15 - Build / verification status

## Purpose

v0.1.15 makes the **Last measurement / Letzte Messung** field on the Status page easier to read by formatting the API-provided measurement timestamp as an absolute local date and time.

Example:

`01.09.2026, 08:03:27`

The timestamp is converted using the timezone reported by the external API when that timezone is supported by the browser. If it is missing or invalid, the browser-local timezone is used as a safe fallback.

## Verification performed in this environment

- `python tools/check_project.py` - PASS
- JavaScript syntax (`node --check`, invoked by the project check) - PASS
- display format C++ host tests with `-Wall -Wextra -Werror -Wformat-truncation` - PASS
- validation C++ host tests with `-Wall -Wextra -Werror` - PASS
- GitHub Actions workflow YAML parse - PASS
- Date/time formatting smoke test with the real API fixture timestamp and `Europe/Berlin` - PASS
  - German: `31.08.2026, 17:45:56`
  - English: `31/08/2026, 17:45:56`

## PlatformIO

PlatformIO is not installed in this execution environment, therefore the real ESP8266 firmware compile was not executed here.

Run before tagging:

```text
pio run -e d1_mini
```

The GitHub Actions workflow also performs the real `d1_mini` PlatformIO build.
