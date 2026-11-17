"""Render known native fixture matrices and standard linear barcode patterns as SVG."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
s=(root/'src/native_fixtures.h').read_text()
a=['<svg xmlns="http://www.w3.org/2000/svg" width="880" height="220" viewBox="0 0 880 220"><rect width="880" height="220" fill="white"/><g font-family="Arial,sans-serif" text-anchor="middle" fill="#17212c">']
for i,(name,title,payload) in enumerate([('dm','Data Matrix','LOT-TEST-02'),('qr','QR','PCB-TEST-01')]):
 w,h=map(int,re.search(rf'{name}W=(\d+), {name}H=(\d+)',s).groups())
 bits=re.search(rf'{name}Matrix\[\] PROGMEM="([01]+)"',s).group(1)
 assert len(bits)==w*h
 cx=110+220*i; scale=min(120//w,120//h)
 x=cx-w*scale/2;y=99-h*scale/2
 for k,b in enumerate(bits):
  if b=='1':a.append(f'<rect x="{x+k%w*scale}" y="{y+k//w*scale}" width="{scale}" height="{scale}" fill="black"/>')
 a.append(f'<text x="{cx}" y="22" font-size="17" font-weight="bold">{title}</text><text x="{cx}" y="190" font-size="14">{payload}</text>')
# Code 128 C: start C, 01, 01, checksum 5, stop.
patterns=['211232','222122','222122','131222','2331112']
# checksum (105 + 1 + 2) % 103 = 5; pattern 5 is 131222.
x=660-sum(sum(map(int,p)) for p in patterns)*2/2
for p in patterns:
 for i,n in enumerate(map(int,p)):
  if i%2==0:a.append(f'<rect x="{x}" y="65" width="{n*2}" height="70" fill="black"/>')
  x+=n*2
a.append('<text x="660" y="22" font-size="17" font-weight="bold">Code 128</text><text x="660" y="190" font-size="14">0101</text>')
# Use a simple Code 39 numeric payload, whose patterns are listed explicitly.
patterns39={'*':'nwnnwnwnn','0':'nnnwwnwnn','1':'wnnwnnnnw'}
seq='*01*'; unit=2
width=sum(sum(3 if n=='w' else 1 for n in patterns39[c])+1 for c in seq)-1
x=770+(220-width*unit)/2-110
# Center fourth panel at 770 (first panels centers 110,330,550,770).
# Shift Code 128 into its own third panel.
a=[v.replace('x="660"','x="550"') if '<text x="660"' in v else v for v in a]
# Shift linear bars generated above by 110, not the 2D matrices.
for j,v in enumerate(a):
 if 'height="70"' in v:
  a[j]=re.sub(r'x="([\d.]+)"',lambda m:f'x="{float(m[1])-110:g}"',v,count=1)
x=770-width*unit/2
for c in seq:
 for i,n in enumerate(patterns39[c]):
  width=(3 if n=='w' else 1)*unit
  if i%2==0:a.append(f'<rect x="{x}" y="65" width="{width}" height="70" fill="black"/>')
  x+=width
 x+=unit
a.append('<text x="770" y="22" font-size="17" font-weight="bold">Code 39</text><text x="770" y="190" font-size="14">01</text></g></svg>')
(root/'docs/barcodes/examples.svg').write_text(''.join(a)+'\n')
