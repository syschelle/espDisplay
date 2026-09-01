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
    "display.modeAlternate":"API-Wert und Uhrzeit im Wechsel","display.alternate":"Uhrzeit-Anzeigedauer (s)","display.apiValueDuration":"API-Wert-Anzeigedauer (ms)",
    "display.clkPin":"TM1637 CLK GPIO","display.dioPin":"TM1637 DIO GPIO","display.pinHint":"GPIO-Nummern direkt eingeben (0–5 oder 12–16). GPIO6–11 sind mit dem ESP8266-Flash verbunden und nicht zulässig. GPIO0, GPIO2 und GPIO15 beeinflussen den Bootmodus; GPIO1/GPIO3 werden für die serielle Schnittstelle verwendet. Pinänderungen werden nach dem Speichern durch einen Neustart aktiv.","display.saveRestart":"Speichern","display.pinSame":"CLK und DIO müssen unterschiedliche GPIOs verwenden.","display.pinInvalid":"GPIO muss eine ganze Zahl von 0 bis 16 sein; GPIO6 bis GPIO11 sind nicht zulässig.",
    "settings.title":"Systemeinstellungen","settings.deviceName":"Gerätename","settings.language":"Sprache","settings.theme":"Theme",
    "settings.themeLight":"Hell","settings.themeDark":"Dunkel","settings.wifi":"WLAN","settings.password":"Passwort",
    "settings.passwordPh":"Leer lassen, um das gespeicherte Passwort beizubehalten","settings.passwordSaved":"Passwort ist gespeichert.",
    "settings.passwordMissing":"Kein Passwort gespeichert.","settings.api":"Externe API","settings.apiHost":"Host / IP-Adresse",
    "settings.apiPort":"Port","settings.apiPoll":"Pollingintervall (s)","settings.ntp":"NTP","settings.ntpServer":"NTP-Server","settings.timezone":"POSIX-Zeitzone",
    "common.save":"Speichern","common.saved":"Gespeichert.","common.refresh":"Aktualisieren","common.failed":"Fehlgeschlagen",
    "log.title":"Systemprotokoll","log.clear":"Leeren","ota.title":"OTA Update","ota.channel":"Release-Kanal","ota.current":"Installiert",
    "ota.latest":"Verfügbar","ota.size":"Firmwaregröße","ota.status":"Status","ota.check":"Auf Update prüfen","ota.install":"Update installieren",
    "ota.manifest":"Manifest wird im Browser geladen","ota.firmware":"Firmware wird im Browser geladen","ota.verify":"Firmware wird geprüft",
    "ota.upload":"Firmware wird zum ESP übertragen","ota.browserError":"GitHub konnte im Browser nicht erreicht werden",
    "ota.browserHint":"Updateprüfung und Firmware-Download erfolgen im Browser; der ESP8266 erhält nur die geprüfte Firmware.",
    "factory.title":"Werkseinstellungen","factory.warning":"Löscht WLAN-, API-, NTP- und Displayeinstellungen. Messwerte werden nicht dauerhaft gespeichert.",
    "factory.button":"Werkseinstellungen wiederherstellen","factory.confirm1":"Wirklich alle gespeicherten Einstellungen löschen?",
    "factory.confirm2":"Letzte Bestätigung: Das Gerät startet danach mit Werkseinstellungen neu.",
    "state.connected":"Verbunden","state.requesting":"Abfrage läuft","state.unavailable":"Nicht erreichbar","state.notConfigured":"Nicht konfiguriert","state.stale":"Daten veraltet",
    "state.current":"Aktuell","state.enabled":"Aktiv","state.disabled":"Deaktiviert","state.synced":"Synchronisiert","state.notSynced":"Nicht synchronisiert",
    "state.ok":"OK","state.noData":"Keine Daten","state.noMeasurement":"Noch keine Messung","state.clock":"Uhrzeit","state.updateAvailable":"Update verfügbar","state.upToDate":"Aktuell",
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
    "display.modeAlternate":"Alternate API value and clock","display.alternate":"Clock display duration (s)","display.apiValueDuration":"API value display duration (ms)",
    "display.clkPin":"TM1637 CLK GPIO","display.dioPin":"TM1637 DIO GPIO","display.pinHint":"Enter GPIO numbers directly (0–5 or 12–16). GPIO6–11 are connected to ESP8266 flash and are not allowed. GPIO0, GPIO2 and GPIO15 affect the boot mode; GPIO1/GPIO3 are used by the serial interface. Pin changes become active after saving and restarting.","display.saveRestart":"Save","display.pinSame":"CLK and DIO must use different GPIOs.","display.pinInvalid":"GPIO must be a whole number from 0 to 16; GPIO6 through GPIO11 are not allowed.",
    "settings.title":"System Settings","settings.deviceName":"Device name","settings.language":"Language","settings.theme":"Theme",
    "settings.themeLight":"Light","settings.themeDark":"Dark","settings.wifi":"Wi-Fi","settings.password":"Password",
    "settings.passwordPh":"Leave blank to keep the stored password","settings.passwordSaved":"A password is stored.",
    "settings.passwordMissing":"No password is stored.","settings.api":"External API","settings.apiHost":"Host / IP address",
    "settings.apiPort":"Port","settings.apiPoll":"Polling interval (s)","settings.ntp":"NTP","settings.ntpServer":"NTP server","settings.timezone":"POSIX timezone",
    "common.save":"Save","common.saved":"Saved.","common.refresh":"Refresh","common.failed":"Failed",
    "log.title":"System Log","log.clear":"Clear","ota.title":"OTA Update","ota.channel":"Release channel","ota.current":"Installed",
    "ota.latest":"Available","ota.size":"Firmware size","ota.status":"Status","ota.check":"Check for update","ota.install":"Install update",
    "ota.manifest":"Loading manifest in browser","ota.firmware":"Loading firmware in browser","ota.verify":"Verifying firmware",
    "ota.upload":"Uploading firmware to ESP","ota.browserError":"GitHub could not be reached by the browser",
    "ota.browserHint":"The browser checks and downloads the update; the ESP8266 receives only the verified firmware.",
    "factory.title":"Factory Reset","factory.warning":"Deletes Wi-Fi, API, NTP and display settings. Measurements are never stored persistently.",
    "factory.button":"Restore factory settings","factory.confirm1":"Really delete all stored settings?",
    "factory.confirm2":"Final confirmation: the device will restart with factory settings.",
    "state.connected":"Connected","state.requesting":"Request in progress","state.unavailable":"Unavailable","state.notConfigured":"Not configured","state.stale":"Data stale",
    "state.current":"Current","state.enabled":"Enabled","state.disabled":"Disabled","state.synced":"Synchronized","state.notSynced":"Not synchronized",
    "state.ok":"OK","state.noData":"No data","state.noMeasurement":"No measurement yet","state.clock":"Clock","state.updateAvailable":"Update available","state.upToDate":"Up to date",
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

const OTA_RAW_BASE = "https://raw.githubusercontent.com/syschelle/espDisplay/ota/";
const OTA_API_BASE = "https://api.github.com/repos/syschelle/espDisplay/contents/";

let settings = null;
let state = null;
let otaUpdateActive = false;
let pendingOtaManifest = null;
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

function parseSemver(value){
  const m=/^[vV]?(\d+)\.(\d+)\.(\d+)$/.exec(String(value||"").trim());
  return m?[Number(m[1]),Number(m[2]),Number(m[3])]:null;
}
function compareSemver(a,b){
  const av=parseSemver(a),bv=parseSemver(b);
  if(!av||!bv)return 0;
  for(let i=0;i<3;i++){if(av[i]!==bv[i])return av[i]<bv[i]?-1:1;}
  return 0;
}
function bytesToAscii(bytes){
  let out="";
  for(let i=0;i<bytes.length;i++)out+=String.fromCharCode(bytes[i]);
  return out;
}
function decodeBase64Bytes(value){
  const raw=atob(String(value||"").replace(/\s/g,""));
  const out=new Uint8Array(raw.length);
  for(let i=0;i<raw.length;i++)out[i]=raw.charCodeAt(i);
  return out;
}
async function fetchOtaBytes(name,cacheKey=""){
  const suffix=`?cb=${Date.now()}${cacheKey?`&v=${encodeURIComponent(cacheKey)}`:""}`;
  try{
    const response=await fetch(`${OTA_RAW_BASE}${name}${suffix}`,{cache:"no-store",mode:"cors"});
    if(!response.ok)throw new Error(`HTTP ${response.status}`);
    return new Uint8Array(await response.arrayBuffer());
  }catch(rawError){
    try{
      const response=await fetch(`${OTA_API_BASE}${encodeURIComponent(name)}?ref=ota&cb=${Date.now()}`,{
        cache:"no-store",headers:{Accept:"application/vnd.github+json"}
      });
      if(!response.ok)throw new Error(`HTTP ${response.status}`);
      const meta=await response.json();
      if(meta&&meta.encoding==="base64"&&meta.content)return decodeBase64Bytes(meta.content);
      if(meta&&meta.download_url){
        const fallback=await fetch(`${meta.download_url}${meta.download_url.includes("?")?"&":"?"}cb=${Date.now()}`,{cache:"no-store",mode:"cors"});
        if(!fallback.ok)throw new Error(`HTTP ${fallback.status}`);
        return new Uint8Array(await fallback.arrayBuffer());
      }
      throw new Error("GitHub content response contains no downloadable file");
    }catch(apiError){
      throw new Error(`${t("ota.browserError")}: ${apiError.message||rawError.message}`);
    }
  }
}
function sha256Hex(bytes){
  const K=new Uint32Array([
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
  ]);
  const H=new Uint32Array([0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19]);
  const bitLength=bytes.length*8;
  const total=Math.ceil((bytes.length+9)/64)*64;
  const data=new Uint8Array(total);data.set(bytes);data[bytes.length]=0x80;
  const view=new DataView(data.buffer);
  view.setUint32(total-8,Math.floor(bitLength/0x100000000),false);
  view.setUint32(total-4,bitLength>>>0,false);
  const w=new Uint32Array(64);
  const rotr=(x,n)=>(x>>>n)|(x<<(32-n));
  for(let offset=0;offset<total;offset+=64){
    for(let i=0;i<16;i++)w[i]=view.getUint32(offset+i*4,false);
    for(let i=16;i<64;i++){
      const s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>>3);
      const s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>>10);
      w[i]=(w[i-16]+s0+w[i-7]+s1)>>>0;
    }
    let a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
    for(let i=0;i<64;i++){
      const S1=rotr(e,6)^rotr(e,11)^rotr(e,25);
      const ch=(e&f)^((~e)&g);
      const t1=(h+S1+ch+K[i]+w[i])>>>0;
      const S0=rotr(a,2)^rotr(a,13)^rotr(a,22);
      const maj=(a&b)^(a&c)^(b&c);
      const t2=(S0+maj)>>>0;
      h=g;g=f;f=e;e=(d+t1)>>>0;d=c;c=b;b=a;a=(t1+t2)>>>0;
    }
    H[0]=(H[0]+a)>>>0;H[1]=(H[1]+b)>>>0;H[2]=(H[2]+c)>>>0;H[3]=(H[3]+d)>>>0;
    H[4]=(H[4]+e)>>>0;H[5]=(H[5]+f)>>>0;H[6]=(H[6]+g)>>>0;H[7]=(H[7]+h)>>>0;
  }
  return Array.from(H,n=>n.toString(16).padStart(8,"0")).join("");
}
function validateOtaManifest(manifest){
  if(!manifest||typeof manifest!=="object")throw new Error("Invalid OTA manifest");
  if(!parseSemver(manifest.version))throw new Error("Invalid OTA version");
  if(manifest.firmware!=="firmware.bin")throw new Error("Invalid OTA firmware name");
  if(!Number.isInteger(manifest.size)||manifest.size<=0)throw new Error("Invalid OTA firmware size");
  if(!/^[0-9a-f]{64}$/i.test(String(manifest.sha256||"")))throw new Error("Invalid OTA SHA-256");
  return {version:manifest.version,firmware:"firmware.bin",size:manifest.size,sha256:String(manifest.sha256).toLowerCase()};
}
function uploadFirmwareToEsp(bytes,manifest,token){
  return new Promise((resolve,reject)=>{
    const form=new FormData();
    form.append("firmware",new Blob([bytes],{type:"application/octet-stream"}),"firmware.bin");
    const xhr=new XMLHttpRequest();
    xhr.open("POST",`/api/ota/upload?version=${encodeURIComponent(manifest.version)}&size=${manifest.size}&token=${encodeURIComponent(token)}`);
    xhr.timeout=120000;
    xhr.upload.onprogress=e=>{
      if(e.lengthComputable){
        const pct=Math.min(100,Math.round((e.loaded/e.total)*100));
        text("otaStatus",`${t("ota.upload")} · ${pct}%`);
      }
    };
    xhr.onload=()=>{
      let body={};
      try{body=JSON.parse(xhr.responseText||"{}");}catch(_){body={};}
      if(xhr.status>=200&&xhr.status<300&&body.ok!==false)resolve(body);
      else reject(new Error(body.error||`HTTP ${xhr.status}`));
    };
    xhr.onerror=()=>reject(new Error("Local ESP upload connection failed"));
    xhr.ontimeout=()=>reject(new Error("Local ESP upload timed out"));
    xhr.send(form);
  });
}
async function waitForOtaReboot(targetVersion){
  for(let attempt=0;attempt<20;attempt++){
    await new Promise(resolve=>setTimeout(resolve,1500));
    try{
      const next=await apiFetch("/api/state");
      if(next.firmwareVersion===targetVersion){location.reload();return;}
    }catch(_){}
  }
}
function fmtNumber(value,unit,decimals=1){
  if(typeof value!=="number" || !Number.isFinite(value)) return "--";
  return `${value.toLocaleString(lang==="de"?"de-DE":"en-US",{maximumFractionDigits:decimals})}${unit?` ${unit}`:""}`;
}
function fmtDateTime(value,timeZone){
  if(typeof value!=="string" || !value.trim()) return t("state.noMeasurement");
  // The external API may provide microseconds. Normalize to milliseconds for broad browser compatibility.
  const normalized=value.trim().replace(/(\.\d{3})\d+(?=Z$|[+-]\d{2}:\d{2}$)/,"$1");
  const date=new Date(normalized);
  if(Number.isNaN(date.getTime())) return value;
  const locale=lang==="de"?"de-DE":"en-GB";
  const options={day:"2-digit",month:"2-digit",year:"numeric",hour:"2-digit",minute:"2-digit",second:"2-digit",hour12:false};
  if(typeof timeZone==="string" && timeZone.trim()) options.timeZone=timeZone.trim();
  try{
    return new Intl.DateTimeFormat(locale,options).format(date);
  }catch(_){
    delete options.timeZone;
    return new Intl.DateTimeFormat(locale,options).format(date);
  }
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
  const apiValueTimingGroup=$("apiValueTimingGroup");
  if(metricGroup) metricGroup.classList.toggle("hidden",mode==="clock");
  if(alternateGroup) alternateGroup.classList.toggle("hidden",mode!=="alternate");
  if(apiValueTimingGroup) apiValueTimingGroup.classList.toggle("hidden",mode!=="alternate");
  if($("selectedMetric")) $("selectedMetric").disabled=mode==="clock";
  if($("alternateSeconds")) $("alternateSeconds").disabled=mode!=="alternate";
  if($("apiValueDisplayMs")) $("apiValueDisplayMs").disabled=mode!=="alternate";
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
  $("apiPort").value=settings.api.port||80;
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
  $("apiValueDisplayMs").value=settings.display.apiValueDisplayMs||1000;
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
  text("otaCurrent",state.firmwareVersion);
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
  badge("apiBadge",!configured?t("state.notConfigured"):api.requestInProgress?t("state.requesting"):api.connected?t("state.connected"):t("state.unavailable"),!configured?"neutral":api.requestInProgress?"neutral":api.connected?"good":"bad");
  text("apiServer",configured?api.endpoint:"--");
  text("apiLastRequest",api.lastAttemptAgeSeconds==null?"--":fmtAge(api.lastAttemptAgeSeconds));
  text("apiLastMeasurement",fmtDateTime(state.external.lastMeasurementAt,state.external.timezone));
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
      mode:$("displayMode").value,updateMs:Number($("displayUpdateMs").value),alternateSeconds:Number($("alternateSeconds").value),apiValueDisplayMs:Number($("apiValueDisplayMs").value)
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
      wifiSsid:$("wifiSsid").value.trim(),apiHost:$("apiHost").value.trim(),apiPort:Number($("apiPort").value),apiPollSeconds:Number($("apiPollSeconds").value),
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
  pendingOtaManifest=null;
  text("otaStatus",t("ota.manifest"));
  $("otaInstallBtn").disabled=true;
  try{
    const bytes=await fetchOtaBytes("manifest.json");
    const manifest=validateOtaManifest(JSON.parse(bytesToAscii(bytes)));
    const current=state?.firmwareVersion||"";
    const available=compareSemver(current,manifest.version)<0;
    pendingOtaManifest=available?manifest:null;
    text("otaCurrent",current||"--");
    text("otaLatest",manifest.version);
    text("otaSize",`${manifest.size} B`);
    text("otaStatus",available?t("state.updateAvailable"):t("state.upToDate"));
    $("otaInstallBtn").disabled=!available;
  }catch(err){
    text("otaStatus",`${t("common.failed")}: ${err.message}`);
  }
});
$("otaInstallBtn").addEventListener("click",async()=>{
  if(!pendingOtaManifest)return;
  otaUpdateActive=true;
  $("otaCheckBtn").disabled=true;
  $("otaInstallBtn").disabled=true;
  try{
    text("otaStatus",t("ota.firmware"));
    const bytes=await fetchOtaBytes("firmware.bin",pendingOtaManifest.version);
    if(bytes.length!==pendingOtaManifest.size)throw new Error(`Firmware size mismatch: ${bytes.length}/${pendingOtaManifest.size}`);
    if(bytes.length===0||bytes[0]!==0xE9)throw new Error("Downloaded file is not an ESP8266 firmware image");
    text("otaStatus",t("ota.verify"));
    await new Promise(resolve=>setTimeout(resolve,0));
    const hash=sha256Hex(bytes);
    if(hash!==pendingOtaManifest.sha256)throw new Error("Firmware SHA-256 mismatch");
    const session=await apiFetch("/api/ota/session",{method:"POST"});
    if(!session.token)throw new Error("OTA upload session could not be created");
    text("otaStatus",`${t("ota.upload")} · 0%`);
    await uploadFirmwareToEsp(bytes,pendingOtaManifest,session.token);
    text("otaStatus",t("state.restart"));
    await waitForOtaReboot(pendingOtaManifest.version);
  }catch(err){
    otaUpdateActive=false;
    $("otaCheckBtn").disabled=false;
    $("otaInstallBtn").disabled=false;
    text("otaStatus",`${t("common.failed")}: ${err.message}`);
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
  text("otaChannel","syschelle/espDisplay · ota · Browser");
  await loadState();
  setInterval(loadState,5000);
}
init();
})();

)ED8266";
