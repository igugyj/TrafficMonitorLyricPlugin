# LyricPlugin — Agent guide

Compact C++/MFC DLL plugin for TrafficMonitor that fetches lyrics from LX Music Web API.

## Build

```powershell
msbuild LyricPlugin.vcxproj /p:Configuration=Debug /p:Platform=x64
```

Output: `Bin\x64\Debug\plugins\LyricPlugin.dll`.  
Configurations: Debug|Release × x64|x86. Toolset v143, VS 2022, Unicode, MFC dynamic.  
No tests, lint, formatters, or typecheck exist.

**Build number** (`VER_BUILD`): auto-injected as `YYDDD` (ordinal date) by `ComputeBuildNumber` MSBuild target. Version in `.dll` properties = `major.minor.patch.YYDDD`.

## CI/CD

Two GitHub Actions workflows in `.github/workflows/`:
- **dist.yml** — manual dispatch, builds x64+x86, force-pushes DLL+PDB to `dist` branch
- **release.yml** — manual dispatch, builds x64+x86, creates GitHub Release with auto-generated notes and version compare link

## Architecture

- **Entrypoint**: `TMPluginGetInstance()` at `src/compat/LyricPlugin.cpp:185` — returns singleton `CLyricPlugin::m_instance`.
- **Data flow**: `DataRequired()` → `CDataManager::FetchLyric()` → `CLXMusicAPI::FetchStatus()` → HTTP GET `127.0.0.1:23330/status` → JSON parsed with nlohmann/json (bundled `src/thirdparty/json.hpp`). On failure, lyrics clear after 2 consecutive failed fetches (~2 s, `m_fail_count`), so the item hides when the API goes silent.
- **Rendering**: `CLyricItem` uses `IsCustomDraw()=true`. Auto-width or fixed-width with scroll. `FetchLyric()` only fills a pending buffer; `GetItemWidthEx` (the host's 1 Hz width-sampling point) calls `CommitPending()` so the width change and the new lyric render in the same host frame — no clipped intermediate frames, width always matches the line shown. Auto width = exact line width, capped at 480px; returns 0 when not playing (item hidden). Trade-off: lyric display lags up to ~1 s (committed only at the 1 Hz sample).
- **Scroll animation**: idle 500ms → scroll at `scroll_speed` px/s → pause 500ms → reset.
- **Config**: Fixed filename `LyricPlugin.ini` (decoupled from DLL filename — renaming the DLL keeps settings), written via Win32 INI APIs. Directory supplied by TrafficMonitor via `OnExtenedInfo(EI_CONFIG_DIR)`; falls back to the DLL's own directory. Destructor `CDataManager::~CDataManager()` calls `SaveConfig()`.
  - **Keys**: `port` (23330), `item_width` (0=auto), `font_size` (2-4), `scroll_speed` (30), `timeout` (3000ms), `font_name`. All keys have defaults and are clamped after load (invalid/out-of-range values fall back to defaults; note `GetPrivateProfileInt` returns 0 — not the default — when a value is unparseable).
  - **Hot-reload**: `ShowOptionsDialog()` calls `SaveConfig()` then `ApplySettings()` to push port/timeout to the HTTP client immediately. Without `ApplySettings()`, the old values persist until restart.
- **HTTP timeouts**: `CLXMusicAPI::HttpGet` applies the configurable `timeout` to all four WinHTTP timeouts (resolve/connect/send/receive) via `WinHttpSetTimeouts`.
- **Font size**: encoded as `2` (small, 0.6×height), `3` (medium, 0.75×), `4` (large, 0.9×). Default: `3`.

## Key conventions

- Uses `__ImageBase` linker symbol for module path resolution.
- Dialog uses `DialogBoxParam` with `SettingData*` as `lParam`. Font browse uses `ChooseFont` Win32 dialog.
- Dark mode text: `RGB(230,230,230)`, light mode: `RGB(30,30,30)`.
- Precompiled header in `src/core/pch.h` — include that first in all `.cpp` files.
- Only dependency outside Windows SDK: nlohmann/json (single header, no package manager).
