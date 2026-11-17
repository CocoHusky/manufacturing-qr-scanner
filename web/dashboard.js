const T=Traceability;
function loadLocal(k,d){try{return JSON.parse(localStorage.getItem(k))??d}catch{return d}}
let rules=loadLocal('scanner.rules',[]),parts=loadLocal('scanner.parts',{}),nativeSeen=new Set(),nativeBoot='',nativeLastCount=0,nativeFrameCount=0;
let trials=[],trial=null;
function parseRecord(r){let p=T.parse(r.raw,rules),id=p.fields.part||p.fields.id||'';let desc=Object.prototype.hasOwnProperty.call(parts,id)?parts[id]:'';return {...r,parsed:p,description:typeof desc==='string'?desc:''}}
function addRecord(r){logs.unshift(parseRecord(r));scanTotal++;renderLog()}
function item(r){let li=document.createElement('li'),head=document.createElement('strong');head.textContent=(r.time?new Date(r.time).toLocaleString():'Device uptime '+r.uptime+' ms')+' | '+r.engine+' | '+r.format;li.append(head);
 let p=r.parsed;for(const [k,v] of Object.entries(p.fields)){let row=document.createElement('div');row.textContent=k+' — '+v;li.append(row)}
 if(r.description){let row=document.createElement('div');row.textContent=r.description;li.append(row)}
 if(p.warnings.length){let w=document.createElement('div');w.style.color='#a65a00';w.textContent=p.warnings.join('; ');li.append(w)}
 let details=document.createElement('details'),s=document.createElement('summary'),pre=document.createElement('pre');s.textContent=p.type+' · raw data';pre.style.whiteSpace='pre-wrap';pre.textContent=r.raw;details.append(s,pre);li.append(details);return li}
function renderStructured(){const q=$('searchLog').value.toLowerCase();$('scanTotal').textContent=scanTotal;$('scanLog').replaceChildren(...logs.filter(r=>JSON.stringify(r).toLowerCase().includes(q)).slice(0,100).map(item))}
function downloadFile(name,content,type){let u=URL.createObjectURL(new Blob([content],{type})),a=document.createElement('a');a.href=u;a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(u),1000)}
function syncNative(d){if(d.boot!==nativeBoot||d.count<nativeLastCount){nativeSeen.clear();nativeBoot=d.boot}nativeLastCount=d.count;nativeFrameCount=d.nativeFrames;
 for(const r of (d.records||[]).slice().reverse()){let k=d.boot+':'+r.id;if(nativeSeen.has(k))continue;nativeSeen.add(k);addRecord({engine:'ESP32',format:r.format,raw:r.raw,time:r.time||null,uptime:r.uptime})}
 if(nativeSeen.size>500){nativeSeen=new Set((d.records||[]).map(r=>d.boot+':'+r.id))}
}
function configureTrace(){
 $('searchLog').oninput=renderLog;
 $('rules').value=JSON.stringify(rules,null,2);$('parts').value=JSON.stringify(parts,null,2);
 $('saveRules').onclick=()=>{try{let r=JSON.parse($('rules').value),p=JSON.parse($('parts').value);if(!Array.isArray(r)||r.some(x=>!x||typeof x.prefix!=='string'||!x.prefix||typeof x.separator!=='string'||!x.separator||!Array.isArray(x.fields)||x.fields.some(k=>!T.fields.includes(k))))throw Error('Invalid custom rule');if(!p||typeof p!=='object'||Array.isArray(p)||Object.values(p).some(x=>typeof x!=='string'))throw Error('Part database must map identifiers to descriptions');localStorage.setItem('scanner.rules',JSON.stringify(r));localStorage.setItem('scanner.parts',JSON.stringify(p));rules=r;parts=p;logs=logs.map(parseRecord);renderLog();$('rulesMsg').textContent='Saved locally'}catch(e){$('rulesMsg').textContent=e.message}};
 $('exportJSON').onclick=()=>downloadFile('scans.json',JSON.stringify(logs.slice().reverse(),null,2),'application/json');
 $('rescan').onclick=async()=>{lastCodes='';missingSince=0;lastSignal=0;await fetch('/rearm',{method:'POST'})};
 $('startTrial').onclick=()=>{if(trial)return;let expected=$('expected').value;if(!expected||!$('sampleId').value){$('trialStatus').textContent='Enter sample ID and expected payload';return}trial={expected:T.normalize(expected),symbol_id:$('sampleId').value,start:performance.now(),engine:$('engine').value,results:{},nativeStart:nativeFrameCount};$('trialStatus').textContent='Present target now';$('startTrial').disabled=true;let tr=trial;tr.timer=setTimeout(()=>finishTrial(),5000)};
 $('exportTrials').onclick=()=>downloadFile('validation.csv',['engine,symbol_id,outcome,latency_ms',...trials.map(r=>[r.engine,r.symbol_id,r.outcome,r.latency_ms].map(v=>'"'+String(v).replace(/"/g,'""')+'"').join(','))].join('\r\n'),'text/csv');
 $('snapshot').onclick=()=>{if(!photoBlob)return;photoOriginal=photoBlob;$('photoEditor').hidden=false;let im=new Image();im.onload=()=>{let c=$('photo');c.width=im.width;c.height=im.height;photoImage=im;marks=[];redrawPhoto();URL.revokeObjectURL(im.src)};im.src=URL.createObjectURL(photoBlob)};
 let c=$('photo'),start=null;function point(e){let b=c.getBoundingClientRect();return [(e.clientX-b.left)*c.width/b.width,(e.clientY-b.top)*c.height/b.height]}
 c.onpointerdown=e=>{if(!photoImage)return;start=point(e);c.setPointerCapture(e.pointerId)};
 c.onpointerup=e=>{if(!start)return;let end=point(e);marks.push({a:start,b:end,label:$('markLabel').value});start=null;redrawPhoto()};
 $('undoMark').onclick=()=>{marks.pop();redrawPhoto()};$('savePhoto').onclick=()=>c.toBlob(b=>{let u=URL.createObjectURL(b),a=document.createElement('a');a.href=u;a.download='inspection-annotated.png';a.click();setTimeout(()=>URL.revokeObjectURL(u),1000)});
 $('saveOriginal').onclick=()=>{if(!photoOriginal)return;let u=URL.createObjectURL(photoOriginal),a=document.createElement('a');a.href=u;a.download='inspection-original.jpg';a.click();setTimeout(()=>URL.revokeObjectURL(u),1000)};
}
let photoBlob=null,photoOriginal=null,photoImage=null,marks=[];
function redrawPhoto(){if(!photoImage)return;let c=$('photo'),g=c.getContext('2d');g.drawImage(photoImage,0,0);g.strokeStyle='#ff3030';g.fillStyle='#ff3030';g.lineWidth=Math.max(3,c.width/300);g.font=Math.max(14,c.width/50)+'px sans-serif';for(const m of marks){let x=(m.a[0]+m.b[0])/2,y=(m.a[1]+m.b[1])/2;g.beginPath();g.ellipse(x,y,Math.max(1,Math.abs(m.a[0]-m.b[0])/2),Math.max(1,Math.abs(m.a[1]-m.b[1])/2),0,0,2*Math.PI);g.stroke();g.fillText(m.label,Math.min(m.a[0],m.b[0]),Math.max(20,Math.min(m.a[1],m.b[1])-5))}}
function trialObserve(engine,raw){if(!trial||trial.results[engine]||!raw)return;trial.results[engine]={outcome:T.normalize(raw)===trial.expected?'correct':'wrong',latency_ms:Math.round(performance.now()-trial.start)};let want=trial.engine==='compare'?['browser','native']:[trial.engine==='device'?'native':'browser'];if(want.every(k=>trial.results[k]))finishTrial()}
function finishTrial(){if(!trial)return;clearTimeout(trial.timer);let want=trial.engine==='compare'?['browser','native']:[trial.engine==='device'?'native':'browser'];for(let engine of want)trials.push({engine,symbol_id:trial.symbol_id,...(trial.results[engine]||{outcome:'timeout',latency_ms:''})});$('trialStatus').textContent=want.map(k=>k+': '+(trial.results[k]?.outcome||'timeout')).join(' | ')+' · '+trials.length+' observations';trial=null;$('startTrial').disabled=false}
