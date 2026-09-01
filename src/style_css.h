#pragma once

#include <Arduino.h>

const char STYLE_CSS[] PROGMEM = R"ED8266(
:root{
  --header:#2c3e50;--side:#34495e;--bg:#f5f5f5;--text:#333;--border:#ddd;--surface:#fff;
  --link:#2c3e50;--sidebar-w:210px;--radius:10px;--good:#19733b;--warn:#9a6700;--bad:#b42318;
  --shadow:0 2px 8px rgba(0,0,0,.08);
}
:root[data-theme="dark"]{
  --header:#111;--side:#1b1b1b;--bg:#121212;--text:#eaeaea;--border:#303030;--surface:#1f1f1f;
  --link:#9ec1ff;--good:#61d58b;--warn:#f4c150;--bad:#ff8b82;--shadow:0 2px 8px rgba(0,0,0,.3);
}
*{box-sizing:border-box}
html{font-size:16px}
body{margin:0;font-family:system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif;background:var(--bg);color:var(--text);line-height:1.45}
button,input,select{font:inherit}
code{overflow-wrap:anywhere}
.header{position:sticky;top:0;height:56px;display:flex;align-items:center;justify-content:space-between;gap:.75rem;padding:0 16px;background:var(--header);color:#fff;z-index:50}
.title{font-weight:650;font-size:clamp(1rem,.6vw+.9rem,1.2rem);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.datetime{display:flex;flex-direction:column;align-items:flex-end;font-size:.85rem;line-height:1.2;min-width:105px}
.hamburger{width:40px;height:40px;display:inline-flex;align-items:center;justify-content:center;background:transparent;border:0;color:inherit;cursor:pointer;border-radius:var(--radius);font-size:22px}
.hamburger:focus-visible{outline:2px solid #fff;outline-offset:2px}
.sidebar{position:fixed;inset:0 auto 0 0;width:var(--sidebar-w);background:var(--side);color:#fff;transform:translateX(-100%);transition:transform .25s ease;padding-top:62px;z-index:40}
.sidebar--open{transform:translateX(0)}
.navlink{display:block;padding:12px 18px;color:#fff;text-decoration:none;cursor:pointer}
.navlink:hover{background:rgba(255,255,255,.08)}
.navlink[aria-current="page"]{background:rgba(255,255,255,.14)}
.overlay{position:fixed;inset:0;background:rgba(0,0,0,.35);opacity:0;visibility:hidden;pointer-events:none;transition:.25s;z-index:20}
.overlay--show{opacity:1;visibility:visible;pointer-events:auto}
.content{padding:clamp(14px,1.5vw,24px);max-width:1400px}
.page{display:none;animation:fade .18s}
.page.active{display:block}
@keyframes fade{from{opacity:0}to{opacity:1}}
h1{font-size:clamp(1.35rem,1.2vw + 1rem,1.9rem);margin:0 0 .5rem}
h2{font-size:1.05rem;margin:1.4rem 0 .75rem}
.hint{opacity:.72;font-size:.92rem}
.page-heading,.card-head,.actions,.switch-row{display:flex;align-items:center;justify-content:space-between;gap:12px}
.hero-card,.card,.metric-card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);box-shadow:var(--shadow)}
.hero-card{padding:18px;display:flex;align-items:flex-end;justify-content:space-between;gap:16px;margin:14px 0}
.eyebrow{font-size:.82rem;text-transform:uppercase;letter-spacing:.06em;opacity:.65}
.hero-value{font-size:clamp(2.4rem,6vw,4.5rem);font-weight:750;line-height:1}
.hero-label{margin-top:8px;font-size:1rem;opacity:.75}
.dashboard-grid,.metric-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));gap:14px}
.card{padding:16px}
.metric-card{padding:14px;display:flex;align-items:flex-end;justify-content:space-between;gap:12px;min-height:82px}
.metric-card span{opacity:.72}
.metric-card strong{font-size:1.35rem;text-align:right}
.section-title{margin:22px 0 9px;font-weight:700}
.badge{display:inline-flex;align-items:center;border-radius:999px;padding:4px 9px;font-size:.78rem;font-weight:650;border:1px solid var(--border)}
.badge.good{color:var(--good);border-color:currentColor}
.badge.warn{color:var(--warn);border-color:currentColor}
.badge.bad{color:var(--bad);border-color:currentColor}
.badge.neutral{opacity:.7}
.kv{margin:0}
.kv>div{display:grid;grid-template-columns:minmax(120px,1fr) minmax(120px,1.35fr);gap:12px;padding:7px 0;border-bottom:1px solid var(--border)}
.kv>div:last-child{border-bottom:0}
.kv dt{opacity:.65}
.kv dd{margin:0;text-align:right;overflow-wrap:anywhere}
.kv>div.server-row{grid-template-columns:minmax(90px,.7fr) minmax(0,1.8fr)}
.single-line-value{white-space:nowrap;overflow:hidden;text-overflow:ellipsis;overflow-wrap:normal!important;min-width:0}
.form-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(230px,1fr));gap:14px}
.form-group{margin-bottom:14px}
.form-group.wide{grid-column:span 2}
label{display:block;margin-bottom:6px}
input,select{width:100%;padding:10px;border:1px solid var(--border);border-radius:var(--radius);background:var(--surface);color:var(--text)}
.switch-row{justify-content:flex-start;margin:6px 0 18px}
.switch-row input{width:auto;transform:scale(1.2)}
.primary,.btn{border-radius:8px;cursor:pointer;transition:transform .15s,filter .15s}
.primary{padding:10px 14px;border:0;background:var(--header);color:#fff;box-shadow:0 2px 6px rgba(0,0,0,.15)}
.primary:hover,.btn:hover{transform:translateY(-1px);filter:brightness(1.05)}
.primary:disabled{opacity:.45;cursor:not-allowed;transform:none}
.primary.fit{width:auto}
.btn{padding:7px 10px;border:1px solid var(--border);background:var(--surface);color:var(--text)}
.danger{background:#9f1c16}
.danger-outline{color:var(--bad);border-color:var(--bad)}
.form-status{min-height:1.4em;margin-top:10px;font-size:.92rem}
.weblog{margin-top:12px;background:#000;color:#00ff00;font-family:"Courier New",monospace;font-size:.86rem;line-height:1.4;padding:12px;border-radius:8px;border:1px solid #0f0;height:430px;overflow:auto;white-space:pre-wrap}
.warning-box{border-left:4px solid var(--bad);padding:12px;background:color-mix(in srgb,var(--bad) 8%,var(--surface));border-radius:6px}
.hidden{display:none!important}
@media(min-width:1024px){
  .hamburger{display:none}
  .sidebar{transform:none;position:sticky;inset:auto;top:56px;height:calc(100dvh - 56px);padding-top:6px}
  .layout{display:grid;grid-template-columns:var(--sidebar-w) 1fr;min-height:calc(100dvh - 56px)}
  .overlay{display:none}
  .content{padding:clamp(18px,2vw,32px)}
}
@media(max-width:620px){
  .datetime{font-size:.75rem}
  .hero-card{align-items:flex-start;flex-direction:column}
  .kv>div{grid-template-columns:1fr}
  .kv dd{text-align:left}
  .form-group.wide{grid-column:auto}
  .stack-mobile{align-items:stretch;flex-direction:column}
  .stack-mobile .fit{width:100%}
  .page-heading{align-items:flex-start}
}

)ED8266";
