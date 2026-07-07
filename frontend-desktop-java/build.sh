#!/usr/bin/env bash
# Build script for urFileManager Java Linux Edition
# Requires: Java 17+ (JDK)
#
# Fedora: sudo dnf install java-17-openjdk-devel
# Ubuntu: sudo apt install openjdk-17-jdk
# Arch:   sudo pacman -S jdk17-openjdk
set -euo pipefail

echo "=============================================="
echo "  urFileManager (urFM) - Java Linux Build"
echo "=============================================="
echo ""

JAVAC=""
if command -v javac &> /dev/null; then
    JAVAC="javac"
elif [ -f /usr/lib/jvm/java-17-openjdk/bin/javac ]; then
    JAVAC="/usr/lib/jvm/java-17-openjdk/bin/javac"
elif [ -f /usr/lib/jvm/java-11-openjdk/bin/javac ]; then
    JAVAC="/usr/lib/jvm/java-11-openjdk/bin/javac"
else
    echo "[ERROR] Java 17+ (JDK) not found. Install it with:"
    echo "  Fedora: sudo dnf install java-17-openjdk-devel"
    echo "  Ubuntu: sudo apt install openjdk-17-jdk"
    echo "  Arch:   sudo pacman -S jdk17-openjdk"
    exit 1
fi

# Determine JAVA_HOME from javac path
JAVA_HOME=$(dirname "$(dirname "$(readlink -f "$(which "$JAVAC")")")")
echo "[INFO] Using Java: $JAVA_HOME"

# Find Java runtime
JAVA=""
if command -v java &> /dev/null; then
    JAVA="java"
elif [ -f "$JAVA_HOME/bin/java" ]; then
    JAVA="$JAVA_HOME/bin/java"
else
    echo "[ERROR] java runtime not found."
    exit 1
fi

SRC_DIR="src"
OUT_DIR="build"

echo "[1/4] Cleaning previous build..."
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

echo "[2/4] Compiling Java sources..."
$JAVAC -d "$OUT_DIR" $(find "$SRC_DIR" -name "*.java")

echo "[3/4] Creating executable JAR..."
cd "$OUT_DIR"
jar cfm ../urfm.jar ../MANIFEST.MF urfm/*.class
cd ..

echo "[4/4] Making launcher script..."
cat > urfm << 'LAUNCHER'
#!/usr/bin/env bash
DIR="$(cd "$(dirname "$0")" && pwd)"
exec java -jar "$DIR/urfm.jar" "$@"
LAUNCHER
chmod +x urfm

echo ""
echo "Build successful! Created 'urfm.jar' and launcher 'urfm'."
echo ""
echo "Run: ./urfm                  (launches GUI)"
echo "Run: ./urfm <directory> [--dry-run]  (CLI mode)"
echo "=============================================="
