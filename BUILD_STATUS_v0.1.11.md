# espDisplay v0.1.11 build status

## Completed in this environment

- Project static/API/web checks: PASS
- JavaScript syntax (`node --check` via project checker): PASS
- Duplicate HTML ID check: PASS
- German/English i18n key check: PASS
- API fixture / missing optional field / invalid JSON checks: PASS
- Native display formatter tests with warnings as errors: PASS
- Native host validation tests with warnings as errors: PASS
- Settings schema 1/2 -> 3 migration guards: PASS (static)
- Configurable API port wiring through settings/web/client: PASS (static)
- Blocking `ESP8266HTTPClient` removed from external API polling: PASS (static)
- Cooperative response state machine and per-loop read budget: PASS (static)
- Last-known-good RAM cache preservation on request failure: PASS (static)
- LittleFS/ESP32/FreeRTOS/Preferences dependencies absent: PASS

## Requires PlatformIO / real ESP8266

The actual ESP8266 PlatformIO toolchain is not installed in this execution environment. Before tagging v0.1.11, run:

```bash
pio run -e d1_mini
```

The release tag should only be pushed after this build and the GitHub Actions `main` build are green.

## Runtime validation recommended

1. Configure a non-default API port and confirm the status endpoint shows `http://host:port/api/current-values`.
2. Use a deliberately slow API response (for example 10-20 seconds).
3. During that request, navigate the web UI and confirm the TM1637 clock/colon continues updating.
4. Confirm the API badge shows `Abfrage läuft` / `Request in progress`.
5. Stop the API server and confirm the last valid displayed metric remains visible but is marked stale in the web UI.
6. Confirm no overlapping external API requests are created.
7. Confirm existing v0.1.10 settings migrate and API port defaults to 80.
