#pragma once

#include <Arduino.h>

const char JAVA_SCRIPT[] PROGMEM = R"ED8266(
(() => {
'use strict';

const I18N = {
  de: {
    "a11y.menu":"Menü öffnen/schließen","nav.status":"Status","nav.values":"Messwerte","nav.display":"Anzeige",
    "nav.settings":"Systemeinstellungen","nav.logging":"Systemprotokoll","nav.ota":"OTA Update","nav.factory":"Werkseinstellungen",
    "status.title":"Status","status.hint":"Lokaler Gerätestatus und zuletzt gecachte externe Messwerte.",
    "status.currentDisplay":"Aktueller Anzeigewert","status.api":"API","status.server":"Server","status.lastRequest":"Letzter Abruf",
    "status.lastMeasurement":"Letzte Messung","status.dataAge":"Datenalter","status.http":"HTTP Status","status.error":"Letzter Fehler",
    "status.tm1637":"TM1637","status.displayMetric":"Anzeige","status.displayMode":"Modus","status.brightness":"Helligkeit",
    "status.clkPin":"CLK-Pin","status.dioPin":"DIO-Pin","status.rendered":"Gerenderter Wert","status.system":"System","status.localTime":"Lokale Zeit","status.ntp":"NTP",
    "status.lastNtp":"Letzte NTP-Synchronisierung","status.ip":"IP-Adresse","status.rssi":"WLAN RSSI","status.uptime":"Uptime",
    "status.heap":"Free Heap","status.version":"Firmware-Version",
    "values.title":"Messwerte","values.current":"Aktuell","values.today":"Heute","values.total":"Gesamt","values.air":"Externe Luftdaten",
    "metric.solarW":"Solarleistung","metric.gridW":"Netzleistung","metric.importW":"Netzbezug","metric.exportW":"Netzeinspeisung",
    "metric.consumptionW":"Gesamtverbrauch","metric.dailySolar":"Solar","metric.dailyImport":"Netzbezug","metric.dailyExport":"Einspeisung",
    "metric.totalSolar":"Solar","metric.totalImport":"Netzbezug","metric.totalExport":"Einspeisung","metric.temperature":"Temperatur",
    "metric.humidity":"Luftfeuchte","metric.dewPoint":"Taupunkt","metric.pm10":"PM10","metric.pm25":"PM2.5","metric.pressure":"Luftdruck",
    "display.title":"Anzeige","display.enabled":"Display aktiviert","display.metric":"Anzuzeigender Wert","display.brightness":"Helligkeit (0–7)",
    "display.update":"Aktualisierung (ms)","display.mode":"Anzeigemodus","display.modeMetric":"Nur API-Wert","display.modeClock":"Nur Uhrzeit (API optional)",
    "display.modeAlternate":"API-Wert und Uhrzeit im Wechsel","display.alternate":"Wechselintervall (s)",
    "display.clkPin":"TM1637 CLK GPIO","display.dioPin":"TM1637 DIO GPIO","display.pinHint":"GPIO-Nummern direkt eingeben (0–5 oder 12–16). GPIO6–11 sind mit dem ESP8266-Flash verbunden und nicht zulässig. GPIO0, GPIO2 und GPIO15 beeinflussen den Bootmodus; GPIO1/GPIO3 werden für die serielle Schnittstelle verwendet. Pinänderungen werden nach dem Speichern durch einen Neustart aktiv.","display.saveRestart":"Speichern","display.pinSame":"CLK und DIO müssen unterschiedliche GPIOs verwenden.","display.pinInvalid":"GPIO muss eine ganze Zahl von 0 bis 16 sein; GPIO6 bis GPIO11 sind nicht zulässig.",
    "settings.title":"Systemeinstellungen","settings.deviceName":"Gerätename","settings.language":"Sprache","settings.theme":"Theme",
    "settings.themeLight":"Hell","settings.themeDark":"Dunkel","settings.wifi":"WLAN","settings.password":"Passwort",
    "settings.passwordPh":"Leer lassen, um das gespeicherte Passwort beizubehalten","settings.passwordSaved":"Passwort ist gespeichert.",
    "settings.passwordMissing":"Kein Passwort gespeichert.","settings.api":"Externe API","settings.apiHost":"Host / IP-Adresse",
    "settings.apiPoll":"Pollingintervall (s)","settings.ntp":"NTP","settings.ntpServer":"NTP-Server","settings.timezone":"POSIX-Zeitzone",
    "common.save":"Speichern","common.saved":"Gespeichert.","common.refresh":"Aktualisieren","common.failed":"Fehlgeschlagen",
    "log.title":"Systemprotokoll","log.clear":"Leeren","ota.title":"OTA Update","ota.channel":"Release-Kanal","ota.current":"Installiert",
    "ota.latest":"Verfügbar","ota.size":"Firmwaregröße","ota.status":"Status","ota.check":"Auf Update prüfen","ota.install":"Update installieren",
    "factory.title":"Werkseinstellungen","factory.warning":"Löscht WLAN-, API-, NTP- und Displayeinstellungen. Messwerte werden nicht dauerhaft gespeichert.",
    "factory.button":"Werkseinstellungen wiederherstellen","factory.confirm1":"Wirklich alle gespeicherten Einstellungen löschen?",
    "factory.confirm2":"Letzte Bestätigung: Das Gerät startet danach mit Werkseinstellungen neu.",
    "state.connected":"Verbunden","state.unavailable":"Nicht erreichbar","state.notConfigured":"Nicht konfiguriert","state.stale":"Daten veraltet",
    "state.current":"Aktuell","state.enabled":"Aktiv","state.disabled":"Deaktiviert","state.synced":"Synchronisiert","state.notSynced":"Nicht synchronisiert",
    "state.ok":"OK","state.noData":"Keine Daten","state.clock":"Uhrzeit","state.updateAvailable":"Update verfügbar","state.upToDate":"Aktuell",
    "state.restart":"Gespeichert. Neustart wird ausgeführt."
  },
  en: {
    "a11y.menu":"Open/close menu","nav.status":"Status","nav.values":"Measurements","nav.display":"Display",
    "nav.settings":"System Settings","nav.logging":"System Log","nav.ota":"OTA Update","nav.factory":"Factory Reset",
    "status.title":"Status","status.hint":"Local device status and the latest cached external measurements.",
    "status.currentDisplay":"Current display value","status.api":"API","status.server":"Server","status.lastRequest":"Last request",
    "status.lastMeasurement":"Last measurement","status.dataAge":"Data age","status.http":"HTTP status","status.error":"Last error",
    "status.tm1637":"TM1637","status.displayMetric":"Metric","status.displayMode":"Mode","status.brightness":"Brightness",
    "status.clkPin":"CLK pin","status.dioPin":"DIO pin","status.rendered":"Rendered value","status.system":"System","status.localTime":"Local time","status.ntp":"NTP",
    "status.lastNtp":"Last NTP synchronization","status.ip":"IP address","status.rssi":"Wi-Fi RSSI","status.uptime":"Uptime",
    "status.heap":"Free heap","status.version":"Firmware version",
    "values.title":"Measurements","values.current":"Current","values.today":"Today","values.total":"Total","values.air":"External air data",
    "metric.solarW":"Solar production","metric.gridW":"Grid power","metric.importW":"Grid import","metric.exportW":"Grid export",
    "metric.consumptionW":"Total consumption","metric.dailySolar":"Solar","metric.dailyImport":"Grid import","metric.dailyExport":"Grid export",
    "metric.totalSolar":"Solar","metric.totalImport":"Grid import","metric.totalExport":"Grid export","metric.temperature":"Temperature",
    "metric.humidity":"Humidity","metric.dewPoint":"Dew point","metric.pm10":"PM10","metric.pm25":"PM2.5","metric.pressure":"Pressure",
    "display.title":"Display","display.enabled":"Display enabled","display.metric":"Displayed metric","display.brightness":"Brightness (0–7)",
    "display.update":"Refresh interval (ms)","display.mode":"Display mode","display.modeMetric":"API value only","display.modeClock":"Clock only (API optional)",
    "display.modeAlternate":"Alternate API value and clock","display.alternate":"Alternation interval (s)",
    "display.clkPin":"TM1637 CLK GPIO","display.dioPin":"TM1637 DIO GPIO","display.pinHint":"Enter GPIO numbers directly (0–5 or 12–16). GPIO6–11 are connected to ESP8266 flash and are not allowed. GPIO0, GPIO2 and GPIO15 affect the boot mode; GPIO1/GPIO3 are used by the serial interface. Pin changes become active after saving and restarting.","display.saveRestart":"Save","display.pinSame":"CLK and DIO must use different GPIOs.","display.pinInvalid":"GPIO must be a whole number from 0 to 16; GPIO6 through GPIO11 are not allowed.",
    "settings.title":"System Settings","settings.deviceName":"Device name","settings.language":"Language","settings.theme":"Theme",
    "settings.themeLight":"Light","settings.themeDark":"Dark","settings.wifi":"Wi-Fi","settings.password":"Password",
    "settings.passwordPh":"Leave blank to keep the stored password","settings.passwordSaved":"A password is stored.",
    "settings.passwordMissing":"No password is stored.","settings.api":"External API","settings.apiHost":"Host / IP address",
    "settings.apiPoll":"Polling interval (s)","settings.ntp":"NTP","settings.ntpServer":"NTP server","settings.timezone":"POSIX timezone",
    "common.save":"Save","common.saved":"Saved.","common.refresh":"Refresh","common.failed":"Failed",
    "log.title":"System Log","log.clear":"Clear","ota.title":"OTA Update","ota.channel":"Release channel","ota.current":"Installed",
    "ota.latest":"Available","ota.size":"Firmware size","ota.status":"Status","ota.check":"Check for update","ota.install":"Install update",
    "factory.title":"Factory Reset","factory.warning":"Deletes Wi-Fi, API, NTP and display settings. Measurements are never stored persistently.",
    "factory.button":"Restore factory settings","factory.confirm1":"Really delete all stored settings?",
    "factory.confirm2":"Final confirmation: the device will restart with factory settings.",
    "state.connected":"Connected","state.unavailable":"Unavailable","state.notConfigured":"Not configured","state.stale":"Data stale",
    "state.current":"Current","state.enabled":"Enabled","state.disabled":"Disabled","state.synced":"Synchronized","state.notSynced":"Not synchronized",
    "state.ok":"OK","state.noData":"No data","state.clock":"Clock","state.updateAvailable":"Update available","state.upToDate":"Up to date",
    "state.restart":"Saved. Restarting device."
  }
};


const DISPLAY_PINS = [
  [5,"D1 / GPIO5",false],[4,"D2 / GPIO4",false],[14,"D5 / GPIO14",false],[12,"D6 / GPIO12",false],[13,"D7 / GPIO13",false],
  [16,"D0 / GPIO16",false],[0,"D3 / GPIO0",true],[2,"D4 / GPIO2",true],[15,"D8 / GPIO15",true],[3,"RX / GPIO3",true],[1,"TX / GPIO1",true]
];

const METRICS = [
  ["current_solar_production_w","metric.solarW"],["current_grid_power_w","metric.gridW"],
  ["current_grid_import_w","metric.importW"],["current_grid_export_w","metric.exportW"],
  ["current_total_consumption_w","metric.consumptionW"],["daily_solar_production_kwh","metric.dailySolar"],
  ["daily_grid_import_kwh","metric.dailyImport"],["daily_grid_export_kwh","metric.dailyExport"],
  ["total_solar_production_kwh","metric.totalSolar"],["total_grid_import_kwh","metric.totalImport"],
  ["total_grid_export_kwh","metric.totalExport"],["air_temperature_c","metric.temperature"],
  ["air_humidity_percent","metric.humidity"],["air_dew_point_c","metric.dewPoint"],["air_pm10","metric.pm10"],["air_pm25","metric.pm25"]
];

let settings = null;
let state = null;
let otaUpdateActive = false;
let lang = "de";

const $ = id => document.getElementById(id);
const text = (id, value) => { const el=$(id); if(el) el.textContent = value ?? "--"; };
const t = key => (I18N[lang] && I18N[lang][key]) || I18N.de[key] || key;

function applyTranslations(){
  document.documentElement.lang = lang;
  document.querySelectorAll("[data-i18n]").forEach(el => {
    const key=el.dataset.i18n, value=t(key), attr=el.dataset.i18nAttr;
    if(attr) el.setAttribute(attr,value); else el.textContent=value;
  });
  fillMetricSelect();
  if(state) renderState();
}
function applyTheme(value){ document.documentElement.setAttribute("data-theme", value === "dark" ? "dark" : "light"); }

function badge(id,label,tone){
  const el=$(id); if(!el) return;
  el.textContent=label;
  el.className=`badge ${tone||"neutral"}`;
}
function apiFetch(url,options={}){
  return fetch(url,{cache:"no-store",...options}).then(async r=>{
    const body=await r.json().catch(()=>({ok:false,error:`HTTP ${r.status}`}));
    if(!r.ok || body.ok===false) throw new Error(body.error||`HTTP ${r.status}`);
    return body;
  });
}
function fmtNumber(value,unit,decimals=1){
  if(typeof value!=="number" || !Number.isFinite(value)) return "--";
  return `${value.toLocaleString(lang==="de"?"de-DE":"en-US",{maximumFractionDigits:decimals})}${unit?` ${unit}`:""}`;
}
function fmtAge(seconds){
  if(typeof seconds!=="number" || seconds<0) return "--";
  if(seconds<60) return `${seconds} s`;
  if(seconds<3600) return `${Math.floor(seconds/60)} min`;
  return `${Math.floor(seconds/3600)} h ${Math.floor((seconds%3600)/60)} min`;
}
function fmtUptime(seconds){
  if(typeof seconds!=="number") return "--";
  const d=Math.floor(seconds/86400),h=Math.floor((seconds%86400)/3600),m=Math.floor((seconds%3600)/60);
  return d?`${d} d ${h} h`:h?`${h} h ${m} min`:`${m} min`;
}
function metricLabel(id){
  const item=METRICS.find(([key])=>key===id);
  return item?t(item[1]):id||"--";
}
function modeLabel(mode){
  return mode==="clock"?t("display.modeClock"):mode==="alternate"?t("display.modeAlternate"):t("display.modeMetric");
}
function metricUnit(id){
  if(id.endsWith("_w")) return "W";
  if(id.includes("kwh")) return "kWh";
  if(id==="air_temperature_c"||id==="air_dew_point_c") return "°C";
  if(id==="air_humidity_percent") return "%";
  if(id==="air_pm10"||id==="air_pm25") return "µg/m³";
  return "";
}
function externalMetric(id){
  const e=state?.external||{},a=e.airSensor||{};
  const map={
    current_solar_production_w:e.currentSolarProductionW,current_grid_power_w:e.currentGridPowerW,
    current_grid_import_w:e.currentGridImportW,current_grid_export_w:e.currentGridExportW,
    current_total_consumption_w:e.currentTotalConsumptionW,daily_solar_production_kwh:e.dailySolarProductionKwh,
    daily_grid_import_kwh:e.dailyGridImportKwh,daily_grid_export_kwh:e.dailyGridExportKwh,
    total_solar_production_kwh:e.totalSolarProductionKwh,total_grid_import_kwh:e.totalGridImportKwh,
    total_grid_export_kwh:e.totalGridExportKwh,air_temperature_c:a.temperatureC,air_humidity_percent:a.humidityPercent,
    air_dew_point_c:a.dewPointC,air_pm10:a.pm10,air_pm25:a.pm25
  };
  return map[id];
}

function pinLabel(gpio){
  const item=DISPLAY_PINS.find(([id])=>id===Number(gpio));
  return item?item[1]:`GPIO${gpio}`;
}


function fillMetricSelect(){
  const select=$("selectedMetric"); if(!select) return;
  const selected=select.value||settings?.display.metric||"current_grid_import_w";
  select.innerHTML="";
  METRICS.forEach(([id,key])=>{
    const option=document.createElement("option"); option.value=id; option.textContent=t(key); select.appendChild(option);
  });
  select.value=selected;
}

function updateDisplayModeUi(){
  const mode=$("displayMode")?.value||"metric";
  const metricGroup=$("metricSettingGroup");
  const alternateGroup=$("alternateSettingGroup");
  if(metricGroup) metricGroup.classList.toggle("hidden",mode==="clock");
  if(alternateGroup) alternateGroup.classList.toggle("hidden",mode!=="alternate");
  if($("selectedMetric")) $("selectedMetric").disabled=mode==="clock";
  if($("alternateSeconds")) $("alternateSeconds").disabled=mode!=="alternate";
}

function setupNavigation(){
  const sidebar=$("sidebar"),overlay=$("overlay"),burger=$("hamburgerBtn");
  const open=()=>{sidebar.classList.add("sidebar--open");overlay.classList.add("overlay--show");burger.setAttribute("aria-expanded","true");};
  const close=()=>{sidebar.classList.remove("sidebar--open");overlay.classList.remove("overlay--show");burger.setAttribute("aria-expanded","false");};
  burger.addEventListener("click",()=>sidebar.classList.contains("sidebar--open")?close():open());
  overlay.addEventListener("click",close);
  sidebar.addEventListener("click",e=>{
    const link=e.target.closest(".navlink"); if(!link) return;
    document.querySelectorAll(".page").forEach(p=>p.classList.toggle("active",p.id===link.dataset.page));
    document.querySelectorAll(".navlink").forEach(a=>a.toggleAttribute("aria-current",a===link));
    close();
    if(link.dataset.page==="logging") loadLog();
  });
  sidebar.querySelector('.navlink[data-page="status"]').setAttribute("aria-current","page");
}

function populateSettings(){
  if(!settings) return;
  text("deviceTitle",settings.deviceName);
  $("deviceName").value=settings.deviceName||"";
  $("language").value=settings.language||"de";
  $("theme").value=settings.theme||"light";
  $("wifiSsid").value=settings.wifi.ssid||"";
  $("wifiPassword").value="";
  text("wifiPasswordState",settings.wifi.passwordConfigured?t("settings.passwordSaved"):t("settings.passwordMissing"));
  $("apiHost").value=settings.api.host||"";
  $("apiPollSeconds").value=settings.api.pollSeconds;
  $("ntpServer").value=settings.ntp.server||"";
  $("timezone").value=settings.ntp.timezone||"";
  $("displayEnabled").checked=!!settings.display.enabled;
  $("displayBrightness").value=settings.display.brightness;
  $("displayClkGpio").value=settings.display.clkGpio;
  $("displayDioGpio").value=settings.display.dioGpio;
  $("displayUpdateMs").value=settings.display.updateMs;
  $("displayMode").value=settings.display.mode;
  $("alternateSeconds").value=settings.display.alternateSeconds;
  fillMetricSelect();
  $("selectedMetric").value=settings.display.metric;
  updateDisplayModeUi();
  lang=settings.language==="en"?"en":"de";
  applyTheme(settings.theme);
  applyTranslations();
}

function renderState(){
  if(!state) return;
  text("deviceTitle",state.deviceName);
  text("firmwareVersion",state.firmwareVersion);
  text("localTime",state.ntp.localTime||"--");
  if(state.ntp.localTime){
    const parts=state.ntp.localTime.split(" ");
    text("headerDate",parts[0]||"--"); text("headerTime",parts[1]||"--");
  }
  text("lastNtp",state.ntp.lastSync||"--");
  text("ntpStatus",state.ntp.synchronized?t("state.synced"):t("state.notSynced"));
  text("ipAddress",state.wifi.connected?state.wifi.ip:(state.wifi.apActive?state.wifi.apIp:"--"));
  text("wifiRssi",state.wifi.connected?`${state.wifi.rssi} dBm`:"--");
  text("uptime",fmtUptime(state.system.uptimeSeconds));
  text("freeHeap",`${state.system.freeHeap} B`);
  badge("wifiBadge",state.wifi.connected?t("state.connected"):(state.wifi.apActive?"AP":t("state.unavailable")),state.wifi.connected?"good":"warn");

  const api=state.api;
  const configured=api.configured;
  badge("apiBadge",!configured?t("state.notConfigured"):api.connected?t("state.connected"):t("state.unavailable"),!configured?"neutral":api.connected?"good":"bad");
  text("apiServer",configured?api.endpoint:"--");
  text("apiLastRequest",api.lastAttemptAgeSeconds==null?"--":fmtAge(api.lastAttemptAgeSeconds));
  text("apiLastMeasurement",state.external.lastMeasurementAt||"--");
  text("apiDataAge",api.dataAgeSeconds==null?"--":fmtAge(api.dataAgeSeconds));
  text("apiHttp",api.httpStatus||"--");
  text("apiError",api.lastError||"--");

  const d=state.display;
  badge("displayBadge",d.enabled?t("state.enabled"):t("state.disabled"),d.enabled?"good":"neutral");
  text("displayMetric",d.mode==="clock"?t("state.clock"):metricLabel(d.metric));
  text("displayModeStatus",modeLabel(d.mode));
  text("displayBrightnessStatus",`${d.brightness}/7`);
  text("displayClkStatus",pinLabel(d.clkGpio));
  text("displayDioStatus",pinLabel(d.dioGpio));
  text("displayRendered",d.rendered+(d.scaledThousands?" ×1000":""));

  if(d.mode==="clock"){
    const clockText=(state.ntp.localTime||"").split(" ")[1]||"----";
    text("heroValue",clockText); text("heroMetric",t("state.clock"));
    badge("heroFreshness",state.ntp.synchronized?t("state.synced"):t("state.notSynced"),state.ntp.synchronized?"good":"warn");
    badge("overallBadge",state.wifi.connected&&state.ntp.synchronized?t("state.ok"):t("state.unavailable"),state.wifi.connected&&state.ntp.synchronized?"good":"warn");
  }else{
    const selected=externalMetric(d.metric);
    const hero=typeof selected==="number"&&Number.isFinite(selected)?fmtNumber(selected,metricUnit(d.metric),d.metric.endsWith("_w")?0:2):"----";
    text("heroValue",hero); text("heroMetric",metricLabel(d.metric));
    badge("heroFreshness",!api.valid?t("state.noData"):api.stale?t("state.stale"):t("state.current"),!api.valid?"neutral":api.stale?"warn":"good");
    badge("overallBadge",state.wifi.connected&&api.connected?t("state.ok"):t("state.unavailable"),state.wifi.connected&&api.connected?"good":"warn");
  }

  const e=state.external,a=e.airSensor||{};
  text("mSolarW",fmtNumber(e.currentSolarProductionW,"W",0)); text("mGridW",fmtNumber(e.currentGridPowerW,"W",0));
  text("mImportW",fmtNumber(e.currentGridImportW,"W",0)); text("mExportW",fmtNumber(e.currentGridExportW,"W",0));
  text("mConsumptionW",fmtNumber(e.currentTotalConsumptionW,"W",0));
  text("mDailySolar",fmtNumber(e.dailySolarProductionKwh,"kWh",2)); text("mDailyImport",fmtNumber(e.dailyGridImportKwh,"kWh",2));
  text("mDailyExport",fmtNumber(e.dailyGridExportKwh,"kWh",2)); text("mTotalSolar",fmtNumber(e.totalSolarProductionKwh,"kWh",2));
  text("mTotalImport",fmtNumber(e.totalGridImportKwh,"kWh",2)); text("mTotalExport",fmtNumber(e.totalGridExportKwh,"kWh",2));
  $("airSection").classList.toggle("hidden",!a.present);
  text("mAirTemp",fmtNumber(a.temperatureC,"°C",1)); text("mAirHumidity",fmtNumber(a.humidityPercent,"%",1));
  text("mDewPoint",fmtNumber(a.dewPointC,"°C",1)); text("mPm10",fmtNumber(a.pm10,"µg/m³",2));
  text("mPm25",fmtNumber(a.pm25,"µg/m³",2)); text("mPressure",fmtNumber(a.pressureHpa,"hPa",1));
}

async function loadSettings(){ settings=await apiFetch("/api/settings"); populateSettings(); }
async function loadState(){
  if(otaUpdateActive)return;
  try{ state=await apiFetch("/api/state"); renderState(); }catch(err){ badge("overallBadge",t("state.unavailable"),"bad"); }
}
async function loadLog(){ try{const data=await apiFetch("/api/log"); $("logView").textContent=(data.lines||[]).join("\n")||"--";}catch(err){$("logView").textContent=err.message;} }

$("displayMode").addEventListener("change",updateDisplayModeUi);

$("displayForm").addEventListener("submit",async e=>{
  e.preventDefault(); text("displaySaveStatus","…");
  try{
    const body={
      enabled:$("displayEnabled").checked,brightness:Number($("displayBrightness").value),
      clkGpio:Number($("displayClkGpio").value),dioGpio:Number($("displayDioGpio").value),metric:$("selectedMetric").value||settings?.display.metric||"current_grid_import_w",
      mode:$("displayMode").value,updateMs:Number($("displayUpdateMs").value),alternateSeconds:Number($("alternateSeconds").value)
    };
    const validPin=gpio=>Number.isInteger(gpio)&&gpio>=0&&gpio<=16&&!(gpio>=6&&gpio<=11);
    if(!validPin(body.clkGpio)||!validPin(body.dioGpio)) throw new Error(t("display.pinInvalid"));
    if(body.clkGpio===body.dioGpio) throw new Error(t("display.pinSame"));
    const result=await apiFetch("/api/settings/display",{method:"POST",headers:{"content-type":"application/json"},body:JSON.stringify(body)});
    text("displaySaveStatus",result.restartScheduled?t("state.restart"):t("common.saved"));
    if(!result.restartScheduled){await loadSettings(); await loadState();}
  }catch(err){ text("displaySaveStatus",`${t("common.failed")}: ${err.message}`); }
});

$("systemForm").addEventListener("submit",async e=>{
  e.preventDefault(); text("systemSaveStatus","…");
  try{
    const body={
      deviceName:$("deviceName").value.trim(),language:$("language").value,theme:$("theme").value,
      wifiSsid:$("wifiSsid").value.trim(),apiHost:$("apiHost").value.trim(),apiPollSeconds:Number($("apiPollSeconds").value),
      ntpServer:$("ntpServer").value.trim(),timezone:$("timezone").value.trim()
    };
    if($("wifiPassword").value.length) body.wifiPassword=$("wifiPassword").value;
    const result=await apiFetch("/api/settings/system",{method:"POST",headers:{"content-type":"application/json"},body:JSON.stringify(body)});
    lang=body.language==="en"?"en":"de"; applyTheme(body.theme); applyTranslations();
    text("systemSaveStatus",result.restartScheduled?t("state.restart"):t("common.saved"));
    if(!result.restartScheduled){await loadSettings();await loadState();}
  }catch(err){ text("systemSaveStatus",`${t("common.failed")}: ${err.message}`); }
});
$("language").addEventListener("change",e=>{lang=e.target.value==="en"?"en":"de";applyTranslations();});
$("theme").addEventListener("change",e=>applyTheme(e.target.value));

$("refreshLogBtn").addEventListener("click",loadLog);
$("clearLogBtn").addEventListener("click",async()=>{try{await apiFetch("/api/log/clear",{method:"POST"});await loadLog();}catch(err){$("logView").textContent=err.message;}});

$("otaCheckBtn").addEventListener("click",async()=>{
  text("otaStatus","…"); $("otaInstallBtn").disabled=true;
  try{
    const data=await apiFetch("/api/ota/check");
    text("otaCurrent",data.currentVersion);text("otaLatest",data.latestVersion||"--");
    text("otaSize",data.firmwareSize?`${data.firmwareSize} B`:"--");
    text("otaStatus",data.updateAvailable?t("state.updateAvailable"):t("state.upToDate"));
    $("otaInstallBtn").disabled=!data.updateAvailable;
  }catch(err){text("otaStatus",`${t("common.failed")}: ${err.message}`);}
});
$("otaInstallBtn").addEventListener("click",async()=>{
  otaUpdateActive=true;
  $("otaInstallBtn").disabled=true;text("otaStatus","…");
  try{
    await apiFetch("/api/ota/update",{method:"POST"});
    text("otaStatus",t("state.restart"));
  }catch(err){
    otaUpdateActive=false;
    text("otaStatus",`${t("common.failed")}: ${err.message}`);
    $("otaInstallBtn").disabled=false;
    await loadState();
  }
});

$("factoryResetBtn").addEventListener("click",async()=>{
  if(!confirm(t("factory.confirm1"))||!confirm(t("factory.confirm2"))) return;
  try{const data=await apiFetch("/api/factory-reset",{method:"POST",headers:{"content-type":"application/json"},body:'{"confirm":"RESET"}'});text("factoryStatus",data.message||t("state.restart"));}catch(err){text("factoryStatus",`${t("common.failed")}: ${err.message}`);}
});

async function init(){
  setupNavigation();
  try{await loadSettings();}catch(err){console.error(err);}
  text("otaChannel","syschelle/espDisplay · ota");
  await loadState();
  setInterval(loadState,5000);
}
init();
})();

)ED8266";
