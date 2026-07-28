# LyricPlugin — Agent guide

Compact C++/MFC DLL plugin for TrafficMonitor that fetches lyrics from LX Music Web API.

## Build

```powershell
msbuild LyricPlugin.vcxproj /p:Configuration=Debug /p:Platform=x64
```

Output: `Bin\x64\Debug\plugins\LyricPlugin.dll`.  
Configurations: Debug|Release × x64|x86. Toolset v143, VS 2022, Unicode, MFC dynamic.  
No tests, lint, formatters, typecheck, or CI exist.

**Build number** (`VER_BUILD`): auto-injected as `YYDDD` (ordinal date) by `ComputeBuildNumber` MSBuild target. Version in `.dll` properties = `major.minor.patch.YYDDD`.

## Architecture

- **Entrypoint**: `TMPluginGetInstance()` at `src/compat/LyricPlugin.cpp:185` — returns singleton `CLyricPlugin::m_instance`.
- **Data flow**: `DataRequired()` → `CDataManager::FetchLyric()` → `CLXMusicAPI::FetchStatus()` → HTTP GET `127.0.0.1:23330/status` → JSON parsed with nlohmann/json (bundled `src/thirdparty/json.hpp`).
- **Rendering**: `CLyricItem` uses `IsCustomDraw()=true`. Auto-width or fixed-width with scroll.
- **Scroll animation**: idle 500ms → scroll at `scroll_speed` px/s → pause 500ms → reset.
- **Config**: Auto-derived from DLL filename (`LyricPlugin.ini`), written via Win32 INI APIs. Directory supplied by TrafficMonitor via `OnExtenedInfo(EI_CONFIG_DIR)`. Destructor `CDataManager::~CDataManager()` calls `SaveConfig()`.
  - **Hot-reload**: `ShowOptionsDialog()` calls `SaveConfig()` then `ApplySettings()` to push port change to the HTTP client immediately. Without `ApplySettings()`, the old port persists until restart.
- **Font size**: encoded as `2` (small, 0.6×height), `3` (medium, 0.75×), `4` (large, 0.9×). Default: `3`.

## Key conventions

- Uses `__ImageBase` linker symbol for module path resolution.
- Dialog uses `DialogBoxParam` with `SettingData*` as `lParam`. Font browse uses `ChooseFont` Win32 dialog.
- Dark mode text: `RGB(230,230,230)`, light mode: `RGB(30,30,30)`.
- Precompiled header in `src/core/pch.h` — include that first in all `.cpp` files.
- Only dependency outside Windows SDK: nlohmann/json (single header, no package manager).
