import os,json,struct,sys,hashlib
from pathlib import Path
original=Path(sys.argv[1]); patched=Path(sys.argv[2]); source=Path(sys.argv[3]); out=Path(sys.argv[4]);
with original.open('rb') as f:
 f.seek(4); header_size=struct.unpack('<I',f.read(4))[0];f.seek(16); hdr=json.loads(f.read(header_size-8).rstrip(b'\0'))
# New ESM helper: add it to the ASAR file tree before recomputing integrity.
hdr['files']['dist']['files']['linux-processes.mjs'] = {}
hdr['files']['dist']['files']['linux-steam-bridge.mjs'] = {}
hdr["files"]["dist"]["files"]["linux-proton-bridge.mjs"] = {}
packed=[]
offset=0

def recalc_integrity(item, file):
 if item.get('integrity',{}).get('algorithm')!='SHA256':return
 block_size=item['integrity'].get('blockSize',4194304)
 dig=hashlib.sha256()
 blocks=[]
 with open(file,'rb') as f:
  while data:=f.read(block_size):
   dig.update(data)
   blocks.append(hashlib.sha256(data).hexdigest())
 item['integrity']['hash']=dig.hexdigest()
 item['integrity']['blocks']=blocks


def collect(children,parent=Path('.')):
 global offset
 for name, item in children.items():
  rel=parent/name
  if 'files' in item:
   collect(item['files'],rel)
  elif 'link' in item:
   continue
  elif item.get('unpacked'):
   # Source unpacked content must match the ASAR header; hashes change on patches.
   if not (source/rel).is_file(): raise RuntimeError(f'missing unpacked file {rel}')
   item['size']=(source/rel).stat().st_size
   item.pop('offset',None)
   recalc_integrity(item,source/rel)
  else:
   file=source/rel
   if not file.is_file(): raise RuntimeError(f'missing app file {rel}')
   sz=file.stat().st_size
   item['size']=sz
   item['offset']=str(offset)
   recalc_integrity(item,file)
   offset+=sz
   packed.append(file)
collect(hdr['files'])
json_bytes=json.dumps(hdr,separators=(',',':'),ensure_ascii=False).encode()
padding=(-len(json_bytes))%4
payload=struct.pack('<I',len(json_bytes))+json_bytes+b'\0'*padding
# Pickle structure: pickle contains length and bytes; size is encoded in the outer 2nd integer.
header=struct.pack('<III',4,len(payload)+4,len(payload))+payload
with out.open('wb') as fp:
 fp.write(header)
 for file in packed:
  with file.open('rb') as inp:
   while b:=inp.read(1024*1024):fp.write(b)
print('Written',out,out.stat().st_size,'payload',offset,'header',len(header),'files',len(packed))
