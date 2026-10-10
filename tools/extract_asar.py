from pathlib import Path
import struct,json,os,sys
src=Path(sys.argv[1]); dest=Path(sys.argv[2]);
with src.open('rb') as f:
 f.seek(4); header_bytes=struct.unpack('<I',f.read(4))[0]
 f.seek(16); hdr=json.loads(f.read(header_bytes - 8).strip(b'\x00').decode('utf8'))
base=8+header_bytes
print('base',base,'size',src.stat().st_size)

def process(files, prefix):
 for name, item in files.items():
  rel=prefix / name
  if rel.is_absolute() or '..' in rel.parts: raise RuntimeError('unsafe path')
  out=dest/rel
  if 'files' in item:
   out.mkdir(parents=True, exist_ok=True)
   process(item['files'], rel)
  elif 'link' in item:
   print('link',rel,item['link'])
  elif 'unpacked' in item:
   # present in app.asar.unpacked not here
   continue
  elif 'size' in item:
   size=item['size']; off=int(item['offset']);
   with src.open('rb') as f:
    f.seek(base+off); data=f.read(size)
   out.parent.mkdir(parents=True,exist_ok=True)
   out.write_bytes(data)
   if item.get('executable'):out.chmod(0o755)
process(hdr['files'],Path())
print('wrote',sum(1 for _ in dest.rglob('*') if _.is_file()),'files')
