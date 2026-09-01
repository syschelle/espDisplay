#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"ED8266(
<!DOCTYPE html>
<html lang="de" data-theme="light">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <meta name="color-scheme" content="light dark">
  <title>espDisplay</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <header class="header">
    <button class="hamburger" id="hamburgerBtn" type="button" aria-controls="sidebar" aria-expanded="false" data-i18n="a11y.menu" data-i18n-attr="aria-label">☰</button>
    <div class="title" id="deviceTitle">espDisplay</div>
    <div class="datetime">
      <div id="headerDate">--</div>
      <div id="headerTime">--:--</div>
    </div>
  </header>

  <div class="layout">
    <nav class="sidebar" id="sidebar">
      <a class="navlink" data-page="status" data-i18n="nav.status">Status</a>
      <a class="navlink" data-page="values" data-i18n="nav.values">Messwerte</a>
      <a class="navlink" data-page="display" data-i18n="nav.display">Anzeige</a>
      <a class="navlink" data-page="settings" data-i18n="nav.settings">Systemeinstellungen</a>
      <a class="navlink" data-page="logging" data-i18n="nav.logging">Systemprotokoll</a>
      <a class="navlink" data-page="ota" data-i18n="nav.ota">OTA Update</a>
      <a class="navlink" data-page="factory" data-i18n="nav.factory">Werkseinstellungen</a>
    </nav>
    <div class="overlay" id="overlay"></div>

    <main class="content">
      <section id="status" class="page active">
        <div class="page-heading">
          <div>
            <h1 data-i18n="status.title">Status</h1>
            <p class="hint" data-i18n="status.hint">Lokaler Gerätestatus und zuletzt gecachte externe Messwerte.</p>
          </div>
          <span class="badge neutral" id="overallBadge">--</span>
        </div>

        <div class="hero-card">
          <div>
            <div class="eyebrow" data-i18n="status.currentDisplay">Aktueller Anzeigewert</div>
            <div class="hero-value" id="heroValue">----</div>
            <div class="hero-label" id="heroMetric">--</div>
          </div>
          <div class="hero-meta">
            <span id="heroFreshness" class="badge neutral">--</span>
          </div>
        </div>

        <div class="dashboard-grid">
          <article class="card">
            <div class="card-head"><h2 data-i18n="status.api">API</h2><span class="badge neutral" id="apiBadge">--</span></div>
            <dl class="kv">
              <div><dt data-i18n="status.server">Server</dt><dd id="apiServer">--</dd></div>
              <div><dt data-i18n="status.lastRequest">Letzter Abruf</dt><dd id="apiLastRequest">--</dd></div>
              <div><dt data-i18n="status.lastMeasurement">Letzte Messung</dt><dd id="apiLastMeasurement">--</dd></div>
              <div><dt data-i18n="status.dataAge">Datenalter</dt><dd id="apiDataAge">--</dd></div>
              <div><dt data-i18n="status.http">HTTP Status</dt><dd id="apiHttp">--</dd></div>
              <div><dt data-i18n="status.error">Letzter Fehler</dt><dd id="apiError">--</dd></div>
            </dl>
          </article>

          <article class="card">
            <div class="card-head"><h2 data-i18n="status.tm1637">TM1637</h2><span class="badge neutral" id="displayBadge">--</span></div>
            <dl class="kv">
              <div><dt data-i18n="status.displayMetric">Anzeige</dt><dd id="displayMetric">--</dd></div>
              <div><dt data-i18n="status.displayMode">Modus</dt><dd id="displayModeStatus">--</dd></div>
              <div><dt data-i18n="status.brightness">Helligkeit</dt><dd id="displayBrightnessStatus">--</dd></div>
              <div><dt data-i18n="status.clkPin">CLK-Pin</dt><dd id="displayClkStatus">--</dd></div>
              <div><dt data-i18n="status.dioPin">DIO-Pin</dt><dd id="displayDioStatus">--</dd></div>
              <div><dt data-i18n="status.rendered">Gerenderter Wert</dt><dd id="displayRendered">--</dd></div>
            </dl>
          </article>

          <article class="card">
            <div class="card-head"><h2 data-i18n="status.system">System</h2><span class="badge neutral" id="wifiBadge">--</span></div>
            <dl class="kv">
              <div><dt data-i18n="status.localTime">Lokale Zeit</dt><dd id="localTime">--</dd></div>
              <div><dt data-i18n="status.ntp">NTP</dt><dd id="ntpStatus">--</dd></div>
              <div><dt data-i18n="status.lastNtp">Letzte NTP-Synchronisierung</dt><dd id="lastNtp">--</dd></div>
              <div><dt data-i18n="status.ip">IP-Adresse</dt><dd id="ipAddress">--</dd></div>
              <div><dt data-i18n="status.rssi">WLAN RSSI</dt><dd id="wifiRssi">--</dd></div>
              <div><dt data-i18n="status.uptime">Uptime</dt><dd id="uptime">--</dd></div>
              <div><dt data-i18n="status.heap">Free Heap</dt><dd id="freeHeap">--</dd></div>
              <div><dt data-i18n="status.version">Firmware-Version</dt><dd id="firmwareVersion">--</dd></div>
            </dl>
          </article>
        </div>
      </section>

      <section id="values" class="page">
        <h1 data-i18n="values.title">Messwerte</h1>
        <div class="section-title" data-i18n="values.current">Aktuell</div>
        <div class="metric-grid">
          <div class="metric-card"><span data-i18n="metric.solarW">Solarleistung</span><strong id="mSolarW">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.gridW">Netzleistung</span><strong id="mGridW">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.importW">Netzbezug</span><strong id="mImportW">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.exportW">Netzeinspeisung</span><strong id="mExportW">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.consumptionW">Gesamtverbrauch</span><strong id="mConsumptionW">--</strong></div>
        </div>

        <div class="section-title" data-i18n="values.today">Heute</div>
        <div class="metric-grid">
          <div class="metric-card"><span data-i18n="metric.dailySolar">Solar</span><strong id="mDailySolar">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.dailyImport">Netzbezug</span><strong id="mDailyImport">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.dailyExport">Einspeisung</span><strong id="mDailyExport">--</strong></div>
        </div>

        <div class="section-title" data-i18n="values.total">Gesamt</div>
        <div class="metric-grid">
          <div class="metric-card"><span data-i18n="metric.totalSolar">Solar</span><strong id="mTotalSolar">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.totalImport">Netzbezug</span><strong id="mTotalImport">--</strong></div>
          <div class="metric-card"><span data-i18n="metric.totalExport">Einspeisung</span><strong id="mTotalExport">--</strong></div>
        </div>

        <div id="airSection">
          <div class="section-title" data-i18n="values.air">Externe Luftdaten</div>
          <div class="metric-grid">
            <div class="metric-card"><span data-i18n="metric.temperature">Temperatur</span><strong id="mAirTemp">--</strong></div>
            <div class="metric-card"><span data-i18n="metric.humidity">Luftfeuchte</span><strong id="mAirHumidity">--</strong></div>
            <div class="metric-card"><span data-i18n="metric.dewPoint">Taupunkt</span><strong id="mDewPoint">--</strong></div>
            <div class="metric-card"><span data-i18n="metric.pm10">PM10</span><strong id="mPm10">--</strong></div>
            <div class="metric-card"><span data-i18n="metric.pm25">PM2.5</span><strong id="mPm25">--</strong></div>
            <div class="metric-card"><span data-i18n="metric.pressure">Luftdruck</span><strong id="mPressure">--</strong></div>
          </div>
        </div>
      </section>

      <section id="display" class="page card">
        <h1 data-i18n="display.title">Anzeige</h1>
        <form id="displayForm">
          <label class="switch-row">
            <span data-i18n="display.enabled">Display aktiviert</span>
            <input id="displayEnabled" type="checkbox">
          </label>

          <div class="form-grid">
            <div class="form-group" id="metricSettingGroup">
              <label for="selectedMetric" data-i18n="display.metric">Anzuzeigender Wert</label>
              <select id="selectedMetric"></select>
            </div>
            <div class="form-group">
              <label for="displayBrightness" data-i18n="display.brightness">Helligkeit (0–7)</label>
              <input id="displayBrightness" type="number" min="0" max="7" step="1">
            </div>
            <div class="form-group">
              <label for="displayClkGpio" data-i18n="display.clkPin">TM1637 CLK-Pin</label>
              <input id="displayClkGpio" type="number" min="0" max="16" step="1" inputmode="numeric">
            </div>
            <div class="form-group">
              <label for="displayDioGpio" data-i18n="display.dioPin">TM1637 DIO-Pin</label>
              <input id="displayDioGpio" type="number" min="0" max="16" step="1" inputmode="numeric">
            </div>
            <div class="form-group">
              <label for="displayUpdateMs" data-i18n="display.update">Aktualisierung (ms)</label>
              <input id="displayUpdateMs" type="number" min="250" max="200000" step="50">
            </div>
            <div class="form-group">
              <label for="displayMode" data-i18n="display.mode">Anzeigemodus</label>
              <select id="displayMode">
                <option value="metric" data-i18n="display.modeMetric">Nur API-Wert</option>
                <option value="clock" data-i18n="display.modeClock">Nur Uhrzeit</option>
                <option value="alternate" data-i18n="display.modeAlternate">API-Wert und Uhrzeit im Wechsel</option>
              </select>
            </div>
            <div class="form-group" id="alternateSettingGroup">
              <label for="alternateSeconds" data-i18n="display.alternate">Uhrzeit-Anzeigedauer (s)</label>
              <input id="alternateSeconds" type="number" min="2" max="120" step="1">
            </div>
          </div>
          <p class="hint" data-i18n="display.pinHint">Pinänderungen werden nach dem Speichern durch einen Neustart aktiv. GPIO0, GPIO2 und GPIO15 beeinflussen den Bootmodus; GPIO1/GPIO3 werden für die serielle Schnittstelle verwendet.</p>
          <button class="primary" type="submit" data-i18n="display.saveRestart">Speichern</button>
          <div class="form-status" id="displaySaveStatus"></div>
        </form>
      </section>

      <section id="settings" class="page card">
        <h1 data-i18n="settings.title">Systemeinstellungen</h1>
        <form id="systemForm">
          <div class="form-grid">
            <div class="form-group">
              <label for="deviceName" data-i18n="settings.deviceName">Gerätename</label>
              <input id="deviceName" maxlength="32">
            </div>
            <div class="form-group">
              <label for="language" data-i18n="settings.language">Sprache</label>
              <select id="language">
                <option value="de">Deutsch</option>
                <option value="en">English</option>
              </select>
            </div>
            <div class="form-group">
              <label for="theme" data-i18n="settings.theme">Theme</label>
              <select id="theme">
                <option value="light" data-i18n="settings.themeLight">Hell</option>
                <option value="dark" data-i18n="settings.themeDark">Dunkel</option>
              </select>
            </div>
          </div>

          <h2 data-i18n="settings.wifi">WLAN</h2>
          <div class="form-grid">
            <div class="form-group">
              <label for="wifiSsid">SSID</label>
              <input id="wifiSsid" maxlength="32" autocomplete="off">
            </div>
            <div class="form-group">
              <label for="wifiPassword" data-i18n="settings.password">Passwort</label>
              <input id="wifiPassword" type="password" maxlength="64" autocomplete="new-password" data-i18n="settings.passwordPh" data-i18n-attr="placeholder">
              <small id="wifiPasswordState" class="hint"></small>
            </div>
          </div>

          <h2 data-i18n="settings.api">Externe API</h2>
          <div class="form-grid">
            <div class="form-group">
              <label for="apiHost" data-i18n="settings.apiHost">Host / IP-Adresse</label>
              <input id="apiHost" maxlength="64" placeholder="192.168.178.50">
              <small class="hint"><code>/api/current-values</code></small>
            </div>
            <div class="form-group">
              <label for="apiPort" data-i18n="settings.apiPort">Port</label>
              <input id="apiPort" type="number" min="1" max="65535" step="1" value="80">
            </div>
            <div class="form-group">
              <label for="apiPollSeconds" data-i18n="settings.apiPoll">Pollingintervall (s)</label>
              <input id="apiPollSeconds" type="number" min="5" max="300" step="1">
            </div>
          </div>

          <h2 data-i18n="settings.ntp">NTP</h2>
          <div class="form-grid">
            <div class="form-group">
              <label for="ntpServer" data-i18n="settings.ntpServer">NTP-Server</label>
              <input id="ntpServer" maxlength="64">
            </div>
            <div class="form-group wide">
              <label for="timezone" data-i18n="settings.timezone">POSIX-Zeitzone</label>
              <input id="timezone" maxlength="64">
            </div>
          </div>

          <button class="primary" type="submit" data-i18n="common.save">Speichern</button>
          <div class="form-status" id="systemSaveStatus"></div>
        </form>
      </section>

      <section id="logging" class="page card">
        <div class="card-head">
          <h1 data-i18n="log.title">Systemprotokoll</h1>
          <div class="actions">
            <button class="btn" id="refreshLogBtn" type="button" data-i18n="common.refresh">Aktualisieren</button>
            <button class="btn danger-outline" id="clearLogBtn" type="button" data-i18n="log.clear">Leeren</button>
          </div>
        </div>
        <pre id="logView" class="weblog">--</pre>
      </section>

      <section id="ota" class="page card">
        <h1 data-i18n="ota.title">OTA Update</h1>
        <p class="hint"><span data-i18n="ota.channel">Release-Kanal</span>: <code id="otaChannel">--</code></p>
        <dl class="kv">
          <div><dt data-i18n="ota.current">Installiert</dt><dd id="otaCurrent">--</dd></div>
          <div><dt data-i18n="ota.latest">Verfügbar</dt><dd id="otaLatest">--</dd></div>
          <div><dt data-i18n="ota.size">Firmwaregröße</dt><dd id="otaSize">--</dd></div>
          <div><dt data-i18n="ota.status">Status</dt><dd id="otaStatus">--</dd></div>
        </dl>
        <div class="actions stack-mobile">
          <button class="primary fit" id="otaCheckBtn" type="button" data-i18n="ota.check">Auf Update prüfen</button>
          <button class="primary fit" id="otaInstallBtn" type="button" disabled data-i18n="ota.install">Update installieren</button>
        </div>
      </section>

      <section id="factory" class="page card">
        <h1 data-i18n="factory.title">Werkseinstellungen</h1>
        <p class="warning-box" data-i18n="factory.warning">Löscht WLAN-, API-, NTP- und Displayeinstellungen. Messwerte werden ohnehin nicht dauerhaft gespeichert.</p>
        <button class="primary danger" id="factoryResetBtn" type="button" data-i18n="factory.button">Werkseinstellungen wiederherstellen</button>
        <div class="form-status" id="factoryStatus"></div>
      </section>
    </main>
  </div>

  <script src="/script.js"></script>
</body>
</html>

)ED8266";
