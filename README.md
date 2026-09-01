# espDisplay

`espDisplay` is a small ESP8266 application for a four-digit TM1637 display. It shows NTP-synchronized local time and one external API value: **air temperature**.

The project deliberately keeps the ESP8266 workload small. The external API is polled cooperatively, the last successful temperature is kept in RAM, the web UI is served from PROGMEM, and browser-assisted OTA avoids GitHub TLS/BearSSL allocations on the ESP8266.

## Hardware

- ESP8266, Wemos/Lolin D1 mini compatible
- TM1637 four-digit seven-segment display

Default TM1637 pins:

- CLK: GPIO14 / D5
- DIO: GPIO12 / D6

The pins can be changed in the web interface. GPIO6-GPIO11 are rejected because they are used by the ESP8266 flash.

## Build

PlatformIO environment:

```text
d1_mini
```

Build:

```bash
pio run -e d1_mini
```

Upload by USB:

```bash
pio run -e d1_mini -t upload
```

## External API

The API endpoint is always:

```text
http://<host>:<port>/api/current-values
```

Configurable settings:

- host / IP address
- TCP port, default `80`
- polling interval, `5` to `300` seconds

### Consumed API data

Starting with v0.1.20, espDisplay intentionally consumes only:

```json
{
  "timezone": "Europe/Berlin",
  "last_measurement_at": "2026-09-01T12:00:00+00:00",
  "air_sensor": {
    "temperature_c": 23.6,
    "last_success_at": "2026-09-01T12:00:00Z"
  }
}
```

Only `air_sensor.temperature_c` is a measurement used by the firmware. `air_sensor.last_success_at` is used for the Status page when available; root `last_measurement_at` is a compatibility fallback. `timezone` is used to render that timestamp readably in the browser.

All unrelated API fields are ignored, including power, energy, humidity, dew point, pressure, PM10 and PM2.5.

The complete HTTP response still has to be received from the remote server, but ArduinoJson materializes only the temperature and required timestamp metadata.

## Slow API handling

The external API client is a cooperative state machine rather than a blocking `HTTPClient::GET()` call.

- only one request can run at a time
- TCP connect timeout is bounded
- a slow response may take up to 60 seconds
- body processing is limited per main-loop iteration
- the local web server, clock display, NTP and other services continue running while the API responds
- response storage is dynamically allocated only while a response is being received

## Last-known-good temperature

Measurements are RAM-only and are never persisted to EEPROM.

After boot:

- until the first valid temperature arrives, alternate mode stays on the clock
- no empty temperature phase is displayed

During operation:

- the last valid temperature remains available when the API becomes unavailable
- after the greater of 30 seconds or two API polling intervals without fresh data, the degree digit gains a lower horizontal warning segment
- the warning segment disappears immediately after fresh temperature data returns

## Temperature display

The TM1637 temperature is rounded to a whole degree.

Examples:

```text
22.49 °C -> 22°
22.50 °C -> 23°
-4.6 °C  -> -5°
```

The fourth digit is reserved for the degree symbol.

## Display modes

Three display modes are available:

- Temperature only
- Clock only
- Temperature and clock alternating

In alternating mode:

- clock duration is configured in seconds
- temperature duration is configured independently in milliseconds
- the cycle starts with a complete clock phase
- no temperature phase occurs until a valid temperature has been received after boot

Display refresh interval range:

```text
250 ms to 200000 ms
```

Temperature display duration range:

```text
100 ms to 200000 ms
```

## Startup display

Until a valid local time exists, the TM1637 shows only:

```text
Conn
```

No `8888`, `----`, clock or API value is shown before time becomes valid.

## NTP

Defaults:

- server: `de.pool.ntp.org`
- timezone: `CET-1CEST,M3.5.0,M10.5.0/3`

The boot wait is bounded. Later synchronization retries continue in normal operation.

## Web interface

Pages:

- Status
- Display
- System Settings
- System Log
- OTA Update
- Factory Reset

The former **Measurements** page was removed in v0.1.20 because temperature is now the only external measurement consumed by espDisplay.

The UI supports German and English plus light and dark themes.

## Persistent settings

Settings are stored using ESP8266 EEPROM emulation with magic value, schema and CRC32.

The current persistence layout remains schema 5. The historical metric-selection byte remains only as a compatibility slot in the EEPROM record and is ignored by the runtime. When settings are next saved, that legacy byte is normalized to the historical air-temperature value.

Persisted settings include:

- Wi-Fi credentials
- device name
- language/theme
- API host/port/poll interval
- NTP server/timezone
- display enable/brightness
- TM1637 GPIOs
- display mode
- display refresh interval
- clock duration
- temperature duration

Temperature measurements themselves are not persisted.

## OTA

OTA is browser-assisted.

The browser:

1. loads `manifest.json` from the repository's `ota` branch
2. checks whether a newer semantic version exists
3. downloads `firmware.bin`
4. verifies size, ESP8266 magic byte and SHA-256
5. uploads the verified firmware over the local HTTP connection to the ESP8266

The ESP8266 does not open a GitHub HTTPS/BearSSL connection for update checks or firmware downloads.

Release channel:

```text
syschelle/espDisplay / branch ota
```

Generated OTA files:

- `manifest.json`
- `firmware.bin`
- `firmware.bin.sha256`
- `README.md`

## License

Apache License 2.0
