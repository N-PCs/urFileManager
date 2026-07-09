# urFileManager (urFM)

A cross-platform utility that organizes cluttered folders into categorized subdirectories — Images, Documents, Audio, Video, Archives — in seconds. Comes with a native Win32 GUI (Windows) and Java Swing GUI (Linux), plus CLI mode, dry-run preview, PDF reports, and customizable themes.

**Platforms:** Windows (Native C++ Win32 GUI) · Linux (Java Swing GUI with Fedora RPM & Ubuntu DEB support)

## Download

### Via Command Line

Download the release zip / tarball from any terminal:

| Platform | Command |
|----------|---------|
| Windows (PowerShell) | `Invoke-WebRequest -Uri "https://urfilemanager.vercel.app/urfm-windows.zip" -OutFile "urfm-windows.zip"` |
| Windows (CMD) | `curl -L -o urfm-windows.zip "https://urfilemanager.vercel.app/urfm-windows.zip"` |
| Linux (tarball) | `curl -LO "https://urfilemanager.vercel.app/urfm-linux.tar.gz"` |

### Package Manager Install (Linux)

**Fedora / RHEL:**
```bash
sudo dnf install ./urfm-1.0.0-1.noarch.rpm
```

**Ubuntu / Debian:**
```bash
sudo apt install ./urfm_1.0.0_all.deb
```

## Usage

### Windows

Extract the ZIP, then:

```powershell
.\ufmgr.exe                          # Launch GUI
.\ufmgr-cli.exe C:\Downloads         # Preview (dry-run is the default)
.\ufmgr-cli.exe C:\Downloads --no-dry-run  # Actually move files
.\ufmgr-cli.exe --revert C:\Downloads      # Undo last organization
```

### Linux

**Tarball:** `tar -xzf urfm-linux.tar.gz && chmod +x urfm && ./urfm`

**RPM/DEB:** Run `urfm` from terminal or launch from app menu.

```bash
urfm ~/Downloads --dry-run    # Preview
urfm ~/Downloads               # Organize
urfm ~/Downloads --revert      # Undo
urfm --version                 # Show version
```

## Building from Source

### Windows (MinGW-w64)

```cmd
cd desktop-windows
build.bat
```

Requires MinGW-w64 with `windres`.

### Linux — Java Swing

```bash
cd desktop-linux
chmod +x build.sh
./build.sh
# Produces urfm.jar + urfm launcher
```

### Linux — RPM / DEB packaging

```bash
cd desktop-linux
./build-rpm.sh      # Fedora/RHEL (.rpm)
./build-deb.sh      # Ubuntu/Debian (.deb)
```

## Configuration

Sorting rules are defined in `config.json` (placed next to the executable). Map category names to lowercase extension lists:

```json
{
  "Images": [".jpg", ".jpeg", ".png", ".gif", ".webp", ".svg"],
  "Documents": [".pdf", ".docx", ".doc", ".txt", ".xlsx", ".pptx"],
  "Audio": [".mp3", ".wav", ".aac", ".flac", ".m4a"],
  "Video": [".mp4", ".mkv", ".mov", ".avi", ".webm"],
  "Archives": [".zip", ".tar.gz", ".rar", ".7z", ".tar"]
}
```

Files with unrecognized extensions land in an `Other/` folder. After editing `config.json`, restart the app or re-run the CLI.

## License

MIT
