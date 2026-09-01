# espDisplay v0.1.21

espDisplay v0.1.21 improves the API Status card and simplifies release-document handling in the repository.

## Status page

The API **Server** row now displays only the actual server address:

`host:port`

Example:

`192.168.178.211:5173`

The fixed protocol/path text is no longer rendered inside the Status row:

`http://.../api/current-values`

This keeps the server value on one line in normal desktop/tablet layouts and gives the remaining API status values more visual space.

When a long hostname is shortened with ellipsis, the complete `host:port` value remains available as the browser tooltip.

For unusually long host names the value stays on one line and uses ellipsis instead of wrapping the Status row to multiple lines.

## Local state API

`/api/state` now exposes the compact API server value as `api.server` rather than returning the complete fixed endpoint string.

The external API itself is unchanged and remains:

`http://<host>:<port>/api/current-values`

## Repository cleanup

The repository no longer accumulates versioned build/release Markdown files.

Only the current release documents remain:

- `BUILD_STATUS.md`
- `RELEASE_NOTES.md`

Historical files matching these patterns are removed:

- `BUILD_STATUS_v*.md`
- `RELEASE_NOTES_v*.md`

The GitHub release workflow now reads the canonical current file:

`RELEASE_NOTES.md`

Published GitHub Releases remain the historical release record.

Project checks now fail if historical versioned build/release Markdown files are reintroduced into the repository.

## Existing functionality retained

- temperature-only external API parsing
- configurable API host and TCP port
- cooperative slow external API polling
- RAM-only last-known-good temperature cache
- stale temperature warning segment
- readable last-measurement date/time
- `Conn` startup display until valid local time exists
- rounded whole-degree TM1637 temperature with degree symbol
- configurable temperature display duration in milliseconds
- configurable clock display duration in seconds
- configurable TM1637 refresh interval up to 200 seconds
- runtime-configurable TM1637 GPIOs
- NTP synchronization
- EEPROM-backed settings
- German and English responsive web UI
- light and dark themes
- Wi-Fi fallback access point
- browser-assisted OTA
- reset-reason logging

## Settings

There is no EEPROM schema change in v0.1.21.

Existing v0.1.20 settings are retained.

## OTA channel

Repository: `syschelle/espDisplay`

Branch: `ota`

Files:

- `manifest.json`
- `firmware.bin`
- `firmware.bin.sha256`

## Platform

- ESP8266
- Arduino Framework
- PlatformIO
- Wemos/Lolin D1 mini compatible target
- TM1637 four-digit seven-segment display

## License

Apache License 2.0
