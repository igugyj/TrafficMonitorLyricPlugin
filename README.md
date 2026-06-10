# LyricPlugin

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 的歌词显示插件，实时获取 LX Music 正在播放的歌词并渲染到任务栏。

## 功能

- 通过 LX Music Web API 实时获取歌词、状态、歌曲信息
- 大字体 CustomDraw 渲染，支持自定义字体和字号（小/中/大）
- 歌词过长时自动滚动，首尾各停顿 500ms
- 自动宽度：刚好显示完整歌词，无需滚动
- 固定宽度：超过部分截断或滚动
- 暂停/停止/无信号时自动隐藏插件
- 鼠标悬停 ToolTip 显示歌曲名 - 歌手
- 所有设置保存到 `.ini` 文件

## 下载

在 [Releases](https://github.com/igugyj/TrafficMonitor-LyricPlugin/releases) 页面下载最新版本，解压到 TrafficMonitor 的插件目录即可。

## 构建

### 依赖

| 依赖                                                             | 版本     | 说明                                  |
| ---------------------------------------------------------------- | -------- | ------------------------------------- |
| [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) | 主程序   | 插件宿主机                            |
| [LX Music](https://github.com/lyswhut/lx-music-desktop)          | 歌词源   | 需在设置中启用 Web API                |
| Visual Studio 2022                                               | 编译器   | 需安装 v143 工具集                    |
| C++ MFC for v143                                                 | VS 组件  | `C++ MFC for latest v143 build tools` |
| Windows SDK                                                      | 系统组件 | 随 VS 安装                            |

### 第三方库

| 库                                                           | 说明                                                       |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| [nlohmann/json](https://github.com/nlohmann/json) (json.hpp) | 已内置到 `src/thirdparty/`，单头文件，无需额外下载配置 MIT |

### LX Music 配置

LX Music → 设置 → 启用 Web API，默认端口 **23330**。

### 步骤

```bash
git clone https://github.com/igugyj/TrafficMonitorLyricPlugin.git
cd TrafficMonitorLyricPlugin
msbuild LyricPlugin.vcxproj /p:Configuration=Debug /p:Platform=x64
```

输出：`Bin\x64\Debug\plugins\LyricPlugin.dll`

复制到 TrafficMonitor 插件目录即可加载。

## 项目结构

```txt
src/
  core/          核心逻辑（LX Music API、数据管理）
  compat/        TrafficMonitor 兼容层（插件适配接口）
  thirdparty/    第三方库（json.hpp）
```

## 设置项

| 字段           | 默认值             | 说明                          |
| -------------- | ------------------ | ----------------------------- |
| `port`         | 23330              | LX Music Web API 端口         |
| `item_width`   | 0                  | 项目宽度，0=自动，>0=固定像素 |
| `font_name`    | Microsoft YaHei UI | 渲染字体                      |
| `font_size`    | 3                  | 2=小，3=中，4=大              |
| `scroll_speed` | 30                 | 滚动速度(px/s)，0=不滚动      |

## 相关项目

- [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) — 任务栏监控工具
- [TrafficMonitor Plugins](https://github.com/zhongyang219/TrafficMonitorPlugins) — 官方插件仓库
- [LX Music](https://github.com/lyswhut/lx-music-desktop) — 洛雪音乐助手

## License

MIT
