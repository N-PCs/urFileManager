urFileManager (urFM) — Quick Start (Windows)
=============================================

Windows
-------
  GUI:  Double-click run.bat   (launches ufmgr.exe)
  CLI:  ufmgr-cli.bat "C:\Users\You\Downloads" --no-dry-run

  CLI commands (ufmgr-cli.exe):
    ufmgr-cli.exe <folder> [--no-dry-run]   Organize a folder (preview by default)
    ufmgr-cli.exe --revert <folder>         Undo a previous organization
    ufmgr-cli.exe -h                        Show usage / examples

  See windows_usage.md for full details.

Options
-------
  --dry-run      Preview moves without changing files (CLI default)
  --no-dry-run   Apply the moves immediately
  --revert       Reverse a previous organization

Files
-----
  config.json    Sorting rules — edit to add extensions or categories
  organizer.log  Full audit log of every action (generated at runtime)

More: https://github.com/N-PCs/urFileManager
