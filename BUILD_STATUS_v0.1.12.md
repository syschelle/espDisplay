# espDisplay v0.1.12 build status

## Root cause addressed

Real-device testing after v0.1.11 reported:

`OTA manifest returned HTTP -1`

On the ESP8266 HTTPClient, negative code `-1` is a transport connection failure, not an HTTP status. v0.1.11 introduced a long-lived external API socket and a permanently reserved 8193-byte API body buffer. BearSSL TLS needs a large contiguous heap allocation when opening the OTA HTTPS connection, so the new API resources can starve the OTA manifest connection.

## Changes in v0.1.12

- External API body buffer moved from permanent object/BSS storage to dynamic request-scoped storage.
- Dynamic API response buffer is released on success, failure, timeout, cancellation and OTA suspension.
- Dynamic buffer grows geometrically for chunked/unknown-length responses instead of reallocating for every byte.
- External API service now supports explicit suspend/resume.
- OTA check suspends and aborts any active external API request before starting BearSSL TLS.
- OTA install does the same before firmware download/flashing.
- API polling resumes after an OTA check or failed OTA install.
- API remains suspended after a successful OTA flash while the scheduled reboot is pending.
- Negative HTTPClient OTA errors now include `HTTPClient::errorToString()` detail and current free heap.
- OTA check/install log free heap immediately before the TLS request.
- Last-known-good external values remain in RAM when a request is aborted for OTA.

## Completed in this environment

- Project static/API/web checks: PASS
- JavaScript syntax (`node --check` via project checker): PASS
- Duplicate HTML ID check: PASS
- German/English i18n key check: PASS
- API fixture / missing optional field / invalid JSON checks: PASS
- Native display formatter tests with warnings as errors: PASS
- Native host validation tests with warnings as errors: PASS
- Dynamic API body buffer architecture guard: PASS
- OTA/API suspend/resume architecture guard: PASS
- Negative OTA connection diagnostic guard: PASS
- GitHub Actions workflow YAML parse: PASS

## Requires PlatformIO / real ESP8266

The actual ESP8266 PlatformIO toolchain is not installed in this execution environment. Before tagging v0.1.12, run:

```bash
pio run -e d1_mini
```

The release tag should only be pushed after this build and the GitHub Actions `main` build are green.

## Runtime validation recommended

1. Keep the slow external API configured and active.
2. Confirm the web UI shows an API request in progress.
3. Open OTA and press **Check for updates** while that API request is active.
4. Confirm the system log contains `External API request paused for OTA` when applicable.
5. Confirm the OTA log contains `Manifest check starting (free heap ...)`.
6. Confirm the OTA manifest request succeeds.
7. Confirm API polling resumes after the OTA check.
8. When a newer version exists, start OTA and confirm the active API request is stopped before TLS/flash begins.
9. If a connection still fails, capture the new detailed message containing the HTTPClient reason and free heap.
