urFileManager (urFM) — Java Linux Edition Quick Start
======================================================

Requirements
------------
  Java Runtime 17+ (JRE or JDK)

GUI Mode
--------
  ./urfm
  (or: java -jar urfm.jar)

CLI Mode
--------
  ./urfm ~/Downloads --dry-run
  (or: java -jar urfm.jar ~/Downloads --dry-run)

Revert
------
  ./urfm ~/Downloads --revert

Options
-------
  --dry-run    Preview moves without changing files (default in GUI)
  --revert     Undo the last organization
  --version    Show version

Files
-----
  config.json     Sorting rules — edit to add extensions or categories
  organizer.log   Full audit log of every action

Themes
------
  Terminal Green (default), Amber Monitor, White on Dark,
  Blue Matrix, Red Alert

More: https://github.com/N-PCs/bulk-file-organiser
