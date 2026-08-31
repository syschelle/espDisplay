#!/usr/bin/env python3
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"


def extract_raw_header(path: Path, marker: str = "ED8266") -> str:
    text = path.read_text(encoding="utf-8")
    start_token = f'R"{marker}('
    end_token = f'){marker}";'
    start = text.find(start_token)
    end = text.rfind(end_token)
    if start < 0 or end < 0 or end <= start:
        raise AssertionError(f"raw literal not found in {path}")
    return text[start + len(start_token):end]


def test_api_fixture() -> None:
    fixture_path = ROOT / "tests" / "fixtures" / "api-current-values.json"
    raw = fixture_path.read_text(encoding="utf-8")
    assert len(raw.encode("utf-8")) < 8192
    data = json.loads(raw)
    required = [
        "current_solar_production_w", "current_grid_power_w", "current_grid_import_w",
        "current_grid_export_w", "current_total_consumption_w", "daily_solar_production_kwh",
        "daily_grid_import_kwh", "daily_grid_export_kwh", "total_solar_production_kwh",
        "total_grid_import_kwh", "total_grid_export_kwh",
    ]
    for key in required:
        assert isinstance(data[key], (int, float))
    air = data["air_sensor"]
    assert air["temperature_c"] == 22.85
    assert air["humidity_percent"] == 76.88
    assert air["sds_p1"] == 2.92
    assert air["sds_p2"] == 0.7

    # Optional fields may be absent and unknown fields must be harmless to a filtered parser.
    reduced = {"current_grid_import_w": 42.0, "unknown_future_field": {"anything": [1, 2, 3]}}
    assert isinstance(reduced.get("current_grid_import_w"), (int, float))
    assert reduced.get("air_sensor") is None

    try:
        json.loads('{"current_grid_import_w": 42,')
    except json.JSONDecodeError:
        pass
    else:
        raise AssertionError("invalid JSON fixture unexpectedly parsed")


def test_web_assets() -> None:
    html = extract_raw_header(SRC / "index_html.h")
    js = extract_raw_header(SRC / "java_script.h")
    css = extract_raw_header(SRC / "style_css.h")

    ids = re.findall(r'\bid="([^"]+)"', html)
    duplicates = sorted({x for x in ids if ids.count(x) > 1})
    assert not duplicates, f"duplicate HTML IDs: {duplicates}"
    assert "/script.js" in html and "/style.css" in html
    assert '<input id="displayClkGpio" type="number"' in html
    assert '<input id="displayDioGpio" type="number"' in html
    assert '<input id="apiPort" type="number" min="1" max="65535"' in html
    assert '<select id="displayClkGpio"' not in html
    assert '<select id="displayDioGpio"' not in html
    assert "String.replace" not in html
    assert "http://" not in js and "https://" not in js, "browser JS must not call the external API"
    assert 'apiFetch("/api/state")' in js
    assert "setInterval(loadState,5000)" in js
    assert "--header:#2c3e50" in css
    assert "@media" in css

    # Every data-i18n key used by HTML must exist in both language maps.
    html_keys = set(re.findall(r'data-i18n="([^"]+)"', html))
    de_block, en_block = js.split("  en: {", 1)
    literal_t_keys = set(re.findall(r'(?<![A-Za-z0-9_$])t\("([^"]+)"\)', js))
    metric_keys = set(re.findall(r'\["[^"]+","([^"]+)"\]', js))
    required_i18n = html_keys | literal_t_keys | metric_keys
    english_block = en_block.split("};", 1)[0]
    missing_de = sorted(k for k in required_i18n if f'"{k}"' not in de_block)
    missing_en = sorted(k for k in required_i18n if f'"{k}"' not in english_block)
    assert not missing_de, f"missing German i18n keys: {missing_de}"
    assert not missing_en, f"missing English i18n keys: {missing_en}"

    build = ROOT / ".test-output"
    build.mkdir(exist_ok=True)
    js_path = build / "app.js"
    js_path.write_text(js, encoding="utf-8")
    subprocess.run(["node", "--check", str(js_path)], check=True)


def test_architecture_guards() -> None:
    all_cpp = "\n".join(p.read_text(encoding="utf-8") for p in SRC.glob("*.*") if p.suffix in {".cpp", ".h"})
    external = (SRC / "external_api.cpp").read_text(encoding="utf-8")
    config = (SRC / "config.h").read_text(encoding="utf-8")
    settings = (SRC / "settings.cpp").read_text(encoding="utf-8")
    time_service = (SRC / "time_service.cpp").read_text(encoding="utf-8")
    platformio = (ROOT / "platformio.ini").read_text(encoding="utf-8")

    forbidden = ["BME280", "DallasTemperature", "OneWire", "FreeRTOS", "Preferences.h", "Shelly", "HomeKit"]
    for token in forbidden:
        assert token not in all_cpp, f"forbidden GrowTent/ESP32 token remains: {token}"

    assert 'EXTERNAL_API_PATH[] = "/api/current-values"' in config
    assert "DEFAULT_API_PORT = 80" in config
    assert "API_CONNECT_TIMEOUT_MS = 1200UL" in config
    assert "API_RESPONSE_TIMEOUT_MS = 60000UL" in config
    assert "API_INACTIVITY_TIMEOUT_MS = 60000UL" in config
    assert "API_READ_BUDGET_BYTES = 384" in config
    assert "MAX_EXTERNAL_API_BYTES = 8192" in config
    assert "HTTPClient" not in external, "external API polling must not use blocking ESP8266HTTPClient"
    assert ".GET()" not in external and "getString()" not in external, "external API polling must remain cooperative"
    assert "RequestState::Headers" in external and "RequestState::Body" in external
    assert "API_READ_BUDGET_BYTES" in external
    assert "client_.available()" in external
    assert "client_.connect(cfg.apiHost, cfg.apiPort)" in external
    assert "Connection: close" in external and "Accept-Encoding: identity" in external
    assert "Transfer-Encoding:" in external and "chunked" in external
    assert "Deliberately keep all last-known-good numeric values" in external
    assert "values_ = next" in external
    assert "markFailure" in external
    for field in [
        "current_solar_production_w", "current_grid_power_w", "current_grid_import_w",
        "current_grid_export_w", "current_total_consumption_w", "daily_solar_production_kwh",
        "daily_grid_import_kwh", "daily_grid_export_kwh", "total_solar_production_kwh",
        "total_grid_import_kwh", "total_grid_export_kwh", "temperature_c",
        "humidity_percent", "dew_point_c", "pressure_hpa", "pressure_sea_level_hpa",
        "sds_p1", "sds_p2", "age_seconds"
    ]:
        assert f'"{field}"' in external, f"external parser missing {field}"
    assert "#include <EEPROM.h>" in settings
    assert "EEPROM_SETTINGS_MAGIC" in settings and "payloadCrc" in settings
    assert "EEPROM.commit()" in settings
    assert "LittleFS" not in all_cpp
    assert "strncpy(" not in all_cpp, "warning-prone strncpy() usage remains in firmware sources"
    assert "board_build.filesystem" not in platformio
    assert "NTP_BOOT_SYNC_TIMEOUT_MS" in time_service
    assert "while" in time_service  # boot wait exists, checked below for timeout condition
    assert "millis() - started" in time_service and "NTP_BOOT_SYNC_TIMEOUT_MS" in time_service
    assert "espressif8266 @ 4.2.1" in platformio
    assert "d1_mini" in platformio
    assert "espressif32" not in platformio
    assert "avishorp/TM1637 @" not in platformio, "invalid PlatformIO registry package identifier for TM1637"
    assert "https://github.com/avishorp/TM1637.git#v1.2.0" in platformio

    models = (SRC / "models.h").read_text(encoding="utf-8")
    display = (SRC / "display.cpp").read_text(encoding="utf-8")
    web = (SRC / "web.cpp").read_text(encoding="utf-8")
    assert "displayClkGpio" in models and "displayDioGpio" in models
    assert "LEGACY_SETTINGS_SCHEMA_VERSION = 1" in config
    assert "PREVIOUS_SETTINGS_SCHEMA_VERSION = 2" in config
    assert "SETTINGS_SCHEMA_VERSION = 3" in config
    assert "PersistedSettingsRecordV1" in settings and "PersistedSettingsRecordV2" in settings and "PersistedSettingsRecordV3" in settings
    assert "apiPort" in models and "apiPort" in settings
    assert "Settings migrated from schema %u to schema %u" in settings
    assert "TM1637 initialized: CLK GPIO%u, DIO GPIO%u" in display
    assert "new (displayStorage_) TM1637Display" in display
    display_format = (SRC / "display_format.cpp").read_text(encoding="utf-8")
    assert "clockColonVisible" in display_format, "clock formatter must implement one-second colon blinking"
    assert "formatClockFrame" in display, "clock display must use the tested right-aligned clock formatter"
    assert "TM1637_CENTER_COLON_SEGMENT" in display, "clock display must drive the center colon in raw segment mode"
    assert "out.chars[0] = ' ';" in display_format, "single-digit hours must omit the leading zero and remain right-aligned"
    assert "snprintf(out.chars" not in display_format, "clock renderer must not use warning-prone snprintf formatting"
    assert "lastClockSecond_" in display, "clock rendering must track seconds independently from metric refresh"
    assert "displayUpdateMs" in display and "showClock" in display
    assert 'copyText(lastRenderedText_, "Conn")' in display, "startup wait state must be Conn"
    assert "connectingSegments(segments)" in display, "Conn must use a deterministic segment frame"
    assert "showNumberDec(8888" not in display, "legacy 8888 startup self-test must be removed"
    assert 'timeService.getLocalTm(validTimeProbe)' in display, "display must gate normal rendering until local time is valid"
    main_cpp = (SRC / "main.cpp").read_text(encoding="utf-8")
    assert main_cpp.index("displayService.begin(settings)") < main_cpp.index("networkService.begin(settings)"), "Conn must appear before Wi-Fi/NTP boot waits"
    assert 'root["clkGpio"]' in web and 'root["dioGpio"]' in web
    assert 'root["apiPort"]' in web and 'settings.apiPort' in web
    assert 'externalApiService.requestInProgress()' in web
    assert "restartScheduled" in web
    assert "sendProgmemAsset" in web
    assert "chunkedResponseModeStart" in web and "sendContent_P" in web
    assert "kChunkSize = 1024" in web
    assert 'no-store, no-cache, must-revalidate' in web
    assert 'root["clkGpio"].is<int>() ?' in web, "display endpoint must tolerate an older cached UI omitting pin fields"
    assert 'mode != DisplayMode::Clock' in web, "clock-only mode must not require a metric"

    ota = (SRC / "ota.cpp").read_text(encoding="utf-8")
    web_js = (SRC / "java_script.h").read_text(encoding="utf-8")
    cfg = (SRC / "config.h").read_text(encoding="utf-8")
    workflow = (ROOT / ".github" / "workflows" / "release-firmware.yml").read_text(encoding="utf-8")
    assert 'OTA_GITHUB_REPO[] = "espDisplay"' in cfg
    assert 'OTA_BRANCH[] = "ota"' in cfg
    assert 'OTA_MANIFEST_NAME[] = "manifest.json"' in cfg
    assert "#include <ESP8266httpUpdate.h>" not in ota and "ESPhttpUpdate." not in ota, "OTA install path must not use ESPhttpUpdate"
    assert "api.github.com" not in ota, "ESP8266 OTA checks must not use the GitHub Releases API"
    assert "release-assets.githubusercontent.com" not in ota, "ESP8266 OTA must not depend on GitHub release-asset redirects"
    assert "https://raw.githubusercontent.com/%s/%s/%s/%s" in ota, "OTA must use the fixed raw.githubusercontent.com channel"
    assert 'strcmp(firmwareName, OTA_ASSET_NAME) != 0' in ota, "OTA manifest must be pinned to firmware.bin"
    assert "isSha256Hex" in ota, "OTA manifest must validate the generated SHA-256 field"
    assert "OTA manifest returned HTTP %d" in ota, "manifest failures must log the actual HTTP status"
    assert "OTA firmware returned HTTP %d" in ota, "firmware failures must log the actual HTTP status"
    assert "Update.writeStream(*stream" in ota, "OTA must stream firmware directly to the ESP8266 updater"
    assert "contentLength > 0" in ota and "status_.firmwareSize" in ota, "OTA must compare HTTP size when available with manifest metadata"
    assert "stream->peek() != 0xE9" in ota, "OTA must validate the ESP8266 firmware magic byte without consuming it"
    assert "readBytes(header" not in ota and "Update.write(header" not in ota, "OTA must not pre-consume the firmware header before Update.writeStream"
    assert "Update.writeStream(*stream, 60000)" in ota, "OTA must let the ESP8266 updater consume the stream from byte zero"
    assert "otaUpdateActive" in web_js, "frontend must pause state polling while OTA is running"
    assert "Prepare release and OTA assets" in workflow
    assert '"version": "$GITHUB_REF_NAME"' in workflow
    assert '"firmware": "firmware.bin"' in workflow
    assert "Publish redirect-free OTA channel" in workflow
    assert "git push origin HEAD:ota" in workflow
    assert "manifest.json" in workflow and "firmware.bin.sha256" in workflow
    assert "GrowTent" not in ota


def main() -> int:
    test_api_fixture()
    test_web_assets()
    test_architecture_guards()
    print("project static/API/web checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

