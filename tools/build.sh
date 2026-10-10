#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Recreate the Linux port using a Melty installer provided by the builder.
set -euo pipefail
PROJECT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo 'Usage: ./tools/build.sh /path/to/Melty-Setup-1.0.72.exe [output-directory]' >&2
  exit 2
fi
SETUP="$(realpath -- "$1")"
OUT="$(realpath -m -- "${2:-$PROJECT/dist}")"
[[ -f "$SETUP" ]] || { echo "Melty installer not found: $SETUP" >&2; exit 1; }
for tool in python3 cc patch sha256sum; do command -v "$tool" >/dev/null || { echo "Missing build tool: $tool" >&2; exit 1; }; done
command -v pkg-config >/dev/null || { echo 'Missing pkg-config' >&2; exit 1; }
pkg-config --exists libarchive || { echo 'Missing libarchive-dev' >&2; exit 1; }
# X11 development headers are needed for the two small graphical dialogs.
pkg-config --exists x11 || { echo 'Missing libx11-dev' >&2; exit 1; }
if command -v 7z >/dev/null 2>&1; then EXTRACTOR=7z
elif command -v 7zz >/dev/null 2>&1; then EXTRACTOR=7zz
elif command -v 7za >/dev/null 2>&1; then EXTRACTOR=7za
else echo 'Missing 7z, 7zz, or 7za to extract the original Windows installer.' >&2; exit 1; fi
WORK="$(mktemp -d "${TMPDIR:-/tmp}/melty-linux-build.XXXXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
mkdir -p "$WORK/win" "$WORK/source" "$WORK/AppDir/usr/share/melty/app.asar.unpacked" "$OUT"
# Original upstream files are never shipped in the GitHub source archive.
"$EXTRACTOR" x -y -bd "-o$WORK/win" "$SETUP" 'resources/app.asar' 'resources/app.asar.unpacked/*' >/dev/null
ORIGINAL="$WORK/win/resources/app.asar"
UNPACKED="$WORK/win/resources/app.asar.unpacked"
[[ -s "$ORIGINAL" && -f "$UNPACKED/src/preload/preload.cjs" ]] || { echo 'Unexpected Melty installer structure' >&2; exit 1; }
python3 "$PROJECT/tools/extract_asar.py" "$ORIGINAL" "$WORK/source" >/dev/null
# Patches are pinned to the exact 1.0.72 upstream Electron bundle.
EXPECTED='e20aad378a68a742b37583e0037e17a9759fb1005a1566026de3606d063af83d'
ACTUAL="$(sha256sum "$WORK/source/dist/main.bundle.mjs" | cut -d' ' -f1)"
[[ "$EXPECTED" == "$ACTUAL" ]] || { echo "Unsupported upstream main.bundle.mjs ($ACTUAL). Expected Melty 1.0.72." >&2; exit 1; }
# Upstream preload uses CRLF line endings. Normalize before applying the small change.
cp -a "$UNPACKED/." "$WORK/source/"
python3 - "$WORK/source/src/preload/preload.cjs" <<'PY'
import sys
from pathlib import Path
p=Path(sys.argv[1]);p.write_bytes(p.read_bytes().replace(b'\r\n',b'\n'))
PY
for patchfile in main.bundle preload package; do
  patch --batch --fuzz=0 --no-backup-if-mismatch -d "$WORK/source" -p1 < "$PROJECT/patches/${patchfile}.patch" >/dev/null
 done
for module in linux-processes linux-steam-bridge linux-proton-bridge; do
  cp "$PROJECT/src/${module}.mjs" "$WORK/source/dist/${module}.mjs"
done
python3 "$PROJECT/tools/repack_asar.py" "$ORIGINAL" '' "$WORK/source" "$WORK/AppDir/usr/share/melty/app.asar" >/dev/null
APPDIR="$WORK/AppDir"
RES="$APPDIR/usr/share/melty"
# Only carry original unpacked items needed by the 1.0.72 runtime.
while IFS= read -r rel; do
  [[ -n "$rel" ]] || continue
  mkdir -p "$RES/app.asar.unpacked/$(dirname "$rel")"
  cp -p "$WORK/source/$rel" "$RES/app.asar.unpacked/$rel"
done < "$PROJECT/tools/unpacked-manifest.txt"
cp "$PROJECT/src/linux-processes.mjs" "$RES/linux-processes.mjs"
cp "$PROJECT/src/linux-proton-bridge.mjs" "$RES/linux-proton-bridge.mjs"
cp "$PROJECT/src/steam-diagnostic.cjs" "$RES/steam-diagnostic.cjs"
mkdir -p "$APPDIR/usr/share/icons/hicolor/512x512/apps" "$APPDIR/usr/lib/melty"
cp "$WORK/source/icon.png" "$APPDIR/usr/share/icons/hicolor/512x512/apps/gg.melty.app.png"
cp "$WORK/source/icon.png" "$APPDIR/gg.melty.app.png"
cp "$PROJECT/linux/gg.melty.app.desktop" "$APPDIR/gg.melty.app.desktop"
cp "$PROJECT/linux/AppRun" "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"
cc -O2 -Wall -Wextra "$PROJECT/src/gui_downloader.c" -o "$APPDIR/usr/lib/melty/gui-downloader" $(pkg-config --cflags --libs x11)
cc -O2 -Wall -Wextra "$PROJECT/src/gui_installer.c" -o "$APPDIR/usr/lib/melty/gui-installer" $(pkg-config --cflags --libs x11)
cc -O2 -Wall -Wextra "$PROJECT/src/make_iso.c" -o "$WORK/make_iso" $(pkg-config --cflags --libs libarchive)
cc -O2 -Wall -Wextra "$PROJECT/src/type1_runtime.c" -o "$WORK/runtime" $(pkg-config --cflags --libs libarchive)
"$WORK/make_iso" "$APPDIR" "$WORK/payload.iso"
python3 "$PROJECT/tools/make_appimage.py" "$WORK/runtime" "$WORK/payload.iso" "$OUT/Melty-Linux-1.0.72-x86_64.AppImage"
echo "Output: $OUT/Melty-Linux-1.0.72-x86_64.AppImage"
