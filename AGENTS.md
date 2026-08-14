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
- **Data flow**: `DataRequired()` → `CDataManager::FetchLyric()` → `CLXMusicAPI::FetchStatus()` → HTTP GET `127.0.0.1:23330/status` → JSON parsed with nlohmann/json (bundled `src/thirdparty/json.hpp`).
- **Rendering**: `CLyricItem` uses `IsCustomDraw()=true`. Auto-width or fixed-width with scroll.
- **Scroll animation**: idle 500ms → scroll at `scroll_speed` px/s → pause 500ms → reset.
- **Config**: Auto-derived from DLL filename (`LyricPlugin.ini`), written via Win32 INI APIs. Directory supplied by TrafficMonitor via `OnExtenedInfo(EI_CONFIG_DIR)`. Destructor `CDataManager::~CDataManager()` calls `SaveConfig()`.
  - **Hot-reload**: `ShowOptionsDialog()` calls `SaveConfig()` then `ApplySettings()` to push port change to the HTTP client immediately. Without `ApplySettings()`, the old port persists until restart.
- **Font size**: encoded as `2` (small, 0.6×height), `3` (medium, 0.75×), `4` (large, 0.9×). Default: `3`.
- **API v7/v8 dual support**: Bundled `src/compat/PluginInterface.h` is synced verbatim from upstream master (API **v8**, UTF-8 BOM). The plugin reports v8 to any host; on v7 hosts (e.g. release ≤ v1.86) the extra vtables are simply never called — safe degradation. `CLyricItem::IsDoubleLineExclusive()` returns 1 when the `double_line_exclusive` setting (INI key `double_line_exclusive`, default **1**) is on — v8 hosts then give the item a full-height cell in grouped/side taskbar layouts. `DrawItemEx` is intentionally NOT overridden (host falls back to `DrawItem`).

## Key conventions

- Uses `__ImageBase` linker symbol for module path resolution.
- Dialog uses `DialogBoxParam` with `SettingData*` as `lParam`. Font browse uses `ChooseFont` Win32 dialog.
- `src/compat/LyricPlugin.rc` is **UTF-16 LE** (BOM `FF FE`) with Chinese strings; never edit with plain text tools — modify via PowerShell byte-safe transforms (read/write with `[System.Text.Encoding]::Unicode`, build Chinese strings from `[char[]]` code points, e.g. `-join ([char[]]@(0x5E03,0x5C40))`). Do NOT wrap in a PowerShell function taking `[int[]]` — positional binding drops all but the first element.
- Dark mode text: `RGB(230,230,230)`, light mode: `RGB(30,30,30)`.
- Precompiled header in `src/core/pch.h` — include that first in all `.cpp` files.
- Only dependency outside Windows SDK: nlohmann/json (single header, no package manager).
