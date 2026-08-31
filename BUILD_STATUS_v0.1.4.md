# Build status — espDisplay v0.1.4

## Completed in preparation environment

- Project static/API/web checks: PASS
- JavaScript syntax check: PASS
- TM1637 metric formatting host tests: PASS
- TM1637 clock formatting host tests: PASS
- Single-digit hour right-alignment regression tests: PASS
- One-second colon blink regression tests: PASS
- Host/IP validation host tests: PASS
- OTA redirect regression guard retained: PASS
- Release ZIP integrity: PASS after packaging

## External verification required

A real PlatformIO ESP8266 build cannot be executed in this environment because PlatformIO is not available here. Before tagging v0.1.4, run the GitHub Actions build or locally:

```text
pio run -e d1_mini
```

The intended OTA verification path is **v0.1.3 -> v0.1.4**. Do not consider OTA verified until the tagged v0.1.4 firmware has been downloaded, flashed and rebooted successfully on the device.
