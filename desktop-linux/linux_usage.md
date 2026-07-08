# urFileManager (urFM) — Linux Usage Guide

Bulk file separator & PDF report generator. Java 17+ CLI + Swing GUI.

---

## 1. Requirements

- **Java 17 or newer** (JRE to run, JDK to build).
  - Fedora: `sudo dnf install java-17-openjdk`
  - Ubuntu: `sudo apt install openjdk-17-jre`
  - Arch:   `sudo pacman -S jre17-openjdk`

The `urfm` launcher auto-detects `java` and common OpenJDK paths; if it can't find one it prints install instructions and exits.

---

## 2. File layout (after `./build.sh`)

```
desktop-linux/
├── urfm            # launcher script (runs the jar)
├── urfm.jar        # the compiled application (Main-Class: urfm.Main)
├── config.json     # sorting rules (looked up at runtime)
├── MANIFEST.MF
├── build/          # compiled .class files
└── src/urfm/
    ├── Main.java          # entry point / dispatcher (GUI vs CLI)
    ├── Cli.java           # command-line interface (colored output)
    ├── Console.java       # ANSI color helper
    ├── ColoredLogger.java # colorizes engine messages
    ├── OrganizerEngine.java # does the moving + PDF generation
    ├── ConfigParser.java  # parses config.json
    └── UrfmGUI.java       # Swing GUI
```

---

## 3. How to run

From the `desktop-linux/` folder:

```bash
# Run via the launcher (recommended)
./urfm <arguments>

# Or run the jar directly
java -jar urfm.jar <arguments>
```

> The launcher and jar must sit together so `config.json` is found next to them (or in the parent folder, or the current working directory).

---

## 4. Commands

| Command | What it does |
|---|---|
| `./urfm` | **Launches the Swing GUI** (no console needed). Opens a window to pick a folder, toggle dry-run, execute, revert, and view the PDF report. |
| `./urfm <directory>` | **Live run (CLI).** Organizes the files in `<directory>` into category sub-folders defined by `config.json`, writing the real files and a `organization_report.pdf`. An undo log `.organize_undo.json` is saved. |
| `./urfm <directory> --dry-run` | **Safe preview (CLI).** Shows exactly which files *would* move where, generates `organization_report_preview.pdf`, but **does not touch the file system**. |
| `./urfm <directory> --revert` | **Undo (CLI).** Reads `.organize_undo.json` in the target folder and moves every file back to its original location, then deletes the undo log. |
| `./urfm --gui` | **Force the Swing GUI** even if other flags are present. |
| `./urfm --version` | Prints `urFileManager Java CLI v1.0.0` and exits. |
| `./urfm --help`  /  `./urfm -h` | **Prints the full help** (this command list, examples, notes) and exits. |

### Argument rules
- `<directory>` must be a real, existing folder. An invalid path prints a red `[ERROR]` and exits with code 1.
- Flags can be combined, e.g. `./urfm ~/Downloads --dry-run`.
- Unknown flags (e.g. `--foo`) print an error and the help text, exit code 1.
- `--help` and `--version` take priority over everything else and work in any context.

---

## 5. Output & colors

The CLI prints colored, status-aware output automatically **when run in a real terminal**:

- Banner shows the active mode:
  - `cyan` → `[ DRY RUN — NO FILES WILL BE MOVED ]`
  - `green` → `[ LIVE RUN — FILE SYSTEM WILL BE MODIFIED ]`
  - `yellow` → `[ REVERT MODE ]`
- Per-line coloring from `OrganizerEngine`:
  - red bold `[ERROR]`
  - yellow `[WARN]`
  - blue `[INFO]`
  - cyan `[DRY RUN] Would move ...`
  - green `Moved: ...` / `Reverted: ...`

Colors are **auto-disabled** when stdout is not a TTY (e.g. piped to a file) or when the `NO_COLOR` environment variable is set, so logs stay readable. To force colors in a non-TTY context:

```bash
java -Durfm.forceColor=true -jar urfm.jar ~/Downloads --dry-run
```

---

## 6. Config & reports

- **Rules file:** `config.json` maps a category name to a list of lowercase extensions, e.g.
  ```json
  { "Documents": [".pdf",".txt",".doc"], "Images": [".jpg",".png"] }
  ```
  Looked up in this order: next to `urfm.jar` → its parent folder → current directory.
- **Reports:** a PDF (`organization_report.pdf` for live, `organization_report_preview.pdf` for dry-run) is written into the target folder after each run.
- **Undo log:** `.organize_undo.json` is created during a live run and removed after a successful revert.

---

## 7. How to test it yourself

### A. In a graphical desktop session (full test)
1. `cd desktop-linux`
2. `./build.sh` (rebuilds `urfm.jar`).
3. Launch the GUI: `./urfm`  (or `./urfm --gui`). A window appears.

> **GUI prerequisite:** the GUI needs a **full (headful) JDK/JRE** — the `java-*-openjdk-headless`
> packages cannot show a window, and on Wayland an X11/XWayland display must be reachable
> (`DISPLAY`, e.g. `:0`). The `urfm` launcher now automatically prefers a headful JDK
> (`java-latest-openjdk`) so the GUI starts. If it still can't, it prints a clear fix message
> instead of a silent failure. Over SSH use `ssh -X user@host`.

4. Pick a folder, tick *Dry-Run Preview*, click **EXECUTE RUN**, then open the preview PDF.
5. Untick dry-run and run again to actually organize; use **REVERT LAST RUN** to undo.
   **Revert also cleans up**: it deletes the generated `organization_report*.pdf` files and any
   category folders that are now empty, leaving the folder exactly as it was before.

### B. In a terminal / SSH / headless box (CLI test)
The GUI cannot open without a display, so `--gui` will print:
```
[ERROR] No graphical display available (headless environment).
The GUI requires a desktop session with a display.
Use CLI mode instead, e.g.: urfm <directory> --dry-run
```
Test the CLI instead:

```bash
# 1. Rebuild
./build.sh

# 2. Make a scratch folder with mixed files
mkdir -p ~/urfm-demo
touch ~/urfm-demo/a.pdf ~/urfm-demo/b.jpg ~/urfm-demo/c.txt ~/urfm-demo/d.png

# 3. Safe preview (colors + preview PDF, no changes)
./urfm ~/urfm-demo --dry-run
ls ~/urfm-demo                      # files still there, plus organization_report_preview.pdf

# 4. Real organization
./urfm ~/urfm-demo
ls ~/urfm-demo/Documents ~/urfm-demo/Images   # files sorted, organization_report.pdf present

# 5. Undo it
./urfm ~/urfm-demo --revert
ls ~/urfm-demo                      # back to the original flat layout

# 6. Help & version
./urfm --help
./urfm --version
```

### C. Quick color sanity check (any environment)
Force colors even when piped/non-TTY:
```bash
java -Durfm.forceColor=true -jar urfm.jar ~/urfm-demo --dry-run
```

---

## 8. Packaging & distribution

All build outputs land in `build/` and are also copied to `frontend-web/public/` for download.

| Artifact | How to build | What it's for |
|---|---|---|
| `urfm.jar` + `urfm` launcher | `./build.sh` | Local dev / direct run |
| `build/urfm-linux.tar.gz` | `./build-tarball.sh` | Portable archive for any Linux user |
| `build/urfm-1.0.0-1.noarch.rpm` | `./build-rpm.sh` | Fedora/RHEL install (`dnf install ./urfm-1.0.0-1.noarch.rpm`) |

### Tarball (portable, any distro)
```bash
./build-tarball.sh
tar -xzf build/urfm-linux.tar.gz && cd urfm-linux
./urfm                      # GUI
./urfm ~/Downloads --dry-run   # CLI preview
```
Contains: `urfm` launcher (prefers a headful JDK), `urfm.jar`, `config.json`,
`linux_usage.md`, `RELEASE_README.md`, `urfm.desktop`, `urfm-icon.svg`.

### RPM (Fedora / RHEL)
```bash
./build-rpm.sh                       # needs rpmbuild (installed automatically if missing)
sudo dnf install ./build/urfm-1.0.0-1.noarch.rpm
urfm                                # now on PATH as /usr/local/bin/urfm
```
`build-rpm.sh` requires `rpmbuild` (it installs `rpm-build` + `redhat-rpm-config`
automatically when possible). The old dependency-free Python fallback produced
packages that modern `rpm` refuses to install, so it is no longer used. After
install, the GUI launches from the app menu or `urfm`, and CLI via `urfm <dir>`.

---

## 9. Common issues

- **`Java 17+ not found`** → install a JRE/JDK (see section 1) or put `java` on your `PATH`.
- **`[ERROR] The path '...' is not a valid directory.`** → pass an existing folder as the first argument.
- **`[ERROR] Failed to load config from: ...`** → place a valid `config.json` next to `urfm.jar` (or its parent / CWD).
- **GUI won't open** → it's headless. Causes & fixes:
  - You're on a server/SSH without a display → use CLI, or `ssh -X user@host`.
  - The JVM is the **headless** package (`java-*-openjdk-headless`) → install a full JDK:
    `sudo dnf install java-latest-openjdk` (the launcher prefers it automatically).
  - On Wayland, ensure XWayland is running and `DISPLAY` is set (e.g. `export DISPLAY=:0`).
  - The app prints an explicit, actionable error instead of failing silently.
- **`build-rpm.sh` says rpmbuild is required** → install it:
  `sudo dnf install rpm-build redhat-rpm-config`. The RPM is built with the real
  `rpmbuild` tool (a hand-rolled builder is not used because modern `rpm` rejects
  those packages on install).
- **No colors in output** → you're likely piping/redirecting (auto-off) or `NO_COLOR` is set. Use `-Durfm.forceColor=true` to override.
