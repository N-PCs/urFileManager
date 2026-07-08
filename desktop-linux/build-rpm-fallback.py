#!/usr/bin/env python3
"""
Minimal RPM v3 package builder for urFileManager.
Builds a valid noarch RPM without requiring rpmbuild.
Uses the RPM binary format specification directly.
"""
import struct
import hashlib
import gzip
import io
import os
import sys
import time

# RPM tag constants
RPMTAG_NAME = 1000
RPMTAG_VERSION = 1001
RPMTAG_RELEASE = 1002
RPMTAG_SUMMARY = 1004
RPMTAG_DESCRIPTION = 1005
RPMTAG_SIZE = 1009
RPMTAG_LICENSE = 1014
RPMTAG_GROUP = 1016
RPMTAG_OS = 1021
RPMTAG_ARCH = 1022
RPMTAG_PAYLOADFORMAT = 1124
RPMTAG_PAYLOADCOMPRESSOR = 1125
RPMTAG_PAYLOADFLAGS = 1126
RPMTAG_DIRNAMES = 1118
RPMTAG_BASENAMES = 1117
RPMTAG_DIRINDEXES = 1116
RPMTAG_FILESIZES = 1028
RPMTAG_FILEMODES = 1030
RPMTAG_FILERDEVS = 1033
RPMTAG_FILEMTIMES = 1034
RPMTAG_FILEMD5S = 1035
RPMTAG_FILELINKTOS = 1036
RPMTAG_FILEFLAGS = 1037
RPMTAG_FILEUSERNAME = 1039
RPMTAG_FILEGROUPNAME = 1040
RPMTAG_FILEINODES = 1096
RPMTAG_FILELANGS = 1097
RPMTAG_FILEDEVICES = 1095
RPMTAG_REQUIRENAME = 1049
RPMTAG_REQUIREVERSION = 1050
RPMTAG_REQUIREFLAGS = 1048
RPMTAG_PROVIDENAME = 1047
RPMTAG_PROVIDEVERSION = 1113
RPMTAG_PROVIDEFLAGS = 1112

# RPM data types
RPM_NULL = 0
RPM_CHAR = 1
RPM_INT8 = 2
RPM_INT16 = 3
RPM_INT32 = 4
RPM_INT64 = 5
RPM_STRING = 6
RPM_BIN = 7
RPM_STRING_ARRAY = 8
RPM_I18NSTRING = 9

# Header region tag
RPMTAG_HEADERSIGNATURES = 62
RPMTAG_HEADERIMMUTABLE = 63

# Signature tags
RPMSIGTAG_SIZE = 1000
RPMSIGTAG_MD5 = 1004
RPMSIGTAG_PAYLOADSIZE = 1007

RPMSENSE_INTERP = (1 << 8)
RPMSENSE_LESS = (1 << 1)
RPMSENSE_GREATER = (1 << 2)
RPMSENSE_EQUAL = (1 << 3)

def align(data, boundary):
    """Pad data to alignment boundary."""
    remainder = len(data) % boundary
    if remainder:
        data += b'\x00' * (boundary - remainder)
    return data

def make_cpio_entry(filename, content, mode=0o100644, is_dir=False):
    """Create a CPIO newc format entry."""
    if is_dir:
        mode = 0o040755
        content = b''
    
    ino = hash(filename) & 0xFFFFFFFF
    nlink = 2 if is_dir else 1
    mtime = int(time.time())
    devmajor = 0
    devminor = 0
    rdevmajor = 0
    rdevminor = 0
    
    name_bytes = filename.encode('utf-8') + b'\x00'
    namesize = len(name_bytes)
    filesize = len(content)
    
    header = "070701"  # magic
    header += "%08X" % ino
    header += "%08X" % mode
    header += "%08X" % 0  # uid
    header += "%08X" % 0  # gid
    header += "%08X" % nlink
    header += "%08X" % mtime
    header += "%08X" % filesize
    header += "%08X" % devmajor
    header += "%08X" % devminor
    header += "%08X" % rdevmajor
    header += "%08X" % rdevminor
    header += "%08X" % namesize
    header += "%08X" % 0  # checksum
    
    entry = header.encode('ascii') + name_bytes
    # Pad to 4-byte boundary
    entry = align(entry, 4)
    entry += content
    entry = align(entry, 4)
    return entry

def make_cpio_trailer():
    """Create CPIO trailer entry."""
    return make_cpio_entry("TRAILER!!!", b'')

def build_header(entries):
    """Build an RPM header structure from a list of (tag, type, data) tuples."""
    index_entries = []
    store = b''
    
    for tag, dtype, data in sorted(entries, key=lambda x: x[0]):
        offset = len(store)
        count = 1
        
        if dtype == RPM_STRING:
            if isinstance(data, str):
                data = data.encode('utf-8')
            encoded = data + b'\x00'
            count = 1
        elif dtype == RPM_STRING_ARRAY:
            if isinstance(data, (list, tuple)):
                encoded = b''
                count = len(data)
                for s in data:
                    if isinstance(s, str):
                        s = s.encode('utf-8')
                    encoded += s + b'\x00'
            else:
                encoded = data.encode('utf-8') + b'\x00' if isinstance(data, str) else data + b'\x00'
                count = 1
        elif dtype == RPM_INT32:
            if isinstance(data, (list, tuple)):
                # Align store to 4 bytes
                remainder = len(store) % 4
                if remainder:
                    store += b'\x00' * (4 - remainder)
                    offset = len(store)
                count = len(data)
                encoded = b''
                for v in data:
                    encoded += struct.pack('>I', v & 0xFFFFFFFF)
            else:
                remainder = len(store) % 4
                if remainder:
                    store += b'\x00' * (4 - remainder)
                    offset = len(store)
                encoded = struct.pack('>I', data & 0xFFFFFFFF)
        elif dtype == RPM_INT16:
            if isinstance(data, (list, tuple)):
                remainder = len(store) % 2
                if remainder:
                    store += b'\x00'
                    offset = len(store)
                count = len(data)
                encoded = b''
                for v in data:
                    encoded += struct.pack('>H', v & 0xFFFF)
            else:
                remainder = len(store) % 2
                if remainder:
                    store += b'\x00'
                    offset = len(store)
                encoded = struct.pack('>H', data & 0xFFFF)
        elif dtype == RPM_BIN:
            count = len(data)
            encoded = data
        else:
            encoded = data if isinstance(data, bytes) else str(data).encode('utf-8') + b'\x00'
        
        index_entries.append(struct.pack('>IIII', tag, dtype, offset, count))
        store += encoded
    
    nindex = len(index_entries)
    hsize = len(store)
    
    # Header magic + version + reserved + nindex + hsize
    header = struct.pack('>3sBIII', b'\x8e\xad\xe8', 1, 0, nindex, hsize)
    header += b''.join(index_entries)
    header += store
    
    return header

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(script_dir, 'build')
    
    jar_path = os.path.join(script_dir, 'urfm.jar')
    config_path = os.path.join(script_dir, '..', 'config.json')
    icon_path = os.path.join(script_dir, 'urfm-icon.svg')
    desktop_path = os.path.join(script_dir, 'urfm.desktop')
    readme_path = os.path.join(script_dir, 'RELEASE_README.md')
    
    for f in [jar_path, config_path, icon_path, desktop_path, readme_path]:
        if not os.path.exists(f):
            print(f"[ERROR] Required file not found: {f}")
            sys.exit(1)
    
    # Create launcher script content
    launcher = b'''#!/usr/bin/env bash
# urfm - urFileManager launcher
JAVA=""
for candidate in java /usr/lib/jvm/java-17-openjdk/bin/java /usr/lib/jvm/java-21-openjdk/bin/java /usr/lib/jvm/java-11-openjdk/bin/java; do
    if command -v "$candidate" &>/dev/null; then
        JAVA="$candidate"
        break
    fi
done
if [ -z "$JAVA" ]; then
    echo "Error: Java 17+ not found. Install with:"
    echo "  Fedora: sudo dnf install java-17-openjdk"
    echo "  Ubuntu: sudo apt install openjdk-17-jre"
    echo "  Arch:   sudo pacman -S jre17-openjdk"
    exit 1
fi
exec "$JAVA" -jar /opt/urfm/urfm.jar "$@"
'''
    
    # Read file contents
    with open(jar_path, 'rb') as f:
        jar_data = f.read()
    with open(config_path, 'rb') as f:
        config_data = f.read()
    with open(icon_path, 'rb') as f:
        icon_data = f.read()
    with open(desktop_path, 'rb') as f:
        desktop_data = f.read()
    with open(readme_path, 'rb') as f:
        readme_data = f.read()
    
    # File entries: (install_path, content, mode)
    files = [
        ("./opt/urfm/urfm.jar", jar_data, 0o100644),
        ("./opt/urfm/config.json", config_data, 0o100644),
        ("./usr/local/bin/urfm", launcher, 0o100755),
        ("./usr/share/applications/urfm.desktop", desktop_data, 0o100644),
        ("./usr/share/icons/hicolor/scalable/apps/urfm.svg", icon_data, 0o100644),
        ("./usr/share/doc/urfm/README.md", readme_data, 0o100644),
    ]
    
    dirs = [
        "./opt/urfm",
        "./usr/local/bin",
        "./usr/share/applications",
        "./usr/share/icons/hicolor/scalable/apps",
        "./usr/share/doc/urfm",
    ]
    
    # Build CPIO payload
    cpio_data = b''
    for d in dirs:
        cpio_data += make_cpio_entry(d, b'', is_dir=True)
    for path, content, mode in files:
        cpio_data += make_cpio_entry(path, content, mode=mode)
    cpio_data += make_cpio_trailer()
    
    # Gzip the payload
    payload_buf = io.BytesIO()
    with gzip.GzipFile(fileobj=payload_buf, mode='wb') as gz:
        gz.write(cpio_data)
    payload = payload_buf.getvalue()
    
    # Compute total installed size
    total_size = sum(len(c) for _, c, _ in files)
    
    # Extract directory names and base names for file entries
    all_paths = [p.lstrip('./') for p in [f[0] for f in files]]
    dirnames_set = sorted(set('/' + os.path.dirname(p) + '/' for p in all_paths))
    basenames = [os.path.basename(p) for p in all_paths]
    dirindexes = []
    for p in all_paths:
        d = '/' + os.path.dirname(p) + '/'
        dirindexes.append(dirnames_set.index(d))
    
    now = int(time.time())
    
    # Build main header
    header_entries = [
        (RPMTAG_NAME, RPM_STRING, "urfm"),
        (RPMTAG_VERSION, RPM_STRING, "1.0.0"),
        (RPMTAG_RELEASE, RPM_STRING, "1"),
        (RPMTAG_SUMMARY, RPM_STRING, "urFileManager - Terminal-aesthetic bulk file organizer"),
        (RPMTAG_DESCRIPTION, RPM_STRING, "urFileManager (urFM) is a bulk file organizer with a retro Swing GUI and CLI."),
        (RPMTAG_SIZE, RPM_INT32, total_size),
        (RPMTAG_LICENSE, RPM_STRING, "MIT"),
        (RPMTAG_GROUP, RPM_STRING, "Applications/File"),
        (RPMTAG_OS, RPM_STRING, "linux"),
        (RPMTAG_ARCH, RPM_STRING, "noarch"),
        (RPMTAG_PAYLOADFORMAT, RPM_STRING, "cpio"),
        (RPMTAG_PAYLOADCOMPRESSOR, RPM_STRING, "gzip"),
        (RPMTAG_PAYLOADFLAGS, RPM_STRING, "9"),
        (RPMTAG_DIRNAMES, RPM_STRING_ARRAY, dirnames_set),
        (RPMTAG_BASENAMES, RPM_STRING_ARRAY, basenames),
        (RPMTAG_DIRINDEXES, RPM_INT32, dirindexes),
        (RPMTAG_FILESIZES, RPM_INT32, [len(c) for _, c, _ in files]),
        (RPMTAG_FILEMODES, RPM_INT16, [m & 0xFFFF for _, _, m in files]),
        (RPMTAG_FILERDEVS, RPM_INT16, [0] * len(files)),
        (RPMTAG_FILEMTIMES, RPM_INT32, [now] * len(files)),
        (RPMTAG_FILEMD5S, RPM_STRING_ARRAY, [hashlib.md5(c).hexdigest() for _, c, _ in files]),
        (RPMTAG_FILELINKTOS, RPM_STRING_ARRAY, [""] * len(files)),
        (RPMTAG_FILEFLAGS, RPM_INT32, [0] * len(files)),
        (RPMTAG_FILEUSERNAME, RPM_STRING_ARRAY, ["root"] * len(files)),
        (RPMTAG_FILEGROUPNAME, RPM_STRING_ARRAY, ["root"] * len(files)),
        (RPMTAG_FILEINODES, RPM_INT32, list(range(1, len(files) + 1))),
        (RPMTAG_FILELANGS, RPM_STRING_ARRAY, [""] * len(files)),
        (RPMTAG_FILEDEVICES, RPM_INT32, [1] * len(files)),
        (RPMTAG_REQUIRENAME, RPM_STRING_ARRAY, ["java-headless"]),
        (RPMTAG_REQUIREVERSION, RPM_STRING_ARRAY, [""]),
        (RPMTAG_REQUIREFLAGS, RPM_INT32, [0]),
        (RPMTAG_PROVIDENAME, RPM_STRING_ARRAY, ["urfm"]),
        (RPMTAG_PROVIDEVERSION, RPM_STRING_ARRAY, ["1.0.0-1"]),
        (RPMTAG_PROVIDEFLAGS, RPM_INT32, [RPMSENSE_EQUAL]),
    ]
    
    header = build_header(header_entries)
    
    # Build signature header
    header_plus_payload = header + payload
    md5_digest = hashlib.md5(header_plus_payload).digest()
    
    sig_entries = [
        (RPMSIGTAG_SIZE, RPM_INT32, len(header_plus_payload)),
        (RPMSIGTAG_MD5, RPM_BIN, md5_digest),
        (RPMSIGTAG_PAYLOADSIZE, RPM_INT32, len(cpio_data)),
    ]
    
    sig_header = build_header(sig_entries)
    # Signature must be padded to 8-byte boundary
    sig_header = align(sig_header, 8)
    
    # Build RPM lead (96 bytes)
    lead = struct.pack('>I', 0xedabeedb)  # magic
    lead += struct.pack('>BB', 3, 0)      # major.minor version
    lead += struct.pack('>H', 0)          # type (binary)
    lead += struct.pack('>H', 0)          # archnum (0 = noarch concept, but use 1 for compat)
    name_padded = b'urfm-1.0.0-1'
    name_padded = name_padded[:66].ljust(66, b'\x00')
    lead += name_padded
    lead += struct.pack('>H', 1)          # osnum (1 = Linux)
    lead += struct.pack('>H', 5)          # signature type
    lead += b'\x00' * 16                  # reserved
    
    assert len(lead) == 96, f"Lead is {len(lead)} bytes, expected 96"
    
    # Assemble final RPM
    rpm_data = lead + sig_header + header + payload
    
    os.makedirs(build_dir, exist_ok=True)
    rpm_path = os.path.join(build_dir, 'urfm-1.0.0-1.noarch.rpm')
    with open(rpm_path, 'wb') as f:
        f.write(rpm_data)
    
    # Copy to frontend-web/public
    public_dir = os.path.join(script_dir, '..', 'frontend-web', 'public')
    os.makedirs(public_dir, exist_ok=True)
    public_rpm = os.path.join(public_dir, 'urfm-1.0.0-1.noarch.rpm')
    with open(public_rpm, 'wb') as f:
        f.write(rpm_data)
    
    print(f"  RPM package created: {rpm_path} ({len(rpm_data)} bytes)")
    print(f"  Copied to: {public_rpm}")

if __name__ == '__main__':
    main()
