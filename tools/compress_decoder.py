"""Compress original browser decoder assets for HTTP gzip delivery."""
from pathlib import Path
import gzip,re,sys
src=Path(sys.argv[1]).read_text()
js=src.split('R"ZXJS(',1)[1].split(')ZXJS"',1)[0].encode()
wasm=bytes(map(int,re.findall(r'\d+',src.split('ZXWASM',1)[1].split('{',1)[1].split('}',1)[0])))
assert wasm[:4]==b'\0asm'
out=Path(sys.argv[2])
with out.open('w') as f:
    f.write('#pragma once\n// Gzip-compressed ZXing-WASM 3.1.7 assets.\n')
    for name,data in [('ZXJS',js),('ZXWASM',wasm)]:
        z=gzip.compress(data,compresslevel=9,mtime=0)
        f.write('static const unsigned char '+name+'[] PROGMEM={\n')
        for i in range(0,len(z),32):f.write(','.join(map(str,z[i:i+32]))+',\n')
        f.write('};\n')
        print(name,len(data),len(z))
