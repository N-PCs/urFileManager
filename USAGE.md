# urFileManager — Usage Guide

## Quick Start

### Python CLI (cross-platform)

```bash
# 1. Install dependencies
pip install -r requirements.txt

# 2. Preview what would happen (safe — no files moved)
python organizer.py /path/to/folder --dry-run

# 3. Actually organize the folder
python organizer.py /path/to/folder
```

### Java GUI (Linux — recommended)

```bash
# 1. Install Java 17+
sudo apt install openjdk-17-jre   # Ubuntu
sudo dnf install java-17-openjdk  # Fedora

# 2. Build & run
cd frontend-desktop-java
chmod +x build.sh
./build.sh
./urfm                           # opens GUI (terminal aesthetic)
./urfm /path/to/folder --dry-run # CLI mode
```

### C++ FLTK GUI (Linux — alternative)

```bash
# 1. Install FLTK
sudo apt install libfltk1.3-dev   # Ubuntu
sudo dnf install fltk-devel       # Fedora

# 2. Build & run
cd frontend-desktop
chmod +x build.sh
./build.sh
./urfm                           # opens GUI
./urfm /path/to/folder --dry-run # CLI mode
```

### C++ GUI (Windows)

Double-click `run.bat` or run from terminal:

```powershell
.\ufmgr.exe C:\Path\To\Folder --dry-run
```

---

## Python CLI Reference

```
usage: organizer.py [-h] [--dry-run] [--revert] [--version] [source_directory]

Organize files in a directory by their type.

Positional arguments:
  source_directory    The path to the directory you want to organize.

Optional arguments:
  -h, --help          Show this help message and exit.
  --dry-run           Preview organization without moving any files.
  --revert            Undo the last organization (uses .organize_undo.json).
  --version           Show the CLI version and exit.
```

### Examples

```bash
# Show help
python organizer.py --help

# Check version
python organizer.py --version

# Preview organization of ~/Downloads
python organizer.py ~/Downloads --dry-run

# Organize ~/Downloads (moves files into subfolders)
python organizer.py ~/Downloads

# Undo the last organization
python organizer.py ~/Downloads --revert
```

### Output

- **`organizer.log`** — Full audit log with timestamps (created in CWD).
- **`organization_report.pdf`** / **`organization_report_preview.pdf`** — PDF report with file names, categories, sizes, and status.
- **`.organize_undo.json`** — Hidden undo file (created in the organized folder). Used by `--revert`.

---

## Java CLI Reference (Linux `urfm`)

```bash
usage: urfm <directory> [--dry-run] [--revert] [--version]

# Preview
./urfm ~/Downloads --dry-run

# Execute
./urfm ~/Downloads

# Revert
./urfm ~/Downloads --revert
```

## C++ CLI Reference (Linux FLTK `urfm`)

```bash
usage: urfm <directory> [--dry-run]

# Preview
./urfm ~/Downloads --dry-run

# Execute
./urfm ~/Downloads
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

---

## Features

| Feature | Description |
|---|---|
| **Dry-run mode** | Preview every move before committing (`--dry-run` / GUI checkbox) |
| **Conflict resolution** | Duplicates renamed automatically (e.g. `report (1).pdf`) |
| **PDF reports** | Detailed report with file names, categories, sizes, and status |
| **Undo / Revert** | Revert the last organization via `--revert` |
| **Audit logging** | Every action logged to `organizer.log` with timestamps |
| **Editable config** | Add/remove file types in `config.json` — no recompile needed |
| **6 GUI themes** | Midnight Dark, Minimalist Light, Red Sakura, Forest Emerald, Neon Cyberpunk, Obsidian Volt |

---

## Troubleshooting

| Error | Cause | Fix |
|---|---|---|
| `No module named 'tqdm'` | Missing Python dependency | `pip install -r requirements.txt` |
| `python: command not found` | Python 3 not installed | Install Python 3, or use `python3` instead of `python` |
| `config.json not found` | Config missing from script directory | Copy `config.json` next to `organizer.py` |
| `Permission denied` (on Linux) | Script not executable | `chmod +x organizer.py` or use `python organizer.py` |
| `Invalid directory` | Path doesn't exist | Double-check the folder path |
