#!/usr/bin/env bash
# Script to build RPM package for Fedora
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# 1. Build the jar first
echo "=============================================="
echo "  urFileManager (urFM) - RPM Build"
echo "=============================================="
echo "[1/5] Building Java JAR..."
bash build.sh

# 2. Make sure rpmbuild is available (it produces a correct, installable v4 RPM).
#    The old no-rpmbuild Python fallback produced packages modern rpm refuses to
#    install, which is why the GUI/CLI would not run after install.
if ! command -v rpmbuild &>/dev/null; then
    echo "[2/5] rpmbuild not found — attempting to install rpm-build..."
    if command -v sudo &>/dev/null; then
        sudo dnf install -y rpm-build redhat-rpm-config || true
    else
        dnf install -y rpm-build redhat-rpm-config || true
    fi
fi

if ! command -v rpmbuild &>/dev/null; then
    echo "[ERROR] rpmbuild is required to build a valid RPM but could not be installed."
    echo "        Install it manually and re-run:"
    echo "          sudo dnf install rpm-build redhat-rpm-config"
    exit 1
fi

# 3. Stage source files for rpmbuild
echo "[3/5] Preparing source files for RPM..."
RPM_ROOT="$SCRIPT_DIR/build/rpmbuild"
rm -rf "$RPM_ROOT"
mkdir -p "$RPM_ROOT"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cp urfm.jar "$RPM_ROOT/SOURCES/"
cp ../config.json "$RPM_ROOT/SOURCES/"
cp urfm-icon.svg "$RPM_ROOT/SOURCES/"
cp urfm.desktop "$RPM_ROOT/SOURCES/"
cp RELEASE_README.md "$RPM_ROOT/SOURCES/"
cp urfm.spec "$RPM_ROOT/SPECS/"

# 4. Build the RPM
echo "[4/5] Building RPM package (rpmbuild)..."
# __os_install_post disabled so the build still succeeds on minimal systems
# that lack redhat-rpm-config's brp-* helper scripts (no effect on the package).
rpmbuild -bb "$RPM_ROOT/SPECS/urfm.spec" --define "_topdir $RPM_ROOT" --define "__os_install_post %{nil}"

RPM_FILE=$(find "$RPM_ROOT/RPMS" -name "*.rpm" | head -n 1)
if [ -z "$RPM_FILE" ]; then
    echo "[ERROR] RPM build failed: Output package not found."
    exit 1
fi

# 5. Publish to the web download page (copies artifact + updates downloads.json)
cp "$RPM_FILE" "$SCRIPT_DIR/build/urfm-1.0.0-1.noarch.rpm"
bash "$SCRIPT_DIR/publish.sh"
echo "[5/5] RPM build successful! Created:"
echo "  $SCRIPT_DIR/build/urfm-1.0.0-1.noarch.rpm"
echo "=============================================="
echo ""
echo "  Install:  sudo dnf install $SCRIPT_DIR/build/urfm-1.0.0-1.noarch.rpm"
echo "  or:       sudo rpm -i $SCRIPT_DIR/build/urfm-1.0.0-1.noarch.rpm"
echo "=============================================="
