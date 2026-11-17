"""Render the density CSV as a dependency-free SVG. Marker offsets are display-only."""
import csv
from collections import defaultdict
from pathlib import Path
from xml.sax.saxutils import escape
root=Path(__file__).resolve().parents[1]
rows=list(csv.DictReader((root/'docs/characterization/optical-results.csv').open()))
names=['QR alpha L','QR alpha M','QR byte M','Data Matrix']
colors=['#2563eb','#d97706','#9333ea','#15803d']
groups=defaultdict(list)
for r in rows:
    if r['format'] in names:
        groups[r['format']].append((int(r['footprint_mm']),int(r['ascii_characters'])))
X=lambda x:80+(x-4)*65
Y=lambda y:365-y*3.2
a=['<svg xmlns="http://www.w3.org/2000/svg" width="840" height="425" viewBox="0 0 840 425"><rect width="840" height="425" fill="white"/><g font-family="Arial,sans-serif" fill="#17212c" font-size="13">']
for y in [0,16,32,64,96]:
    a.append(f'<path d="M65 {Y(y)}H615" stroke="#e2e8f0"/><text x="57" y="{Y(y)+4}" text-anchor="end">{y}</text>')
for x in [4,5,6,8,10,12]:
    a.append(f'<path d="M{X(x)} 365V40" stroke="#e2e8f0"/><text x="{X(x)}" y="385" text-anchor="middle">{x}</text>')
a.append('<path d="M65 40V365H615" stroke="#475569" fill="none"/><text x="340" y="412" text-anchor="middle">Square footprint (mm)</text><text transform="translate(18 202) rotate(-90)" text-anchor="middle">Payload (ASCII characters)</text>')
def marker(i,x,y):
    c=colors[i]
    if i==0:return f'<circle cx="{x}" cy="{y}" r="5" fill="{c}" stroke="white" stroke-width="1"/>'
    if i==1:return f'<rect x="{x-5}" y="{y-5}" width="10" height="10" fill="{c}" stroke="white" stroke-width="1"/>'
    if i==2:return f'<path d="M{x} {y-6}l6 6 -6 6 -6 -6z" fill="{c}" stroke="white" stroke-width="1"/>'
    return f'<path d="M{x} {y-6}l6 11 -12 0z" fill="{c}" stroke="white" stroke-width="1"/>'
coincident=defaultdict(list)
for i,n in enumerate(names):
    for x,y in groups[n]:coincident[x,y].append(i)
for i,n in enumerate(names):
    a.append(f'<polyline points="{" ".join(f"{X(x)},{Y(y)}" for x,y in groups[n])}" fill="none" stroke="{colors[i]}" stroke-width="{7-i*1.6}" stroke-linejoin="round" stroke-linecap="round"/>')
for (x,y),ids in coincident.items():
    for j,i in enumerate(ids):
        dx=(j-(len(ids)-1)/2)*13
        a.append(f'<g><title>{escape(names[i])}: {x} mm, {y} characters</title>{marker(i,X(x)+dx,Y(y))}</g>')
for i,n in enumerate(names):
    y=148+31*i
    a.append(f'<path d="M638 {y}H674" stroke="{colors[i]}" stroke-width="3"/>{marker(i,656,y)}<text x="685" y="{y+4}">{escape(n)}</text>')
a.append('</g></svg>')
(root/'docs/figures/capacity.svg').write_text(''.join(a)+'\n')
