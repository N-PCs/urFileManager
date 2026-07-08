#!/usr/bin/env bash
# Script to build Debian/Ubuntu DEB package
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# 1. Build the jar first
echo "=============================================="
echo "  urFileManager (urFM) - DEB Build"
echo "=============================================="
echo "[1/4] Building Java JAR..."
bash build.sh

# 2. Setup DEB build directory structure
DEB_ROOT="$SCRIPT_DIR/build/debbuild/urfm_1.0.0_all"
echo "[2/4] Preparing source files for DEB..."
rm -rf "$DEB_ROOT"
mkdir -p "$DEB_ROOT"/DEBIAN
mkdir -p "$DEB_ROOT"/opt/urfm
mkdir -p "$DEB_ROOT"/usr/local/bin
mkdir -p "$DEB_ROOT"/usr/share/applications
mkdir -p "$DEB_ROOT"/usr/share/icons/hicolor/scalable/apps
mkdir -p "$DEB_ROOT"/usr/share/doc/urfm

# 3. Create control file
cat > "$DEB_ROOT"/DEBIAN/control << 'EOF'
Package: urfm
Version: 1.0.0
Section: utils
Priority: optional
Architecture: all
Depends: openjdk-17-jre-headless | openjdk-17-jre | openjdk-21-jre-headless | openjdk-21-jre | default-jre
Maintainer: Neel Pandey <neelpandeyofficial@gmail.com>
Description: urFileManager - Terminal-aesthetic bulk file organizer GUI and CLI
 urFileManager (urFM) is a bulk file organizer that categorizes files in a directory by type.
 It features a retro Swing-based GUI and a full-featured CLI.
EOF

# 4. Copy files
cp urfm.jar "$DEB_ROOT/opt/urfm/"
cp ../config.json "$DEB_ROOT/opt/urfm/"
cp urfm-icon.svg "$DEB_ROOT/usr/share/icons/hicolor/scalable/apps/urfm.svg"
cp urfm.desktop "$DEB_ROOT/usr/share/applications/"
cp RELEASE_README.md "$DEB_ROOT/usr/share/doc/urfm/README.md"

# 5. Create launcher script
cat > "$DEB_ROOT"/usr/local/bin/urfm << 'EOF'
#!/usr/bin/env bash
# urfm — urFileManager launcher
JAVA=""
for candidate in java /usr/lib/jvm/java-17-openjdk/bin/java /usr/lib/jvm/java-21-openjdk/bin/java /usr/lib/jvm/java-11-openjdk/bin/java; do
    if command -v "$candidate" &>/dev/null; then
        JAVA="$candidate"
        break
    fi
done
if [ -z "$JAVA" ]; then
    echo "Error: Java 17+ not found. Please install a Java Runtime." >&2
    exit 1
fi
exec "$JAVA" -jar /opt/urfm/urfm.jar "$@"
EOF
chmod +x "$DEB_ROOT"/usr/local/bin/urfm

# 6. Build the package
echo "[3/4] Building DEB package..."
rm -f "$SCRIPT_DIR/build/urfm_1.0.0_all.deb"

if command -v dpkg-deb &> /dev/null; then
    dpkg-deb --build "$DEB_ROOT" "$SCRIPT_DIR/build/urfm_1.0.0_all.deb"
else
    echo "  [INFO] dpkg-deb not found. Using custom tar/ar packager fallback..."
    
    # Create build temp area
    TEMP_BUILD="$SCRIPT_DIR/build/temp_deb"
    rm -rf "$TEMP_BUILD"
    mkdir -p "$TEMP_BUILD"
    
    # 1. debian-binary
    echo "2.0" > "$TEMP_BUILD/debian-binary"
    
    # 2. control.tar.gz
    cd "$DEB_ROOT/DEBIAN"
    tar -czf "$TEMP_BUILD/control.tar.gz" .
    
    # 3. data.tar.gz
    cd "$DEB_ROOT"
    tar -czf "$TEMP_BUILD/data.tar.gz" --exclude=./DEBIAN .
    
    # 4. Assemble using ar
    cd "$TEMP_BUILD"
    ar rcs "$SCRIPT_DIR/build/urfm_1.0.0_all.deb" debian-binary control.tar.gz data.tar.gz
    
    # Clean up
    rm -rf "$TEMP_BUILD"
fi

# 7. Copy to frontend-web public directory
cd "$SCRIPT_DIR"
mkdir -p ../frontend-web/public
cp "$SCRIPT_DIR/build/urfm_1.0.0_all.deb" ../frontend-web/public/

echo "[4/4] DEB build successful! Created:"
echo "  $SCRIPT_DIR/build/urfm_1.0.0_all.deb"
echo "  frontend-web/public/urfm_1.0.0_all.deb"
echo "=============================================="
