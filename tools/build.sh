#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Rebuild the Melty 1.0.76 Linux port from a locally provided official Windows installer.
set -euo pipefail
PROJECT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ $# -lt 1 || $# -gt 2 ]]; then echo 'Usage: tools/build.sh /path/to/Melty-Setup-1.0.76.exe [output-dir]' >&2; exit 2; fi
SETUP="$(realpath -- "$1")"; OUTPUT="$(realpath -m -- "${2:-$PROJECT/dist}")"
[[ -f "$SETUP" ]] || { echo 'Melty installer not found' >&2; exit 1; }
for t in cc patch python3 pkg-config sha256sum; do command -v "$t" >/dev/null || { echo "Missing: $t" >&2; exit 1; }; done
pkg-config --exists libarchive x11 || { echo 'Missing libarchive or X11 development headers' >&2; exit 1; }
if [[ -n "${MELTY_EXTRACTED_INSTALLER:-}" ]]; then
  [[ -s "$MELTY_EXTRACTED_INSTALLER/resources/app.asar" ]] || { echo 'Pre-extracted resources not found' >&2; exit 1; }
else
  if command -v 7z >/dev/null 2>&1; then ZIP=7z
  elif command -v 7zz >/dev/null 2>&1; then ZIP=7zz
  elif command -v 7za >/dev/null 2>&1; then ZIP=7za
  else echo 'Install p7zip or 7zip' >&2; exit 1; fi
fi
WORK="$(mktemp -d "${TMPDIR:-/tmp}/melty-1.0.76.XXXXXXXX")"; trap 'rm -rf -- "$WORK"' EXIT
mkdir -p "$WORK/win" "$WORK/source" "$WORK/AppDir/usr/share/melty/app.asar.unpacked" "$WORK/AppDir/usr/lib/melty" "$WORK/AppDir/usr/share/icons/hicolor/512x512/apps" "$WORK/build" "$OUTPUT"
if [[ -n "${MELTY_EXTRACTED_INSTALLER:-}" ]]; then
  cp -a "$MELTY_EXTRACTED_INSTALLER/resources" "$WORK/win/"
else
  "$ZIP" x -y -bd "-o$WORK/win" "$SETUP" 'resources/app.asar' 'resources/app.asar.unpacked/*' >/dev/null
fi
ORIG="$WORK/win/resources/app.asar"; UNPACKED="$WORK/win/resources/app.asar.unpacked"
[[ -s "$ORIG" && -s "$UNPACKED/src/preload/preload.cjs" ]] || { echo 'Wrong upstream installer format' >&2; exit 1; }
python3 "$PROJECT/tools/extract_asar.py" "$ORIG" "$WORK/source" >/dev/null
EXPECTED='0c1b43eae4fa87d0d555af3ec186d8a4cb381e9200dbef988296c87354ee970e'
ACTUAL="$(sha256sum "$WORK/source/dist/main.bundle.mjs" | cut -d' ' -f1)"
[[ "$EXPECTED" == "$ACTUAL" ]] || { echo "Unsupported upstream main bundle ($ACTUAL). Expected unmodified Melty 1.0.76." >&2; exit 1; }
cp -a "$UNPACKED/." "$WORK/source/"
python3 - "$WORK/source/src/preload/preload.cjs" <<'PY'
import sys
from pathlib import Path
p=Path(sys.argv[1]);p.write_bytes(p.read_bytes().replace(b'\r\n',b'\n'))
PY
for name in dist_main_bundle_mjs package_json src_preload_preload_cjs; do
  patch --batch --fuzz=0 -d "$WORK/source" -p1 < "$PROJECT/patches/$name.patch" >/dev/null
done
for module in linux-processes linux-steam-bridge linux-proton-bridge; do cp "$PROJECT/src/$module.mjs" "$WORK/source/dist/$module.mjs"; done
node --check "$WORK/source/dist/main.bundle.mjs" || { echo 'Port failed JS syntax test' >&2; exit 1; }
APPDIR="$WORK/AppDir"; RES="$APPDIR/usr/share/melty"
python3 "$PROJECT/tools/repack_asar.py" "$ORIG" '' "$WORK/source" "$RES/app.asar" >/dev/null
while IFS= read -r rel; do
  [[ -n "$rel" ]] || continue
  mkdir -p "$RES/app.asar.unpacked/$(dirname "$rel")"
  cp -p "$WORK/source/$rel" "$RES/app.asar.unpacked/$rel"
done < "$PROJECT/tools/unpacked-manifest.txt"
for name in linux-processes.mjs linux-proton-bridge.mjs; do cp "$PROJECT/src/$name" "$RES/$name"; done
cp "$PROJECT/src/steam-diagnostic.cjs" "$RES/steam-diagnostic.cjs"
cp "$WORK/source/icon.png" "$APPDIR/usr/share/icons/hicolor/512x512/apps/gg.melty.app.png"
cp "$WORK/source/icon.png" "$APPDIR/gg.melty.app.png"
cp "$PROJECT/linux/gg.melty.app.desktop" "$APPDIR/gg.melty.app.desktop"
cp "$PROJECT/linux/AppRun" "$APPDIR/AppRun"; chmod +x "$APPDIR/AppRun"
cc -O2 "$PROJECT/src/gui_installer.c" -o "$APPDIR/usr/lib/melty/gui-installer" $(pkg-config --cflags --libs x11)
cc -O2 "$PROJECT/src/gui_downloader.c" -o "$APPDIR/usr/lib/melty/gui-downloader" $(pkg-config --cflags --libs x11)
cc -O2 -w "$PROJECT/src/make_iso.c" -o "$WORK/build/make_iso" $(pkg-config --cflags --libs libarchive)
cc -O2 -w "$PROJECT/src/type1_runtime.c" -o "$WORK/build/runtime" $(pkg-config --cflags --libs libarchive)
"$WORK/build/make_iso" "$APPDIR" "$WORK/build/melty.iso"
PORT="$OUTPUT/Melty-Linux-1.0.76-x86_64.AppImage"
python3 "$PROJECT/tools/make_appimage.py" "$WORK/build/runtime" "$WORK/build/melty.iso" "$PORT"
# The installer contains the complete port as a version-verified payload, not a rebuilt app.
INST="$WORK/InstallerAppDir"
mkdir -p "$INST/payload" "$INST/usr/lib/melty"
cp "$PORT" "$INST/payload/Melty-1.0.76.AppImage"
cp "$WORK/source/icon.png" "$INST/gg.melty.app.png"
cp "$PROJECT/installer/gg.melty.installer.desktop" "$INST/gg.melty.installer.desktop"
sha="$(sha256sum "$PORT" | cut -d' ' -f1)"
sed "s/@APP_SHA@/$sha/g" "$PROJECT/installer/AppRun.template" > "$INST/AppRun"
chmod +x "$INST/AppRun"
cc -O2 "$PROJECT/installer/gui_installer.c" -o "$INST/usr/lib/melty/gui-installer" $(pkg-config --cflags --libs x11)
"$WORK/build/make_iso" "$INST" "$WORK/build/installer.iso"
INSTALLER="$OUTPUT/Melty-Linux-1.0.76-Installer-x86_64.AppImage"
python3 "$PROJECT/tools/make_appimage.py" "$WORK/build/runtime" "$WORK/build/installer.iso" "$INSTALLER"
"$INSTALLER" --verify
printf 'Built: %s\nBuilt: %s\n' "$PORT" "$INSTALLER"
