Name:           urfm
Version:        1.0.0
Release:        1%{?dist}
Summary:        urFileManager - Terminal-aesthetic bulk file organizer GUI and CLI
License:        MIT
URL:            https://github.com/N-PCs/urFileManager
BuildArch:      noarch
# Prefer a full (headful) JDK so the Swing GUI works; headless is accepted for CLI-only use.
Requires:       (java-latest-openjdk or java-17-openjdk or java-21-openjdk or java-11-openjdk or java-latest-openjdk-headless or java-17-openjdk-headless or java-21-openjdk-headless or java-11-openjdk-headless)

%description
urFileManager (urFM) is a terminal-inspired bulk file organizer that organizes files in a directory by type (Images, Documents, Audio, Video, etc.). It features a Swing-based Java GUI and a CLI mode.

%install
mkdir -p %{buildroot}/opt/urfm
mkdir -p %{buildroot}/usr/local/bin
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/icons/hicolor/scalable/apps
mkdir -p %{buildroot}/usr/share/doc/urfm

# Copy the compiled JAR and configuration
cp %{_sourcedir}/urfm.jar %{buildroot}/opt/urfm/urfm.jar
cp %{_sourcedir}/config.json %{buildroot}/opt/urfm/config.json

# Copy desktop integration files
cp %{_sourcedir}/urfm-icon.svg %{buildroot}/usr/share/icons/hicolor/scalable/apps/urfm.svg
cp %{_sourcedir}/urfm.desktop %{buildroot}/usr/share/applications/urfm.desktop

# Copy documentation
cp %{_sourcedir}/RELEASE_README.md %{buildroot}/usr/share/doc/urfm/README.md

# Create the launcher script directly in install
cat > %{buildroot}/usr/local/bin/urfm << 'EOF'
#!/usr/bin/env bash
# urfm — urFileManager launcher
# Prefers a headful JDK so the Swing GUI can start; falls back to any java for CLI.
DIR="$(cd "$(dirname "$0")" && pwd)"

CANDIDATES=(
    /usr/lib/jvm/java-latest-openjdk/bin/java
    /usr/lib/jvm/java-21-openjdk/bin/java
    /usr/lib/jvm/java-17-openjdk/bin/java
    /usr/lib/jvm/java-11-openjdk/bin/java
    java
)

JAVA=""
for candidate in "${CANDIDATES[@]}"; do
    if command -v "$candidate" &>/dev/null; then
        if [[ "$candidate" == "java" ]]; then
            JHOME="$(dirname "$(dirname "$(readlink -f "$(command -v java)")")")"
        else
            JHOME="$(dirname "$(dirname "$(readlink -f "$candidate")")")"
        fi
        if [[ -f "$JHOME/lib/libawt_xawt.so" || -f "$JHOME/lib/libawt.so" ]]; then
            JAVA="$candidate"
            break
        fi
    fi
done

if [ -z "$JAVA" ]; then
    if command -v java &>/dev/null; then
        JAVA="java"
    fi
fi

if [ -z "$JAVA" ]; then
    echo "Error: Java 17+ not found. Install with:" >&2
    echo "  Fedora: sudo dnf install java-latest-openjdk" >&2
    echo "  Ubuntu: sudo apt install openjdk-17-jre" >&2
    echo "  Arch:   sudo pacman -S jre17-openjdk" >&2
    exit 1
fi

exec "$JAVA" -jar /opt/urfm/urfm.jar "$@"
EOF
chmod +x %{buildroot}/usr/local/bin/urfm

%files
/opt/urfm/urfm.jar
/opt/urfm/config.json
/usr/local/bin/urfm
/usr/share/applications/urfm.desktop
/usr/share/icons/hicolor/scalable/apps/urfm.svg
/usr/share/doc/urfm/README.md

%changelog
* Wed Jul 08 2026 Neel Pandey <neelpandeyofficial@gmail.com> - 1.0.0-1
- Initial release of urFileManager Linux Java Swing GUI
