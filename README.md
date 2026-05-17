# FFmAdobe — FFmpeg Exporter Plugin for Adobe Premiere Pro

A native Premiere Pro exporter plugin (`.prm`) that pipes raw video and audio directly to FFmpeg for encoding, with a configurable **FFmpegFreeUI** front-end.

## Features

- **Zero intermediate files** — uses Windows named pipes to stream raw BGRA video and F32LE audio directly to a single FFmpeg process
- **Full Unicode path support** — `CreateProcessW` for CJK/special character output paths
- **FFmpegFreeUI integration** — dedicated tab in Premiere's export panel with a "Configure" button that launches the FFmpegFreeUI configuration tool
- **Plugin-relative FFmpegFreeUI** — automatically finds `FFmpegFreeUI.exe` next to the `.prm` file (no system PATH needed)
- **Threaded audio pipeline** — audio rendering runs in a background thread, parallel with video frame piping
- **Automatic frame orientation fix** — `vflip` filter corrects Premiere's bottom-up BGRA buffer

## Architecture

```
Premiere Pro
    ├─ Main thread (video frames)
    │     └──► \\.\pipe\ffmpeg_video_{PID}
    │                                    ╲
    └─ Background thread (audio)          ╲
          └──► \\.\pipe\ffmpeg_audio_{PID} ──► ffmpeg.exe ──► output.mp4
```

## Requirements

- **Adobe Premiere Pro** 22.0+ (tested with 26.0)
- **Adobe Premiere Pro C++ SDK** (26.0)
- **FFmpeg** in system PATH
- **FFmpegFreeUI** (optional, for encoding configuration UI)
- **Visual Studio 2022** Build Tools (MSVC v143+)

## Build

```powershell
# Configure
cmake -B build -G "Visual Studio 18 2025" `
      -DADOBE_PP_SDK="E:/Code/Premiere Pro 26.0 C++ SDK"

# Build
cmake --build build --config Release
```

Output: `build/Release/FFmpegExporter.prm`

## Install

Copy these files to `C:\Program Files\Adobe\Common\Plug-ins\7.0\MediaCore\`:

```
FFmpegExporter.prm      # The plugin
FFmpegFreeUI.exe        # Configuration UI (optional, placed next to .prm)
```

## Preset Format

FFmpegFreeUI saves presets as `.3fuipreset` JSON files:

```json
{
  "video_args": "-c:v libx264 -preset slow -crf 15 -pix_fmt yuv420p",
  "audio_args": "-c:a aac -b:a 320k"
}
```

If no preset is configured, defaults to: `H.264 CRF 18 fast + AAC 320kbps`.

## Project Structure

```
FFmAdobe/
├── CMakeLists.txt                    # Root build configuration
├── src/
│   ├── configure.h.in                # Build config template
│   └── FFmpegExporter/
│       ├── CMakeLists.txt            # Plugin build target
│       ├── FFmpegExporter.cpp        # Main export logic (pipe architecture)
│       ├── FFmpegExporter.h          # Plugin entry point declarations
│       ├── FFmpegExporter_Params.cpp # UI parameters & FFmpegFreeUI launcher
│       ├── FFmpegExporter_Params.h   # Parameter IDs and string definitions
│       ├── FFmpegExporter.rc         # Windows resource file
│       └── FFmpegExporter_export.h   # DLL export macros (auto-generated)
└── README.md
```

## License

MIT
