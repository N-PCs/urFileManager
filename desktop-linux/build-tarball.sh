#!/usr/bin/env bash
# Build a portable .tar.gz distribution of urFileManager (urFM) for Linux.
# Produces build/urfm-linux.tar.gz containing everything needed to run:
#   urfm-linux/urfm        (launcher, prefers a headful JDK for the GUI)
#   urfm-linux/urfm.jar    (application)
#   urfm-linux/config.json (sorting rules)
#   urfm-linux/linux_usage.md
#   urfm-linux/RELEASE_README.md
#   urfm-linux/urfm.desktop
#   urfm-linux/urfm-icon.svg
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "=============================================="
echo "  urFileManager (urFM) - Tarball Build"
echo "=============================================="

# 1. Ensure the jar + launcher are built
if [ ! -f urfm.jar ] || [ ! -f urfm ]; then
    echo "[1/4] Building application (build.sh)..."
    bash build.sh
else
    echo "[1/4] Application already built; ensuring launcher is current..."
    cp launcher.sh urfm && chmod +x urfm
fi

# 2. Stage files into a top-level urfm-linux/ directory
STAGE="$SCRIPT_DIR/build/urfm-linux"
echo "[2/4] Staging files into $STAGE ..."
rm -rf "$STAGE"
mkdir -p "$STAGE"

cp urfm                      "$STAGE/urfm"
cp urfm.jar                  "$STAGE/urfm.jar"
cp ../config.json            "$STAGE/config.json"
cp linux_usage.md            "$STAGE/linux_usage.md"
cp RELEASE_README.md         "$STAGE/RELEASE_README.md"
cp urfm.desktop              "$STAGE/urfm.desktop"
cp urfm-icon.svg             "$STAGE/urfm-icon.svg"

# 3. Create the tarball
TARBALL="$SCRIPT_DIR/build/urfm-linux.tar.gz"
echo "[3/4] Creating tarball: $TARBALL"
rm -f "$TARBALL"
tar -czf "$TARBALL" -C "$SCRIPT_DIR/build" urfm-linux

# 4. Publish to the web download page (copies artifact + updates downloads.json)
bash "$SCRIPT_DIR/publish.sh"
echo "[4/4] Tarball build successful!"
echo "  $TARBALL"
echo "=============================================="
echo ""
echo "Install / use:"
echo "  tar -xzf urfm-linux.tar.gz && cd urfm-linux"
echo "  ./urfm                      (GUI)"
echo "  ./urfm ~/Downloads --dry-run (CLI preview)"
echo "=============================================="
