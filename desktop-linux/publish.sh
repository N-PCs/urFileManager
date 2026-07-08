#!/usr/bin/env bash
# Publish build artifacts to frontend-web/public and regenerate downloads.json
# so the website always serves the latest packages with correct file sizes.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PUB="$SCRIPT_DIR/../frontend-web/public"
mkdir -p "$PUB"

# --- Copy whatever artifacts exist in build/ -------------------------------
[ -f "$SCRIPT_DIR/build/urfm-linux.tar.gz" ]       && cp "$SCRIPT_DIR/build/urfm-linux.tar.gz" "$PUB/"
if [ -f "$SCRIPT_DIR/build/urfm-1.0.0-1.noarch.rpm" ]; then
    cp "$SCRIPT_DIR/build/urfm-1.0.0-1.noarch.rpm" "$PUB/"
    # Drop any dist-tagged duplicates (e.g. urfm-1.0.0-1.fc44.noarch.rpm)
    rm -f "$PUB"/urfm-1.0.0-*.fc*.noarch.rpm
fi
[ -f "$SCRIPT_DIR/build/urfm_1.0.0_all.deb" ]       && cp "$SCRIPT_DIR/build/urfm_1.0.0_all.deb" "$PUB/"

# --- Regenerate downloads.json with live file sizes ------------------------
python3 - "$PUB" <<'PY'
import json, os, sys, datetime

pub = sys.argv[1]
mapping = [
    ("urfm-linux.tar.gz",       "linux"),
    ("urfm-1.0.0-1.noarch.rpm", "linux-rpm"),
    ("urfm_1.0.0_all.deb",      "linux-deb"),
    ("urfm-windows.zip",        "windows"),
]

files = []
for name, plat in mapping:
    p = os.path.join(pub, name)
    if os.path.exists(p):
        files.append({"name": name, "platform": plat, "size": os.path.getsize(p)})

files.sort(key=lambda x: x["platform"])
out = {
    "version": "1.0.0",
    "generated": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
    "files": files,
}
with open(os.path.join(pub, "downloads.json"), "w", encoding="utf-8") as f:
    json.dump(out, f, indent=2)
    f.write("\n")

print("Published to", pub)
for f in files:
    print(f"  {f['platform']:>10}  {f['name']}  ({f['size']} bytes)")
PY
