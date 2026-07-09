# urFileManager (urFM)

![Windows](https://img.shields.io/badge/Windows-0078D6?logo=windows&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)
![Fedora](https://img.shields.io/badge/Fedora-294172?logo=fedora&logoColor=white)
![Ubuntu](https://img.shields.io/badge/Ubuntu-E95420?logo=ubuntu&logoColor=white)
![Debian](https://img.shields.io/badge/Debian-A81D33?logo=debian&logoColor=white)
![MIT](https://img.shields.io/badge/License-MIT-green)

A cross-platform file organizer that sorts cluttered folders into categorized subdirectories — Images, Documents, Audio, Video, Archives — in seconds. Native C++ on Windows, Java on Linux.

## Features

- **Smart Extension Sorting** — Moves loose files into category folders based on customizable rules in `config.json`
- **Dry-Run Preview** — Preview every move before committing (enabled by default for safety)
- **PDF Reports** — Generate detailed organization reports with file names, sizes, and status
- **Full Audit Logging** — Every action recorded in `organizer.log` with timestamps
- **Conflict Resolution** — Duplicates renamed automatically (e.g. `report (1).pdf`)
- **Six UI Themes** — Midnight Dark, Minimalist Light, Red Sakura, Forest Emerald, Neon Cyberpunk, Obsidian Volt
- **Editable Config** — Add file types or categories via `config.json` — no recompile needed
- **GUI + CLI Modes** — Double-click for the GUI, or pass a folder path for scripting

## Project Structure

```
├── frontend-web/              # React + Vite marketing site
│   ├── src/
│   └── public/
├── desktop-windows/          # Desktop GUI applications
│   ├── gui_win32.cpp          # Windows native Win32 GUI (C++)
│   ├── gui_fltk.cpp           # Cross-platform FLTK GUI (C++, Linux)
│   ├── gui.cpp                # GUI redirect (platform dispatch)
│   ├── core.h / core.cpp      # Shared cross-platform logic
│   ├── build.bat              # Windows build script
│   ├── build.sh               # Linux build script
│   ├── ufmgr.bat              # Windows CLI wrapper
│   ├── run.bat                # Windows GUI launcher
│   ├── ufmgr.rc               # Windows resource file
│   └── ufmgr.manifest         # Windows manifest
├── desktop-linux/                # Linux Java Swing GUI (terminal aesthetic)
│   ├── src/urfm/              # Java sources
│   ├── build.sh               # Java build script
│   ├── MANIFEST.MF            # JAR manifest
│   └── RELEASE_README.md      # Quick start
├── organizer.py               # Python CLI (cross-platform)
├── config.json                # Sorting rules configuration
├── scripts/                   # Release automation
└── release/                   # Release binaries
```

## Workflow Diagram
![workflow-diagram](workflow.png)

## Quick Start

### Windows

1. Download `urfm-windows.zip` from the [website](https://urfilemanager.vercel.app) or via CLI:

```powershell
# PowerShell
Invoke-WebRequest -Uri "https://urfilemanager.vercel.app/urfm-windows.zip" -OutFile "urfm-windows.zip"
```

```cmd
curl -L -o urfm-windows.zip "https://urfilemanager.vercel.app/urfm-windows.zip"
```

2. Extract anywhere
3. Double-click `run.bat` to launch the GUI, or use:

```powershell
.\ufmgr.exe C:\Downloads --dry-run
```

### Linux (Fedora RPM — recommended)

1. Download the `urfm-1.0.0-1.noarch.rpm` package.
2. Install the package:

```bash
cd desktop-linux
./build.sh                             # compile (requires JDK 17+)
./urfm                                 # launch GUI
./urfm ~/Downloads --dry-run           # preview
./urfm ~/Downloads                     # execute
./urfm ~/Downloads --revert            # undo
```

Install via RPM or DEB for system-wide access.

### Python (any OS)

```bash
pip install -r requirements.txt
python organizer.py ~/Downloads --dry-run
python organizer.py ~/Downloads
python organizer.py ~/Downloads --revert
```

## Project Structure

```
├── desktop-windows/           # Windows C++ app (Win32 GUI + CLI)
│   ├── cli.cpp                # CLI entry point
│   ├── gui_app.cpp            # Win32 GUI entry point
│   ├── urfm_common.h/.cpp     # Shared engine (config, PDF, revert)
│   ├── config.json            # Category-to-extension mapping
│   ├── build.bat              # Build script (MinGW-w64)
│   ├── ufmgr.exe / ufmgr-cli.exe  # Compiled binaries
│   └── windows_usage.md       # Full usage guide
├── desktop-linux/             # Linux Java Swing GUI
│   ├── src/urfm/              # Java sources
│   ├── build.sh               # Build script
│   └── RELEASE_README.md      # Quick start
├── frontend-web/              # React + Vite marketing site
├── organizer.py               # Python CLI (cross-platform)
├── config.json                # Sorting rules (shared)
└── scripts/                   # Release automation
```

## Building from Source

### Windows (native Win32 GUI)

Requires MinGW-w64 with `windres`.

```cmd
cd desktop-windows
build.bat
```

### Linux — Java Terminal Edition

```bash
cd desktop-linux
chmod +x build.sh
./build.sh
# Produces urfm.jar + urfm launcher
```

### Linux — FLTK GUI (alternative)

```bash
cd desktop-windows
chmod +x build.sh
./build.sh
```

### Python (cross-platform CLI)

Works on all platforms without compilation.

```bash
pip install tqdm
python organizer.py ~/Downloads
```

## Releasing (website downloads)

Release archives must live in `frontend-web/public/` so Vite copies them into the deployed site.

```powershell
.\scripts\package-release.ps1   # builds Windows zip + Linux tarball
```

## License

MIT
