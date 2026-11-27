from pathlib import Path
from hashlib import blake2b,sha256
import struct,gzip,sys
if len(sys.argv)!=3: raise SystemExit('Usage: python3 mkdelta.py BASE.iso CUSTOM.iso')
base,custom=sys.argv[1:]
def blocks(path):
 with open(path,'rb') as f:
  yield from iter(lambda:f.read(2048),b'')
def key(b):return blake2b(b,digest_size=16).digest()
src={key(b):i*2048 for i,b in enumerate(blocks(base))}
extra=bytearray();ops=[]
sha=sha256()
for b in blocks(custom):
 sha.update(b);p=src.get(key(b));extra_op=p is None
 if extra_op:p=len(extra);extra.extend(b)
 if ops and ops[-1][0]==extra_op and ops[-1][1]+ops[-1][2]==p:ops[-1][2]+=len(b)
 else:ops.append([extra_op,p,len(b)])
with gzip.open('medium.gz','wb') as f:
 f.write(struct.pack('<I',len(ops)))
 for op in ops:f.write(struct.pack('<BII',*op))
 f.write(extra)
Path('size.go').write_text('package main\nconst size int64 = %d\nconst hash = "%s"\n'%(Path(custom).stat().st_size,sha.hexdigest()))
