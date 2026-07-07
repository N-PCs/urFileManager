#!/usr/bin/env bash
#
# install.sh — urFileManager (urFM) Linux Edition Installer
#
# Installs the GUI application system-wide so it appears in the
# application menu and can be launched by clicking the app icon.
#
# Run: sudo bash install.sh
#
set -euo pipefail

APP_NAME="urfm"
APP_HOME="/opt/${APP_NAME}"
BIN_PATH="/usr/local/bin/${APP_NAME}"
ICON_DIR="/usr/share/icons/hicolor/scalable/apps"
DESKTOP_DIR="/usr/share/applications"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=============================================="
echo "  urFileManager (urFM) - Linux Install"
echo "=============================================="

# --- Check prerequisites ---
if [ "$(id -u)" -ne 0 ]; then
    echo "[ERROR] This installer must be run as root (sudo)."
    exit 1
fi

JAVA=""
if command -v java &>/dev/null; then
    JAVA="java"
elif [ -f /usr/lib/jvm/java-17-openjdk/bin/java ]; then
    JAVA="/usr/lib/jvm/java-17-openjdk/bin/java"
elif [ -f /usr/lib/jvm/java-21-openjdk/bin/java ]; then
    JAVA="/usr/lib/jvm/java-21-openjdk/bin/java"
elif [ -f /usr/lib/jvm/java-11-openjdk/bin/java ]; then
    JAVA="/usr/lib/jvm/java-11-openjdk/bin/java"
else
    echo "[ERROR] Java 17+ (JRE or JDK) not found."
    echo "  Fedora: sudo dnf install java-17-openjdk"
    echo "  Ubuntu: sudo apt install openjdk-17-jre"
    echo "  Arch:   sudo pacman -S jre17-openjdk"
    exit 1
fi

JAVA_VER=$("$JAVA" -version 2>&1 | head -1 | cut -d'"' -f2 | cut -d'.' -f1)
if [ "$JAVA_VER" -lt 17 ] 2>/dev/null; then
    echo "[WARN] Java version $JAVA_VER detected. Java 17+ is recommended."
fi

# --- Build if jar doesn't exist ---
if [ ! -f "${SCRIPT_DIR}/urfm.jar" ]; then
    echo "[INFO] urfm.jar not found — building..."
    cd "$SCRIPT_DIR"
    bash build.sh
    cd "$SCRIPT_DIR"
fi

# --- Create application directory ---
echo "[1/5] Creating application directory..."
rm -rf "$APP_HOME"
mkdir -p "$APP_HOME"

echo "[2/5] Installing jar and config..."
cp "${SCRIPT_DIR}/urfm.jar" "${APP_HOME}/"
if [ -f "${SCRIPT_DIR}/../config.json" ]; then
    cp "${SCRIPT_DIR}/../config.json" "${APP_HOME}/"
fi

echo "[3/5] Installing launcher script..."
cat > "$BIN_PATH" << 'LAUNCHER'
#!/usr/bin/env bash
# urfm — urFileManager launcher
# Tries to find Java and launch the GUI (or CLI if arguments are given)
JAVA=""
for candidate in java /usr/lib/jvm/java-17-openjdk/bin/java /usr/lib/jvm/java-21-openjdk/bin/java /usr/lib/jvm/java-11-openjdk/bin/java; do
    if command -v "$candidate" &>/dev/null; then
        JAVA="$candidate"
        break
    fi
done
if [ -z "$JAVA" ]; then
    echo "Error: Java 17+ not found. Install with:" >&2
    echo "  Fedora: sudo dnf install java-17-openjdk" >&2
    echo "  Ubuntu: sudo apt install openjdk-17-jre" >&2
    echo "  Arch:   sudo pacman -S jre17-openjdk" >&2
    exit 1
fi
exec "$JAVA" -jar /opt/urfm/urfm.jar "$@"
LAUNCHER
chmod +x "$BIN_PATH"

echo "[4/5] Installing icon..."
mkdir -p "$ICON_DIR"
cp "${SCRIPT_DIR}/urfm-icon.svg" "${ICON_DIR}/urfm.svg"

# Also install a 256x256 PNG for desktops that prefer it
if command -v convert &>/dev/null; then
    mkdir -p "/usr/share/icons/hicolor/256x256/apps"
    convert "${SCRIPT_DIR}/urfm-icon.svg" -resize 256x256 "/usr/share/icons/hicolor/256x256/apps/urfm.png" 2>/dev/null || true
fi

echo "[5/5] Installing .desktop file..."
mkdir -p "$DESKTOP_DIR"
cp "${SCRIPT_DIR}/urfm.desktop" "${DESKTOP_DIR}/urfm.desktop"
chmod 644 "${DESKTOP_DIR}/urfm.desktop"

# Refresh desktop database
if command -v update-desktop-database &>/dev/null; then
    update-desktop-database "$DESKTOP_DIR" &>/dev/null || true
fi
if command -v gtk-update-icon-cache &>/dev/null; then
    gtk-update-icon-cache /usr/share/icons/hicolor &>/dev/null || true
fi

echo ""
echo "=============================================="
echo "  Installation complete!"
echo "=============================================="
echo ""
echo "  You can now launch urFileManager from your"
echo "  application menu, or run: urfm"
echo ""
echo "  CLI usage:  urfm ~/Downloads --dry-run"
echo "  Revert:     urfm ~/Downloads --revert"
echo "=============================================="
