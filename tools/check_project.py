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
    assert data["timezone"] == "Europe/Berlin"
    assert isinstance(data["last_measurement_at"], str)
    assert data["air_sensor"]["temperature_c"] == 22.85

    # The real API may still contain many unrelated values. v0.1.20 deliberately
    # ignores them and only consumes air_sensor.temperature_c plus timestamp metadata.
    assert "current_grid_import_w" in data
    assert "humidity_percent" in data["air_sensor"]

    reduced = {
        "timezone": "Europe/Berlin",
        "last_measurement_at": "2026-08-31T15:45:56+00:00",
        "air_sensor": {"temperature_c": 21.5},
        "unknown_future_field": {"anything": [1, 2, 3]},
    }
    assert reduced["air_sensor"]["temperature_c"] == 21.5

    try:
        json.loads('{"air_sensor":{"temperature_c":22.5},')
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
    assert '<input id="displayUpdateMs" type="number" min="250" max="200000"' in html
    assert '<input id="apiValueDisplayMs" type="number" min="100" max="200000"' in html
    assert '<select id="displayClkGpio"' not in html
    assert '<select id="displayDioGpio"' not in html
    assert "String.replace" not in html
    assert "/api/current-values" not in js, "browser JS must never call the configured external API directly"
    assert 'data-page="values"' not in html and 'id="values"' not in html, "Measurements page must be removed"
    assert 'id="selectedMetric"' not in html and "selectedMetric" not in js, "metric selection must be removed"
    assert "https://raw.githubusercontent.com/syschelle/espDisplay/ota/" in js, "browser-assisted OTA must use the pinned ota branch"
    assert 'apiFetch("/api/state")' in js
    assert 'id="apiServer" class="single-line-value"' in html, "API server value must stay single-line"
    assert 'api.server' in js and 'api.endpoint' not in js, "Status UI must show compact host:port instead of the full fixed endpoint"
    assert '.single-line-value{white-space:nowrap' in css, "single-line status value CSS missing"
    assert "setInterval(loadState,5000)" in js
    assert "function fmtDateTime(value,timeZone)" in js
    assert 'fmtDateTime(state.external.lastMeasurementAt,state.external.timezone)' in js
    assert '"state.noMeasurement":"Noch keine Messung"' in js
    assert '"state.noMeasurement":"No measurement yet"' in js
    assert "--header:#2c3e50" in css
    assert "@media" in css

    # Every data-i18n key used by HTML must exist in both language maps.
    html_keys = set(re.findall(r'data-i18n="([^"]+)"', html))
    de_block, en_block = js.split("  en: {", 1)
    literal_t_keys = set(re.findall(r'(?<![A-Za-z0-9_$])t\("([^"]+)"\)', js))
    required_i18n = html_keys | literal_t_keys
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

    sha_start = js.index("function sha256Hex(bytes){")
    sha_end = js.index("function validateOtaManifest", sha_start)
    sha_test = js[sha_start:sha_end] + "\nconst value = new TextEncoder().encode(\"abc\");\nif (sha256Hex(value) !== \"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\") { throw new Error(\"browser SHA-256 implementation failed\"); }\n"
    subprocess.run(["node", "-e", sha_test], check=True)


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
    assert "MAX_DISPLAY_UPDATE_MS = 200000UL" in config, "display refresh interval must allow up to 200 seconds"
    assert "MIN_API_VALUE_DISPLAY_MS = 100UL" in config and "MAX_API_VALUE_DISPLAY_MS = 200000UL" in config
    assert "HTTPClient" not in external, "external API polling must not use blocking ESP8266HTTPClient"
    assert ".GET()" not in external and "getString()" not in external, "external API polling must remain cooperative"
    assert "RequestState::Headers" in external and "RequestState::Body" in external
    assert "API_READ_BUDGET_BYTES" in external
    assert "client_.available()" in external
    assert "client_.connect(cfg.apiHost, cfg.apiPort)" in external
    assert "Connection: close" in external and "Accept-Encoding: identity" in external
    assert "Transfer-Encoding:" in external and "chunked" in external
    assert "Deliberately keep all last-known-good numeric values" in external
    assert "char bodyBuffer_[8193]" not in (SRC / "external_api.h").read_text(encoding="utf-8"), "large API response buffer must not remain permanently allocated in BSS"
    assert "char* bodyBuffer_ = nullptr" in (SRC / "external_api.h").read_text(encoding="utf-8")
    assert "ensureBodyCapacity" in external and "releaseBodyBuffer" in external
    assert "void ExternalApiService::suspend()" in external and "void ExternalApiService::resume()" in external
    assert "External API request paused for OTA" in external
    assert "values_ = next" in external
    assert "markFailure" in external
    assert '"temperature_c"' in external, "external parser must consume air temperature"
    assert '"timezone"' in external and '"last_measurement_at"' in external
    for removed_field in [
        "current_solar_production_w", "current_grid_power_w", "current_grid_import_w",
        "current_grid_export_w", "current_total_consumption_w", "daily_solar_production_kwh",
        "daily_grid_import_kwh", "daily_grid_export_kwh", "total_solar_production_kwh",
        "total_grid_import_kwh", "total_grid_export_kwh", "humidity_percent",
        "dew_point_c", "pressure_hpa", "pressure_sea_level_hpa", "sds_p1", "sds_p2"
    ]:
        assert f'"{removed_field}"' not in external, f"unneeded API field still parsed: {removed_field}"
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
    assert "MetricId" not in models and "selectedMetric" not in models, "runtime metric selection must be removed"
    assert "NumericValue temperatureC" in models, "runtime cache must retain only temperature"
    for removed in ["currentSolarProductionW", "currentGridPowerW", "humidityPercent", "dewPointC", "pm10", "pm25", "AirSensorValues"]:
        assert removed not in models, f"unused runtime measurement remains: {removed}"
    assert "LEGACY_SETTINGS_SCHEMA_VERSION = 1" in config
    assert "DISPLAY_GPIO_SETTINGS_SCHEMA_VERSION = 2" in config
    assert "PREVIOUS_SETTINGS_SCHEMA_VERSION = 3" in config
    assert "DISPLAY_UPDATE_SETTINGS_SCHEMA_VERSION = 4" in config
    assert "SETTINGS_SCHEMA_VERSION = 5" in config
    assert "uint32_t displayUpdateMs = 1000UL" in models
    assert "PersistedSettingsPayloadV4" in settings and "uint32_t displayUpdateMs;" in settings
    assert "PersistedSettingsPayloadV5" in settings and "uint32_t apiValueDisplayMs;" in settings
    assert all(token in settings for token in ["PersistedSettingsRecordV1", "PersistedSettingsRecordV2", "PersistedSettingsRecordV3", "PersistedSettingsRecordV4", "PersistedSettingsRecordV5"])
    assert "SettingsManager::metricToString" not in settings and "SettingsManager::metricFromString" not in settings
    assert "target.legacySelectedMetric = 11U" in settings, "legacy EEPROM metric slot should normalize to old temperature id"
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
    assert "alternateClockVisible" in display_format and "alternateDisplayShowsClock" in display, "alternate mode must use independently configurable clock/API phases"
    assert "cfg.apiValueDisplayMs" in display, "alternate mode must honor the configured API-value display duration"
    assert "degreeSymbolSegment" in display_format and "degreeSuffix" in display, "temperature must render a dedicated degree suffix"
    assert "formatTemperatureFrame" in display_format and "lroundf" in display_format, "air temperature must be rounded to an integer"
    assert 'copyText(lastRenderedText_, "Conn")' in display, "startup wait state must be Conn"
    assert "connectingSegments(segments)" in display, "Conn must use a deterministic segment frame"
    assert "showNumberDec(8888" not in display, "legacy 8888 startup self-test must be removed"
    assert 'timeService.getLocalTm(validTimeProbe)' in display, "display must gate normal rendering until local time is valid"
    assert 'if (!temperatureAvailable)' in display and 'showClock = true;' in display and 'alternateDisplayShowsClock' in display, "alternate mode must stay on the clock until temperature exists"
    assert 'apiDataStaleForDisplay' in display, "physical display must apply age-based stale indication"
    assert 'degreeSymbolSegment(staleWarning)' in display, "stale temperature must add the warning segment to the degree digit"
    assert 'lastTemperatureStaleWarning_ != staleWarning' in display, "fresh/stale transitions must refresh the display immediately"
    main_cpp = (SRC / "main.cpp").read_text(encoding="utf-8")
    assert main_cpp.index("displayService.begin(settings)") < main_cpp.index("networkService.begin(settings)"), "Conn must appear before Wi-Fi/NTP boot waits"
    assert "ESP.getResetReason()" in main_cpp, "unexpected reboot diagnostics must record the ESP8266 reset reason"
    assert 'const uint32_t updateMs = root["updateMs"].as<uint32_t>();' in web
    assert 'next.displayUpdateMs = updateMs;' in web
    assert 'next.apiValueDisplayMs = apiValueDisplayMs;' in web
    assert 'static_cast<uint16_t>(updateMs)' not in web, "200-second display interval must not be truncated to 16 bits"
    assert 'root["clkGpio"]' in web and 'root["dioGpio"]' in web
    assert 'root["apiPort"]' in web and 'settings.apiPort' in web
    assert 'externalApiService.requestInProgress()' in web
    assert r'\"temperatureC\"' in web
    assert r'\"server\"' in web and r'\"endpoint\"' not in web, "state JSON must expose compact API server only"
    for removed in ["currentSolarProductionW", "currentGridPowerW", "humidityPercent", "dewPointC", "pm10", "pm25", "airSensor"]:
        assert removed not in web, f"unused web state measurement remains: {removed}"
    assert "restartScheduled" in web
    assert "sendProgmemAsset" in web
    assert "chunkedResponseModeStart" in web and "sendContent_P" in web
    assert "kChunkSize = 1024" in web
    assert 'no-store, no-cache, must-revalidate' in web
    assert 'root["clkGpio"].is<int>() ?' in web, "display endpoint must tolerate an older cached UI omitting pin fields"

    ota = (SRC / "ota.cpp").read_text(encoding="utf-8")
    web_js = (SRC / "java_script.h").read_text(encoding="utf-8")
    cfg = (SRC / "config.h").read_text(encoding="utf-8")
    workflow = (ROOT / ".github" / "workflows" / "release-firmware.yml").read_text(encoding="utf-8")
    assert 'OTA_GITHUB_REPO[] = "espDisplay"' in cfg
    assert 'OTA_BRANCH[] = "ota"' in cfg
    assert 'OTA_MANIFEST_NAME[] = "manifest.json"' in cfg
    assert "WiFiClientSecure" not in ota and "HTTPClient" not in ota and "BearSSL" not in ota, "ESP8266 OTA must not open GitHub TLS connections"
    assert "raw.githubusercontent.com" not in ota and "api.github.com" not in ota, "ESP8266 OTA code must remain browser-assisted and network-independent"
    assert "Update.begin(expectedSize, U_FLASH)" in ota, "browser upload must initialize the ESP8266 updater with the manifest size"
    assert "Update.write(data, length)" in ota, "browser upload must stream chunks directly into flash"
    assert "data[0] != 0xE9" in ota, "uploaded firmware must validate the ESP8266 magic byte"
    assert "status_.writtenSize + length > status_.expectedSize" in ota, "OTA chunks must never exceed the expected firmware size"
    assert "uploadedSize != status_.expectedSize" in ota, "OTA completion must require the exact manifest size"
    assert "compareSemver(FW_VERSION, targetVersion) >= 0" in ota, "device must refuse same/older browser OTA images"
    assert '"/api/ota/session"' in web and '"/api/ota/upload"' in web, "web server must expose a short-lived OTA session and local streaming upload endpoint"
    assert "ESP.random()" in web and "otaUploadTokenExpiresMs_" in web, "local OTA upload must require a short-lived one-time session token"
    assert "UPLOAD_FILE_START" in web and "UPLOAD_FILE_WRITE" in web and "UPLOAD_FILE_END" in web, "OTA upload must use ESP8266WebServer streaming callbacks"
    assert "externalApiService.suspend()" in web, "external API must be suspended only when local firmware flashing begins"
    assert "handleOtaCheck" not in web and "otaService.check()" not in web, "update checks must not perform TLS on the ESP8266"
    assert "OTA_HOLD_TIMEOUT_MS" not in (SRC / "web.h").read_text(encoding="utf-8"), "obsolete manifest/check OTA hold must be removed"
    assert "OTA_RAW_BASE" in web_js and "OTA_API_BASE" in web_js, "browser must own GitHub access with a fallback path"
    assert 'fetchOtaBytes("manifest.json")' in web_js and 'fetchOtaBytes("firmware.bin"' in web_js
    assert "sha256Hex(bytes)" in web_js and "Firmware SHA-256 mismatch" in web_js, "browser must verify manifest SHA-256 before uploading"
    assert 'bytes[0]!==0xE9' in web_js, "browser must reject a non-ESP8266 firmware image before local upload"
    assert 'apiFetch("/api/ota/session",{method:"POST"})' in web_js, "browser must obtain a same-origin one-time upload session before flashing"
    assert "FormData" in web_js and "XMLHttpRequest" in web_js and "/api/ota/upload?version=" in web_js and "&token=" in web_js, "browser must upload firmware locally with streaming multipart form data and the one-time session token"
    assert "otaUpdateActive" in web_js, "frontend must pause state polling while OTA flashing is running"
    assert "Prepare release and OTA assets" in workflow
    assert 'NOTES_FILE="RELEASE_NOTES.md"' in workflow, "release workflow must use the canonical current release notes file"
    assert "RELEASE_NOTES_${GITHUB_REF_NAME}.md" not in workflow

    versioned_docs = sorted([p.name for p in ROOT.glob("BUILD_STATUS_v*.md")] + [p.name for p in ROOT.glob("RELEASE_NOTES_v*.md")])
    assert not versioned_docs, f"historical versioned release docs must not remain in repo: {versioned_docs}"
    assert (ROOT / "BUILD_STATUS.md").is_file(), "current BUILD_STATUS.md missing"
    assert (ROOT / "RELEASE_NOTES.md").is_file(), "current RELEASE_NOTES.md missing"
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

