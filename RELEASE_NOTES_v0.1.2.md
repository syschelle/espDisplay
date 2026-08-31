# espDisplay v0.1.2

## Added

- One-second blinking of the TM1637 center colon while local time is displayed.
- The colon is lit on even seconds and hidden on odd seconds, producing a clear `HH:MM` clock pulse.
- The same blinking behavior is active in both **Clock only** mode and during the clock phase of **Metric / clock alternation** mode.

## Changed

- Clock rendering now tracks the current second independently from the normal measurement/display refresh interval.
- A configured value refresh such as 4000 ms therefore no longer limits clock-colon animation.
- Clock rendering only writes a new TM1637 frame when the second actually changes, keeping ESP8266 overhead low.
- The web status text remains stable as `HH:MM`; only the physical TM1637 colon blinks.

## Retained from v0.1.1

- Runtime-configurable numeric TM1637 CLK/DIO GPIOs with EEPROM persistence.
- Clock-only operation without a configured external API.
- Periodic external `/api/current-values` polling with RAM last-known-good cache and stale handling.
- German/English responsive web interface with light/dark theme.
- NTP synchronization with configurable server and POSIX timezone.
- Fallback Wi-Fi access point.
- EEPROM-backed settings with schema/CRC protection.
- Chunked PROGMEM web asset delivery and no-cache headers.
- GitHub-based OTA using `syschelle/espDisplay`.
- GitHub Actions build/release workflow for the `d1_mini` environment.

## Upgrade

This release is designed for normal OTA upgrade from **v0.1.1**. No settings schema change is required, so existing Wi-Fi, API, NTP and display settings are retained.
