#!/usr/bin/env bash
# Package urFileManager for all platforms on Linux
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$ROOT_DIR"

PUBLIC_DIR="$ROOT_DIR/frontend-web/public"
mkdir -p "$PUBLIC_DIR"

echo "=============================================="
echo "  urFileManager (urFM) - Release Packager"
echo "=============================================="

# 1. Build and package Linux Java version
echo "[1/4] Building Java JAR and packaging Linux Tarball..."
cd desktop-linux
bash build.sh
cd "$ROOT_DIR"

LINUX_STAGE=$(mktemp -d)
cp desktop-linux/urfm.jar "$LINUX_STAGE/"
cp desktop-linux/urfm "$LINUX_STAGE/"
cp config.json "$LINUX_STAGE/"
cp desktop-linux/RELEASE_README.md "$LINUX_STAGE/README.txt"

rm -f "$PUBLIC_DIR/urfm-linux.tar.gz"
tar -czf "$PUBLIC_DIR/urfm-linux.tar.gz" -C "$LINUX_STAGE" .
rm -rf "$LINUX_STAGE"
echo "  Created: urfm-linux.tar.gz"

# 2. Build and package Fedora RPM and Debian DEB
echo ""
echo "[2/4] Building Linux System Packages (RPM & DEB)..."

# RPM Build
cd desktop-linux
if ./build-rpm.sh; then
    echo "  RPM package created successfully."
else
    echo "  [WARN] RPM packaging failed."
fi
cd "$ROOT_DIR"

# DEB Build
cd desktop-linux
if ./build-deb.sh; then
    echo "  DEB package created successfully."
else
    echo "  [WARN] DEB packaging failed."
fi
cd "$ROOT_DIR"

# 3. Package Windows version
echo ""
echo "[3/4] Packaging Windows C++ Version..."
WIN_ZIP="$PUBLIC_DIR/urfm-windows.zip"
WIN_STAGE=$(mktemp -d)

# Try to find ufmgr.exe
EXE_PATH="desktop-windows/ufmgr.exe"
if [ ! -f "$EXE_PATH" ] && [ -f "$WIN_ZIP" ]; then
    echo "  desktop-windows/ufmgr.exe not found. Extracting from existing ZIP..."
    unzip -p "$WIN_ZIP" ufmgr.exe > "$WIN_STAGE/ufmgr.exe"
elif [ -f "$EXE_PATH" ]; then
    cp "$EXE_PATH" "$WIN_STAGE/"
else
    echo "  [WARN] ufmgr.exe not found and no existing ZIP to copy from."
fi

if [ -f "$WIN_STAGE/ufmgr.exe" ]; then
    cp desktop-windows/run.bat "$WIN_STAGE/"
    cp desktop-windows/ufmgr.bat "$WIN_STAGE/"
    cp config.json "$WIN_STAGE/"
    cp desktop-windows/RELEASE_README.md "$WIN_STAGE/README.txt"
    
    rm -f "$WIN_ZIP"
    cd "$WIN_STAGE"
    zip -q -r "$WIN_ZIP" *
    cd "$ROOT_DIR"
    echo "  Created: urfm-windows.zip"
else
    echo "  [ERROR] Cannot package Windows release: ufmgr.exe is missing."
fi
rm -rf "$WIN_STAGE"

# 4. Generate downloads.json manifest for the website
echo ""
echo "[4/4] Generating downloads.json manifest..."
WIN_SIZE=0
if [ -f "$PUBLIC_DIR/urfm-windows.zip" ]; then
    WIN_SIZE=$(stat -c%s "$PUBLIC_DIR/urfm-windows.zip")
fi

LINUX_SIZE=0
if [ -f "$PUBLIC_DIR/urfm-linux.tar.gz" ]; then
    LINUX_SIZE=$(stat -c%s "$PUBLIC_DIR/urfm-linux.tar.gz")
fi

RPM_SIZE=0
RPM_NAME=""
RPM_FILE=$(find "$PUBLIC_DIR" -name "*.rpm" | head -n 1)
if [ -n "$RPM_FILE" ]; then
    RPM_NAME=$(basename "$RPM_FILE")
    RPM_SIZE=$(stat -c%s "$RPM_FILE")
fi

DEB_SIZE=0
DEB_NAME=""
DEB_FILE=$(find "$PUBLIC_DIR" -name "*.deb" | head -n 1)
if [ -n "$DEB_FILE" ]; then
    DEB_NAME=$(basename "$DEB_FILE")
    DEB_SIZE=$(stat -c%s "$DEB_FILE")
fi

GENERATED_DATE=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

cat > "$PUBLIC_DIR/downloads.json" << EOF
{
  "version": "1.0.0",
  "generated": "$GENERATED_DATE",
  "files": [
    {
      "name": "urfm-windows.zip",
      "platform": "windows",
      "size": $WIN_SIZE
    },
    {
      "name": "urfm-linux.tar.gz",
      "platform": "linux",
      "size": $LINUX_SIZE
    }$(
      if [ -n "$RPM_NAME" ]; then
        echo ","
        echo "    {"
        echo "      \"name\": \"$RPM_NAME\","
        echo "      \"platform\": \"linux-rpm\","
        echo "      \"size\": $RPM_SIZE"
        echo "    }"
      fi
    )$(
      if [ -n "$DEB_NAME" ]; then
        echo ","
        echo "    {"
        echo "      \"name\": \"$DEB_NAME\","
        echo "      \"platform\": \"linux-deb\","
        echo "      \"size\": $DEB_SIZE"
        echo "    }"
      fi
    )
  ]
}
EOF
echo "  Created: downloads.json"
echo "=============================================="
