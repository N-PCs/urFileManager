Name:           urfm
Version:        1.0.0
Release:        1%{?dist}
Summary:        urFileManager - Terminal-aesthetic bulk file organizer GUI and CLI
License:        MIT
URL:            https://github.com/N-PCs/bulk-file-organiser
BuildArch:      noarch
Requires:       (java-17-openjdk-headless or java-21-openjdk-headless or java-11-openjdk-headless or java-latest-openjdk-headless)

%description
urFileManager (urFM) is a terminal-inspired bulk file organizer that organizes files in a directory by type (Images, Documents, Audio, Video, etc.). It features a Swing-based Java GUI and a CLI mode.

%install
mkdir -p %{buildroot}/opt/urfm
mkdir -p %{buildroot}/usr/local/bin
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/icons/hicolor/scalable/apps

# Copy the compiled JAR and configuration
cp %{_sourcedir}/urfm.jar %{buildroot}/opt/urfm/urfm.jar
cp %{_sourcedir}/config.json %{buildroot}/opt/urfm/config.json

# Copy desktop integration files
cp %{_sourcedir}/urfm-icon.svg %{buildroot}/usr/share/icons/hicolor/scalable/apps/urfm.svg
cp %{_sourcedir}/urfm.desktop %{buildroot}/usr/share/applications/urfm.desktop

# Create the launcher script directly in install
cat > %{buildroot}/usr/local/bin/urfm << 'EOF'
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
    echo "Error: Java 17+ not found. Install with:" >&2
    echo "  Fedora: sudo dnf install java-17-openjdk" >&2
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

%changelog
* Wed Jul 08 2026 Neel Pandey <neelpandeyofficial@gmail.com> - 1.0.0-1
- Initial release of urFileManager Linux Java Swing GUI
