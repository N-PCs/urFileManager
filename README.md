# urFileManager (urFM)

A cross-platform, native C++ utility that organizes cluttered folders into categorized subdirectories — Images, Documents, Audio, Video, Archives — in seconds. Comes with a polished GUI, CLI mode, dry-run preview, PDF reports, and 5 stunning themes.

**Platforms:** Windows (native Win32 GUI) · Linux (FLTK GUI) · macOS (FLTK GUI)

## Features

- **Smart Extension Sorting** — Moves loose files into category folders based on customizable rules in `config.json`
- **Dry-Run Preview** — Preview every move before committing (enabled by default for safety)
- **PDF Reports** — Generate detailed organization reports with file names, sizes, and status
- **Full Audit Logging** — Every action recorded in `organizer.log` with timestamps
- **Conflict Resolution** — Duplicates renamed automatically (e.g. `report (1).pdf`)
- **Five UI Themes** — Midnight Dark, Minimalist Light, Nordic Frost, Forest Emerald, Neon Cyberpunk
- **Editable Config** — Add file types or categories via `config.json` — no recompile needed
- **GUI + CLI Modes** — Double-click for the GUI, or pass a folder path for scripting

## Project Structure

```
├── frontend-web/              # React + Vite marketing site
│   ├── src/
│   └── public/
├── frontend-desktop/          # Desktop GUI applications
│   ├── gui_win32.cpp          # Windows native Win32 GUI (C++)
│   ├── gui_fltk.cpp           # Cross-platform FLTK GUI (C++, Linux/macOS)
│   ├── gui.cpp                # GUI redirect (platform dispatch)
│   ├── core.h / core.cpp      # Shared cross-platform logic
│   ├── build.bat              # Windows build script
│   ├── build.sh               # Linux build script
│   ├── build_mac.sh           # macOS build script
│   ├── organizer.bat          # Windows CLI wrapper
│   ├── run.bat                # Windows GUI launcher
│   ├── organizer.rc           # Windows resource file
│   └── organizer.manifest     # Windows manifest
├── organizer.py               # Python CLI (cross-platform)
├── config.json                # Sorting rules configuration
├── scripts/                   # Release automation
└── release/                   # Release binaries
```

## Quick Start

### Windows

1. Download `urfm-windows.zip` from the website
2. Extract anywhere
3. Double-click `run.bat` to launch the GUI, or use:

```powershell
.\organizer.exe C:\Downloads --dry-run
```

### Linux (Fedora / Ubuntu / Arch)

```bash
# Install FLTK dependency
sudo apt install libfltk1.3-dev   # Ubuntu
sudo dnf install fltk-devel       # Fedora

# Build and run
chmod +x build.sh
./build.sh
./urfm ~/Downloads --dry-run
```

### macOS

```bash
# Install FLTK via Homebrew
brew install fltk

# Build and run
chmod +x build_mac.sh
./build_mac.sh
./urfm ~/Downloads --dry-run
```

## Building from Source

### Windows (native Win32 GUI)

Requires MinGW-w64 with `windres`.

```cmd
cd frontend-desktop
build.bat
```

### Linux (FLTK GUI)

```bash
cd frontend-desktop
chmod +x build.sh
./build.sh
```

### macOS (FLTK GUI)

```bash
cd frontend-desktop
chmod +x build_mac.sh
./build_mac.sh
```

### Python (cross-platform CLI)

Works on all platforms without compilation.

```bash
pip install tqdm
python organizer.py ~/Downloads
```

## Contributor

Made with ❤️ by @N-PCs 

## License

MIT
