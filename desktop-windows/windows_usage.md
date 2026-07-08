# urFileManager — Windows Usage Guide

This guide covers the **two ways** you can use urFileManager on Windows, both of
which live in the `desktop-windows` folder. They are two **separate programs**
that share the same engine:

1. **`ufmgr.exe` — the GUI launcher** (Graphical Interface). Double-click to use.
2. **`ufmgr-cli.exe` — the Command Line Interface.** Run from a terminal with flags.

> Tip: both programs read the `config.json` file next to them to decide how
> files are sorted into categories (Images, Documents, Videos, etc.).

---

## 1. The GUI Launcher (`ufmgr.exe`)

The simplest way to use the tool — no commands required.

1. Make sure `ufmgr.exe` and `config.json` are in the same folder.
2. **Double-click `ufmgr.exe`** (or run `run.bat`).
3. In the window:
   - Click **Browse** and pick the folder you want to organize.
   - Tick **Dry Run** if you only want a preview (this is on by default).
   - Click **Start Organizing** to move the files.
   - When finished, a PDF report (`organization_report.pdf`) is created in the folder.
4. To undo everything, click **Revert**. This moves the files back to their
   original places, deletes the now-empty category folders, and removes the
   generated PDF report.

You can also theme the window and open the audit log (`organizer.log`) from the GUI.

---

## 2. The Command Line Interface (`ufmgr-cli.exe`)

Open a terminal (Command Prompt or PowerShell) in the `desktop-windows` folder
and run `ufmgr-cli.exe` with the options below. It prints coloured, easy-to-read
output and produces the same PDF report as the GUI.

### Commands / Flags

| Command | What it does |
| --- | --- |
| `<folder>` | Organizes the given folder (preview only by default). |
| `--dry-run` | Preview the moves, change nothing. **This is the CLI default.** |
| `--no-dry-run` | Actually perform the file moves. |
| `--revert <folder>` | Undo a previous organization: move files back to the folder root, delete the now-empty category folders, and delete the PDF report. |
| `-h`, `--help` | Show the usage text with examples. |

Both `--flag` and `-flag` spellings are accepted. (The dedicated GUI launcher
is `ufmgr.exe`; `ufmgr-cli.exe` is console-only, so it points you to `ufmgr.exe`
if launched without a folder.)

### Examples

Preview how a folder would be organized (nothing is moved):

```bat
ufmgr-cli.exe "C:\Users\YourName\Downloads"
```

Actually organize the folder:

```bat
ufmgr-cli.exe "C:\Users\YourName\Downloads" --no-dry-run
```

Reverse a previous organization (moves files back, removes empty folders + report):

```bat
ufmgr-cli.exe --revert "C:\Users\YourName\Downloads"
```

More example folder locations you can use:

```bat
ufmgr-cli.exe "D:\Photos 2024"
ufmgr-cli.exe "E:\Work\Inbox" --no-dry-run
ufmgr-cli.exe --revert "D:\Photos 2024"
```

Show the help text:

```bat
ufmgr-cli.exe -h
```

---

## Building from source (optional)

If you want to rebuild the programs yourself, run `build.bat` from the
`desktop-windows` folder (requires MinGW-w64). It produces two binaries:
- `ufmgr.exe` — the GUI launcher.
- `ufmgr-cli.exe` — the command-line interface.

---

## Files of interest

- `ufmgr.exe` — the GUI launcher.
- `ufmgr-cli.exe` — the command-line interface.
- `cli.cpp` — the CLI source code.
- `gui_app.cpp` — the GUI source code.
- `urfm_common.cpp` / `urfm_common.h` — shared engine (config parsing, PDF report, revert) used by both.
- `config.json` — sorting rules (edit to add extensions or categories).
- `organizer.log` — full audit log of every action.
- `organization_report.pdf` — generated report (created on organize, deleted on revert).
