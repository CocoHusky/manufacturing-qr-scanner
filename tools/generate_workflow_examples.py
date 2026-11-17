"""Generate illustrative firmware-schema journal arrays and pitch diagram."""
from pathlib import Path
import json
from datetime import datetime, timedelta, timezone
from xml.sax.saxutils import escape
root=Path(__file__).resolve().parents[1]
base=root/'docs/workflows'
labels=(root/'test/fixtures/pipeline-labels.tsv').read_text().splitlines()
groups={}
for raw in labels:
    if len(raw)==4 and raw.isdigit():
        header=raw
        groups[header]=[]
    else:
        groups[header].append(raw)
folders=['01-component-assembly','02-benchtop-functional-test','03-thermal-fixture-test','04-environmental-aging']
for (header,rows),folder in zip(groups.items(),folders):
    variants=['PASS','FAIL'] if header in ['0201','0301'] else ['']
    for variant in variants:
        chosen=[x for x in rows if 'TYPE=RESULT' not in x or f'STATUS={variant};' in x]
        events=[]
        state=dict(device='',equipment='',disposition='',failureReason='',resultId='',dataReference='')
        recipe=chosen[0].split(';')[0].split('=')[1]
        step=1
        uptime=1000
        def event(kind,raw,message=''):
            events.append(dict(engine='Workflow',event=kind,run='1000-12345',recipeRevision='01',**state,recipe=recipe,step=step,raw=raw,message=message,time=(datetime(2026,11,17,18,tzinfo=timezone.utc)+timedelta(milliseconds=uptime)).isoformat(timespec='seconds').replace('+00:00','Z'),operator='DEMO',department='Engineering',test='Illustrative replay',sessionStarted='2026-11-17T18:00:00Z',uptimeMs=uptime))
        event('started',header)
        for i,raw in enumerate(chosen):
            uptime+=1000
            f=dict(x.split('=',1) for x in raw.split(';'))
            typ=f['TYPE']
            final=i==len(chosen)-1
            if typ=='DEVICE' and not final: state['device']=f['SN']
            if typ in ('BENCH','FIXTURE') or typ=='CHAMBER' and step==2: state['equipment']=f['ID']
            if typ=='RESULT':
                state.update(disposition=f['STATUS'],failureReason=f['REASON'] if f['STATUS']=='FAIL' else '',resultId=f['ID'],dataReference=f['DATA'])
            message=''
            if header=='0401' and step==3: uptime+=5000
            if header=='0401' and step in (3,4): message='Dwell seconds: 6'
            event('completed' if final else 'accepted',raw,message)
            if not (header=='0101' and typ=='PART' and i<6): step+=1
        name='example-'+variant.lower()+'.json' if variant else 'example-record.json'
        (base/folder/name).write_text(json.dumps(events,indent=2)+'\n')
# Firmware default pitch, durations and gap; y-axis shared across rows.
patterns=[
 ('Workflow start','#3479bd',[(1755,75),(0,40),(2565,150)],'Two rising notes'),
 ('Accepted scan','#19805b',[(3375,55)],'Short chirp'),
 ('Rejected scan','#b33232',[(1215,250)],'Low warning'),
 ('Timeout','#b7791f',[(1215,250),(0,250),(1215,250)],'Double warning'),
 ('Complete / PASS','#19805b',[(2160,90),(0,45),(2700,90),(0,45),(3375,180)],'Rising completion'),
 ('Complete / FAIL','#b33232',[(1485,130),(0,55),(1080,130),(0,55),(810,240)],'Low descending notes'),
]
svg=['<svg xmlns="http://www.w3.org/2000/svg" width="840" height="350" viewBox="0 0 840 350">','<rect width="840" height="350" rx="10" fill="#fff"/>','<g font-family="Arial,sans-serif" fill="#233745">','<text x="20" y="24" font-size="14" font-weight="bold">Audible operator feedback</text><text x="820" y="24" text-anchor="end" font-size="11">Default base: 2700 Hz · warning: 250 ms</text>']
for i,(name,color,notes,desc) in enumerate(patterns):
    y=39+i*48
    svg.append(f'<text x="20" y="{y+15}" font-size="13" font-weight="bold" fill="{color}">{escape(name)}</text><text x="20" y="{y+30}" font-size="10">{desc}</text><path d="M210 {y+38} H590" stroke="#d5dee4"/>')
    t=0
    frequencies=[]
    for freq,ms in notes:
        if freq:
            x=210+t*.5;yy=y+35-(freq-500)/3000*30
            svg.append(f'<rect x="{x:g}" y="{yy:g}" width="{ms*.5:g}" height="4" rx="2" fill="{color}"/>')
            frequencies.append(str(freq))
        t+=ms
    svg.append(f'<text x="610" y="{y+23}" font-size="11">{escape(" / ".join(frequencies))} Hz</text>')
svg.append('<text x="20" y="338" font-size="10">Pitch is vertical; duration is horizontal. Volume is literal 0–10% duty. Frequencies scale with the configured base.</text></g></svg>')
(root/'docs/figures/feedback-tones.svg').write_text(''.join(svg)+'\n')
