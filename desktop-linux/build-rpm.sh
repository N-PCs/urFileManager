#!/usr/bin/env bash
# Script to build RPM package for Fedora
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# 1. Build the jar first
echo "=============================================="
echo "  urFileManager (urFM) - RPM Build"
echo "=============================================="
echo "[1/4] Building Java JAR..."
bash build.sh

# 2. Check for rpmbuild — use native tool if available, otherwise fall back
if command -v rpmbuild &> /dev/null; then
    # ── Native rpmbuild path ──────────────────────────
    RPM_ROOT="$SCRIPT_DIR/build/rpmbuild"
    echo "[2/4] Preparing source files for RPM..."
    rm -rf "$RPM_ROOT"
    mkdir -p "$RPM_ROOT"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

    cp urfm.jar "$RPM_ROOT/SOURCES/"
    cp ../config.json "$RPM_ROOT/SOURCES/"
    cp urfm-icon.svg "$RPM_ROOT/SOURCES/"
    cp urfm.desktop "$RPM_ROOT/SOURCES/"
    cp urfm.spec "$RPM_ROOT/SPECS/"

    echo "[3/4] Building RPM package (rpmbuild)..."
    rpmbuild -bb "$RPM_ROOT/SPECS/urfm.spec" --define "_topdir $RPM_ROOT"

    RPM_FILE=$(find "$RPM_ROOT/RPMS" -name "*.rpm" | head -n 1)
    if [ -n "$RPM_FILE" ]; then
        cp "$RPM_FILE" "$SCRIPT_DIR/build/"
        mkdir -p ../frontend-web/public
        cp "$RPM_FILE" ../frontend-web/public/
        echo "[4/4] RPM build successful! Created:"
        echo "  $SCRIPT_DIR/build/$(basename "$RPM_FILE")"
        echo "  frontend-web/public/$(basename "$RPM_FILE")"
        echo "=============================================="
    else
        echo "[ERROR] RPM build failed: Output package not found."
        exit 1
    fi
else
    # ── Python fallback path ──────────────────────────
    echo "[2/4] rpmbuild not found — using Python fallback builder..."
    echo "[3/4] Building RPM package (Python)..."
    python3 "$SCRIPT_DIR/build-rpm-fallback.py"
    echo "[4/4] RPM build successful (fallback)!"
    echo "=============================================="
fi
