# urFileManager — Usage Guide

urFileManager (urFM) organizes cluttered folders into categorized subdirectories — Images, Documents, Audio, Video, Archives — in seconds, with a polished GUI, CLI mode, dry-run preview, PDF reports, and customizable themes.

This guide covers all four ways to use urFM:

- **Windows GUI** (`ufmgr.exe`)
- **Windows CLI** (`ufmgr-cli.exe`)
- **Linux GUI** (`urfm` Java Swing)
- **Linux CLI** (`urfm`)
- **Python CLI** (`organizer.py`, cross-platform)

---

## Quick Start

### Windows (GUI)

1. Download `urfm-windows.zip` from the [website](https://urfilemanager.vercel.app).
2. Extract anywhere.
3. Double-click `run.bat` or `ufmgr.exe` to launch the GUI.
4. Click **Browse**, pick a folder, then **Start Organizing**.

### Windows (CLI)

```powershell
# Preview (safe — no files moved). Dry-run is the default.
.\ufmgr-cli.exe "C:\Users\YourName\Downloads"

# Actually move the files
.\ufmgr-cli.exe "C:\Users\YourName\Downloads" --no-dry-run

# Undo a previous organization
.\ufmgr-cli.exe --revert "C:\Users\YourName\Downloads"

# Show help
.\ufmgr-cli.exe -h
```

### Linux (GUI)

```bash
# Tarball
tar -xzf urfm-linux.tar.gz && cd urfm-linux
chmod +x urfm
./urfm                       # Open the Swing GUI

# RPM / DEB (installed system-wide)
urfm                         # Launch GUI from app menu or terminal
```

### Linux (CLI)

```bash
# Preview (safe)
urfm ~/Downloads --dry-run

# Run organization
urfm ~/Downloads

# Undo last run
urfm ~/Downloads --revert
```

### Python CLI (cross-platform)

```bash
# Install dependencies, then preview / run
pip install -r requirements.txt
python organizer.py /path/to/folder --dry-run
python organizer.py /path/to/folder
```

---

## Windows GUI Reference (`ufmgr.exe`)

1. Double-click `ufmgr.exe` (or `run.bat`).
2. Click **Browse** and pick a folder.
3. **Dry Run** is checked by default — uncheck to actually move files.
4. Click **Start Organizing**.
5. A PDF report (`organization_report.pdf` / `organization_report_preview.pdf`) is created in the organized folder.
6. Click **Undo Last Organize** to revert — this moves files back, deletes now-empty category folders, and removes the generated PDF.
7. Switch themes and view the audit log from the GUI.

Both `ufmgr.exe` and `ufmgr-cli.exe` read `config.json` from their own folder.

---

## Windows CLI Reference (`ufmgr-cli.exe`)

The CLI defaults to a safe **dry-run preview**. Both `--flag` and `-flag` syntax are accepted.

| Command | Description |
|---------|-------------|
| `ufmgr-cli.exe <folder>` | Preview organization (safe — no files moved). Dry-run is the default. |
| `ufmgr-cli.exe <folder> --no-dry-run` | Actually move the files into category folders. |
| `ufmgr-cli.exe --revert <folder>` | Undo a previous organization of `<folder>`. |
| `ufmgr-cli.exe --gui` | Reminds you to use the GUI launcher (`ufmgr.exe`) instead. |
| `ufmgr-cli.exe -h` / `--help` | Show help. |

### Options

| Flag | Description |
|------|-------------|
| `--dry-run` | Preview moves, change nothing (CLI default). |
| `--no-dry-run` | Perform the actual file moves. |
| `--revert <dir>` | Undo a previous organization of `<dir>`. |
| `--gui` | Open the graphical interface (points you to `ufmgr.exe`). |
| `-h`, `--help` | Show the help message. |

### Examples

```bat
:: Preview (default)
ufmgr-cli.exe "C:\Users\YourName\Downloads"

:: Execute
ufmgr-cli.exe "C:\Users\YourName\Downloads" --no-dry-run

:: Revert
ufmgr-cli.exe --revert "C:\Users\YourName\Downloads"

:: Other folders
ufmgr-cli.exe "D:\Photos 2024" --no-dry-run
ufmgr-cli.exe --revert "D:\Photos 2024"

:: Help
ufmgr-cli.exe -h
```

### Output

- Colored status lines: `[DRY-RUN]`, `[MOVED]`, `[ERROR]`.
- **`organizer.log`** — Audit log (created in the current directory).
- **`organization_report.pdf`** / **`organization_report_preview.pdf`** — PDF report written into the target folder.
- On completion the CLI pauses (`Press any key to exit`) when run outside an existing console.

> **Note:** `ufmgr.bat` is a thin wrapper that forwards all arguments to `ufmgr-cli.exe`, so you can also run `.\ufmgr.bat "C:\Downloads" --no-dry-run`.

---

## Linux GUI Reference (`urfm`)

The `urfm` launcher / `urfm.jar` opens a Java Swing GUI (terminal-inspired theme).

- From the tarball: `./urfm` (or `./urfm --gui`).
- From RPM/DEB: `urfm` from a terminal, or **urFileManager** from the app menu.

In the GUI you can pick a folder, toggle dry-run, execute, revert, switch themes, and view the PDF report. Revert also cleans up — it deletes the generated `organization_report*.pdf` files and any now-empty category folders, leaving the folder exactly as before.

> **GUI prerequisite:** a **headful** JDK/JRE is required (the `java-*-openjdk-headless` packages cannot show a window). On Wayland, ensure XWayland is running and `DISPLAY` is set (e.g. `export DISPLAY=:0`). Over SSH use `ssh -X user@host`. The `urfm` launcher automatically prefers a headful JDK (`java-latest-openjdk`) and prints a clear fix message if none is available.

---

## Linux CLI Reference (`urfm`)

Available via the tarball launcher, or globally after installing the Fedora RPM or Ubuntu DEB package.

```bash
usage: urfm <directory> [--dry-run] [--revert] [--version] [--gui]

# Launch GUI (no args)
urfm

# Force GUI mode (ignores directory arg)
urfm --gui

# Preview
urfm ~/Downloads --dry-run

# Execute
urfm ~/Downloads

# Revert
urfm ~/Downloads --revert

# Show version
urfm --version

# Show help
urfm --help    # or  urfm -h
```

| Flag | Description |
|------|-------------|
| `--dry-run` | Preview moves & generate a preview PDF without touching files. |
| `--revert` | Undo the last organization using the saved undo log. |
| `--gui` | Launch the Java Swing graphical interface. |
| `--version` | Print version information and exit. |
| `--help`, `-h` | Show the help and exit. |

The CLI prints colored, status-aware output in a real terminal (auto-disabled when piped or when `NO_COLOR` is set). Force colors with:

```bash
java -Durfm.forceColor=true -jar urfm.jar ~/Downloads --dry-run
```

---

## Configuration

Edit `config.json` to customize sorting rules. Example:

```json
{
  "Images": [".jpeg", ".jpg", ".png", ".gif", ".svg"],
  "Documents": [".pdf", ".docx", ".txt"],
  "Audio": [".mp3", ".wav", ".flac"],
  "Video": [".mp4", ".mov", ".mkv"],
  "Archives": [".zip", ".rar", ".tar", ".gz"]
}
```

Files with unrecognized extensions go into an `Other/` folder.

- **Windows:** `config.json` is read from the same folder as `ufmgr.exe` / `ufmgr-cli.exe`.
- **Linux:** `config.json` is looked up next to `urfm.jar` → its parent folder → the current working directory.

---

## Features

| Feature | Description |
|---|---|
| **Dry-run mode** | Preview every move before committing (`--dry-run` / GUI checkbox) |
| **Conflict resolution** | Duplicates renamed automatically (e.g. `report (1).pdf`) |
| **PDF reports** | Detailed report with file names, categories, sizes, and status |
| **Undo / Revert** | Revert the last organization via `--revert` (cleans up folders & reports) |
| **Audit logging** | Every action logged to `organizer.log` with timestamps |
| **Editable config** | Add/remove file types in `config.json` — no recompile needed |
| **6 GUI themes** | Midnight Dark, Minimalist Light, Red Sakura, Forest Emerald, Neon Cyberpunk, Obsidian Volt |

---

## Troubleshooting

| Error | Cause | Fix |
|---|---|---|
| `No module named 'tqdm'` | Missing Python dependency | `pip install -r requirements.txt` |
| `python: command not found` | Python 3 not installed | Install Python 3, or use `python3` instead of `python` |
| `config.json not found` | Config missing from script directory | Copy `config.json` next to `organizer.py` / the executable |
| `Permission denied` (on Linux) | Script not executable | `chmod +x organizer.py` or use `python organizer.py` |
| `Invalid directory` | Path doesn't exist | Double-check the folder path |
| `Java 17+ not found` (Linux) | JRE/JDK missing | `sudo dnf install java-17-openjdk` / `sudo apt install openjdk-17-jre` |
| GUI won't open (Linux) | Headless environment | Use CLI, or install a headful JDK (`java-latest-openjdk`) and ensure `DISPLAY` is set |
