#!/usr/bin/env python3
"""Embed a libarchive ISO payload behind our type-1 AppImage-compatible runtime."""
from pathlib import Path
import sys

if len(sys.argv) != 4:
    raise SystemExit('usage: make_appimage.py <runtime> <payload.iso> <output.AppImage>')
runtime, payload, dest = [Path(a) for a in sys.argv[1:]]
header = bytearray(runtime.read_bytes())
if header[:4] != b'\x7fELF':
    raise SystemExit('Expected an ELF runtime')
offset = len(header)
if not 4096 < offset < 2 ** 32:
    raise SystemExit('Runtime size outside supported offset range')
header[8:11] = b'AI\x01'
header[11:15] = offset.to_bytes(4, 'little')
with dest.open('wb') as f:
    f.write(header)
    with payload.open('rb') as p:
        while chunk := p.read(1024 * 1024):
            f.write(chunk)
dest.chmod(0o755)
print(f'Built {dest} ({dest.stat().st_size:,} bytes), embedded ISO offset {offset}')
