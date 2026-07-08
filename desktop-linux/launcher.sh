#!/usr/bin/env bash
# urfm — urFileManager launcher (canonical template)
# Prefers a headful JDK so the Swing GUI can start; falls back to any java for CLI.
DIR="$(cd "$(dirname "$0")" && pwd)"

# Candidate java binaries, in priority order.
# We prefer a JDK/JRE that actually ships the AWT/X11 native libs (libawt_xawt.so)
# so the Swing GUI can start; the "-headless" packages cannot show a GUI.
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

# Fallback: any java at all (GUI may not work, but CLI will)
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

exec "$JAVA" -jar "$DIR/urfm.jar" "$@"
