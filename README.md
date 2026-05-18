# Suzu — FFmpeg Exporter Plugin for Adobe Premiere Pro

<p align="center"><img src="assets/suzu.svg" width="120" alt="Suzu logo" /></p>

A native Premiere Pro exporter plugin (`.prm`) that pipes raw video and audio directly to FFmpeg for encoding, with a configurable **FFmpegFreeUI** front-end.

**Repository:** https://github.com/Cloud-FeiYang/Suzu  
**Latest release:** [26w21e](https://github.com/Cloud-FeiYang/Suzu/releases/tag/26w21e) — `Suzu-26w21e-x64-Setup.exe`

> **Upgrading from FFmAdobe?** Install Suzu over the old build or run the new installer. Presets and paths under `%APPDATA%\FFmAdobe` are migrated automatically to `%APPDATA%\Suzu`; registry keys under `HKLM\SOFTWARE\FFmAdobe` are still read as a fallback.

## Features

- **Zero intermediate files** — Windows named pipes stream raw BGRA video and F32LE audio to a single `ffmpeg.exe` process
- **Full Unicode path support** — `CreateProcessW` for CJK and special-character output paths
- **FFmpegFreeUI integration** — dedicated export-panel tab with a **Configure** button that launches FFmpegFreeUI
- **Preset pipeline** — full FFmpegFreeUI presets are converted to FFmpeg CLI args via PremierePresetConverter
- **Threaded audio pipeline** — audio renders on a background thread while video frames are piped
- **Automatic frame orientation fix** — `vflip` corrects Premiere's bottom-up BGRA buffer
- **Pre-export validation** — short dummy encode using your real resolution before the main export starts

## Architecture

```
Premiere Pro                       PremierePresetConverter
    │                                      │
    │  "Configure" button                  │  FFmpegFreeUI preset JSON
    │  → launches FFmpegFreeUI             │  → simplified JSON
    │  → user saves preset                 │  (video_args, audio_args, filters...)
    │                                      ▼
    │                            %APPDATA%\Suzu\premiere_preset.json
    │                                      │
    ├─ Main thread (video frames)          │
    │     └──► \\.\pipe\ffmpeg_video_{PID}  │
    │                                    ╲ │
    └─ Background thread (audio)          ╲▼
          └──► \\.\pipe\ffmpeg_audio_{PID} ──► ffmpeg.exe ──► output file
```

## Requirements

- **Adobe Premiere Pro** 22.0+ (tested with 26.0)
- **Adobe Premiere Pro C++ SDK** (26.0) — build only
- **FFmpeg** on system `PATH`
- **Visual Studio Build Tools** (MSVC) — build only
- **Inno Setup 6** (`iscc`) — installer build only

## Download

Pre-built installer (plugin + PremierePresetConverter + FFmpegFreeUI):

| Release | Installer |
|---------|-----------|
| [26w21e](https://github.com/Cloud-FeiYang/Suzu/releases/tag/26w21e) | `Suzu-26w21e-x64-Setup.exe` |

Older snapshots (`26w21d`, `26w21c`, …) remain on the [Releases](https://github.com/Cloud-FeiYang/Suzu/releases) page under the previous **FFmAdobe** naming.

## Install

### Option 1: Installer (recommended)

1. Download `Suzu-26w21e-x64-Setup.exe` from [Releases](https://github.com/Cloud-FeiYang/Suzu/releases/tag/26w21e).
2. Run the installer and select components (Plugin / Converter / FFmpegFreeUI).
3. Ensure `ffmpeg.exe` is on your system `PATH`.
4. In Premiere: **File → Export**, choose **Suzu (26w21e)**, open the **FFmpegFreeUI** tab, click **Configure**, then export.

### Option 2: Manual

Copy into `C:\Program Files\Adobe\Common\Plug-ins\7.0\MediaCore\Suzu\`:

```
Suzu.prm
PremierePresetConverter.exe
```

Place `FFmpegFreeUI.exe` anywhere and set:

```
HKLM\SOFTWARE\Suzu\FFmpegFreeUIPath = "C:\path\to\FFmpegFreeUI.exe"
```

## Build from source

```powershell
# 1. C++ plugin
cmake -B build -G "Visual Studio 18 2026" `
      -DADOBE_PP_SDK="E:/Code/Premiere Pro 26.0 C++ SDK"
cmake --build build --config Release
# → build/Release/Suzu.prm

# 2. PremierePresetConverter
dotnet publish src/PremierePresetConverter -c Release -r win-x64 `
  --self-contained true -p:PublishSingleFile=true `
  -o installer/staging/Plugin

# 3. FFmpegFreeUI (bundled UI)
dotnet publish FFmpegFreeUI/FFmpegFreeUI -c Release -r win-x64 `
  --self-contained true -p:PublishSingleFile=true -p:PublishReadyToRun=false `
  -o installer/staging/FFmpegFreeUI

# 4. Stage plugin + installer
Copy-Item build/Release/Suzu.prm installer/staging/Plugin/ -Force
cd installer
iscc Suzu.iss
# → installer/output/Suzu-26w21e-x64-Setup.exe
```

Version strings: snapshot `26w21e` in `CMakeLists.txt` (`SUZU_SNAPSHOT_VERSION`), `installer/Suzu.iss`, and `src/Suzu/Suzu.cpp`.

## Preset format

Simplified JSON written by PremierePresetConverter:

```json
{
  "version": "1",
  "video_args": "-c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p",
  "audio_args": "-c:a aac -b:a 320k",
  "video_filters": "hqdn3d=luma_spatial=4",
  "audio_filters": "",
  "container": ".mp4",
  "extra_input_args": ""
}
```

Default when no preset is configured: H.264 CRF 18 (fast preset) + AAC 320 kbps.

## Project structure

```
Suzu/
├── CMakeLists.txt
├── README.md
├── release_notes.txt
├── assets/                              # Brand icons
├── FFmpegFreeUI/                        # git subtree (upstream + Suzu integration patches)
├── src/
│   ├── Suzu/                            # C++ Premiere Pro plugin (.prm)
│   └── PremierePresetConverter/         # .NET preset converter
└── installer/
    ├── Suzu.iss
    ├── ChineseSimplified.isl
    └── License.rtf
```

## Syncing FFmpegFreeUI upstream

`FFmpegFreeUI/` is a **git subtree** from https://github.com/Lake1059/FFmpegFreeUI.git. Suzu-specific changes (Premiere mode, AppData migration) live in that subtree; adapter logic for preset conversion lives in `src/PremierePresetConverter/`.

```powershell
git subtree pull --prefix=FFmpegFreeUI ffmpegfreeui main --squash

# If the remote is missing:
git remote add ffmpegfreeui https://github.com/Lake1059/FFmpegFreeUI.git
```

## Versioning

Release tags use snapshot IDs: `YYwWWx` (e.g. `26w21e` = 2026, week 21, build `e`). Development happens on `dev`; version branches (e.g. `26w21e`) track release lines.

## Acknowledgments

### [FFmpegFreeUI](https://github.com/Lake1059/FFmpegFreeUI) by [@Lake1059](https://github.com/Lake1059)

> 3FUI 是 FFmpeg 在 Windows 上的轻度专业交互外壳，收录大量参数，界面美观，交互友好。

FFmpegFreeUI provides the encoding UI and preset management. Integrated via `git subtree`.

- **License:** [MIT](https://github.com/Lake1059/FFmpegFreeUI/blob/main/LICENSE.txt)
- **Upstream:** https://github.com/Lake1059/FFmpegFreeUI

## License

MIT — see [LICENSE](LICENSE). FFmpegFreeUI is also MIT-licensed.
