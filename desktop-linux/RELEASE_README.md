urFileManager (urFM) — Java Linux Edition Quick Start
======================================================

Requirements
------------
  Java Runtime 17+ (JRE or JDK)

Installation
------------
  Fedora/RHEL: sudo dnf install ./urfm-1.0.0-1.noarch.rpm
  Ubuntu/Deb:  sudo dpkg -i ./urfm_1.0.0_all.deb && sudo apt install -f
  Tarball:     tar -xzf urfm-linux.tar.gz && cd urfm-linux

GUI Mode
--------
  urfm                    (installed via RPM/DEB)
  ./urfm                  (tarball)
  java -jar urfm.jar      (direct JAR)
  urfm --gui              (force GUI mode)

CLI Mode
--------
  urfm ~/Downloads --dry-run    (preview moves)
  urfm ~/Downloads              (execute organization)
  urfm ~/Downloads --revert     (undo last run)
  urfm --version                (show version)

Options
-------
  --dry-run    Preview moves without changing files (default in GUI)
  --revert     Undo the last organization
  --version    Show version info
  --gui        Force launch the Swing GUI

Files
-----
  config.json     Sorting rules — edit to add extensions or categories
  organizer.log   Full audit log of every action

Themes
------
  Terminal Green (default), Amber Monitor, White on Dark,
  Blue Matrix, Red Alert

More: https://github.com/N-PCs/urFileManager
