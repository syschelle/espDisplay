# espDisplay v0.1.10 build status

## Release scope

v0.1.10 changes the TM1637 startup behavior only:

- initialize the display before Wi-Fi/NTP boot waits
- show exactly `Conn` while no valid local time exists
- remove the previous `8888` startup self-test
- prevent `----`, metric values, or clock frames from replacing `Conn` until local time becomes valid
- preserve all existing display modes after time becomes valid

## Checks executed in the preparation environment

- `python3 tools/check_project.py` — PASS
- display-format host test with `-Wall -Wextra -Werror -Wformat-truncation` — PASS
- validation host test with `-Wall -Wextra -Werror` — PASS
- startup `Conn` segment regression test — PASS
- startup-order guard (display before Wi-Fi/NTP waits) — PASS
- guard that legacy `8888` self-test is absent — PASS

## PlatformIO build

PlatformIO Core is not installed in the preparation environment, so an ESP8266 firmware build was not claimed here.

Before tagging the release, run locally or verify the GitHub Actions `main` build:

```bash
pio run -e d1_mini
```

The tagged workflow must build successfully before the release/OTA assets are published.
