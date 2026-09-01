# espDisplay v0.1.14 - Build / verification status

## Purpose

v0.1.14 addresses the OTA firmware TLS connection failure observed after the cooperative slow external API poller was introduced.

Observed device error:

`OTA firmware connection failed -1: connection failed (heap 2816)`

## Root cause addressed

v0.1.13 suspended the external API only while checking the OTA manifest. Immediately after a successful check it resumed API polling. A slow API request could then open a TCP connection and allocate/free response memory before the user pressed **Install update**. On the ESP8266 this can leave too little suitable heap for the second BearSSL connection used to download `firmware.bin`.

v0.1.14 keeps an OTA-exclusive hold active when an update is available and refreshes the manifest again immediately before the firmware download while the external API remains suspended.

## Verification performed in this environment

- `python tools/check_project.py` - PASS
- JavaScript syntax (`node --check`) - PASS
- display format C++ host tests with `-Wall -Wextra -Werror -Wformat-truncation` - PASS
- validation C++ host tests with `-Wall -Wextra -Werror` - PASS
- OTA regression guards for exclusive API hold - PASS
- OTA diagnostic guards for free heap / largest free block / fragmentation - PASS

## PlatformIO

PlatformIO is not installed in this execution environment, therefore the real ESP8266 firmware compile was not executed here.

Run before tagging:

```text
pio run -e d1_mini
```

The GitHub Actions workflow also performs the real `d1_mini` PlatformIO build.
