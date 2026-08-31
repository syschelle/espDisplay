# espDisplay v0.1.4

## Changed

- Single-digit hours on the TM1637 clock are now displayed without a leading zero.
- The clock remains right-aligned on the four-digit display.
- Minutes always retain two digits.
- Examples:
  - `09:05` is physically rendered as `[blank]9:05`.
  - `00:05` is physically rendered as `[blank]0:05`.
  - `12:34` continues to use all four digits.
- The one-second blinking center colon introduced previously is retained.

## OTA verification release

v0.1.4 is intentionally a small functional release suitable for validating the corrected GitHub OTA download path introduced in v0.1.3.

Expected test path:

1. Install v0.1.3 via USB/serial if not already installed.
2. Publish v0.1.4 with the GitHub Actions workflow.
3. Check for updates from the v0.1.3 web interface.
4. Install v0.1.4 over OTA.
5. Confirm reboot and firmware version v0.1.4.
6. Confirm the new no-leading-zero clock display.

## Retained

- One-second blinking TM1637 center colon in clock mode.
- GitHub OTA redirect handling with connection reuse disabled.
- Runtime-configurable TM1637 CLK/DIO GPIOs.
- Clock-only operation without an external API.
- External API polling with RAM last-known-good cache and stale handling.
- NTP synchronization with configurable server and POSIX timezone.
- EEPROM-backed settings with schema and CRC32 protection.
- Responsive German/English web interface with light/dark theme.
- Chunked PROGMEM web asset delivery.
- GitHub Actions build/release workflow for the `d1_mini` environment.

There is no settings schema change in this release. Existing settings are retained.
