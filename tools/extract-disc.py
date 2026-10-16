import mmap, struct, pathlib, hashlib, json
import argparse
parser=argparse.ArgumentParser(description='Extract local Xbox disc content without modifying the ISO.')
parser.add_argument('iso',type=pathlib.Path); parser.add_argument('output',type=pathlib.Path)
args=parser.parse_args(); iso=args.iso
out=args.output
with iso.open('rb') as f:
 with mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ) as m:
  h=m.find(b'MICROSOFT*XBOX*MEDIA'); assert h>=65536
  base=h-65536
  root,size=struct.unpack_from('<II',m,h+20)
  manifest=[]
  def directory(sector,size,rel):
   start=base+sector*2048; pos=start; end=start+size
   assert 0<=start<=end<=len(m)
   while pos+14<=end:
    unknown,s,n,attr,nlen=struct.unpack_from('<IIIBB',m,pos)
    if unknown==0xffffffff:
     nxt=start+(((pos-start)//2048+1)*2048)
     if nxt<end and all(b==255 for b in m[pos:nxt]): pos=nxt; continue
     break
    assert nlen>0
    name=m[pos+14:pos+14+nlen].decode('ascii')
    assert name not in ('.','..') and not any(c in name for c in '/\\:')
    path=rel/name
    if attr&16: directory(s,n,path)
    else: manifest.append((path,s,n))
    pos+=(14+nlen+3)&~3
  directory(root,size,pathlib.Path())
  selected=manifest
  assert any(str(p).lower()=='default.xex' for p,s,n in selected)
  total=0
  for p,s,n in selected:
   a=base+s*2048; assert a+n<=len(m)
   dest=out/p; dest.parent.mkdir(parents=True,exist_ok=True)
   with dest.open('wb') as w:
    for offset in range(0,n,8*1024*1024): w.write(m[a+offset:a+min(offset+8*1024*1024,n)])
   assert dest.stat().st_size==n
   total+=n
  print(json.dumps({'files':len(selected),'bytes':total,'xex_sha256':hashlib.sha256((out/'default.xex').read_bytes()).hexdigest()}))

