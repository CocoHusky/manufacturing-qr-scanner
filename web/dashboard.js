const T=Traceability;
const session=JSON.parse(sessionStorage.getItem("scanner.session")||"null");let deviceMac="",flipped=false;
function loadLocal(k,d){try{return JSON.parse(localStorage.getItem(k))??d}catch{return d}}
const exampleRule={prefix:'MFG:',separator:'/',fields:['part','revision','lot','serial']},examplePart={'PCB-DEMO-001':'Example control PCB'};
let rules=[exampleRule,...loadLocal('scanner.rules',[]).filter(r=>r.prefix!==exampleRule.prefix)],parts={...loadLocal('scanner.parts',{}),...examplePart},nativeSeen=new Set(),nativeBoot='',nativeLastCount=0,nativeFrameCount=0;
let trials=[],trial=null;
function parseRecord(r){let p=T.parse(r.raw,rules);const startCode=Object.keys(Workflows.recipes).find(k=>Workflows.recipes[k].start===r.raw&&r.format==='Code128');if(startCode)p={type:'Workflow start',fields:{workflow:startCode,recipeRevision:'01'},warnings:[],unknown:[]};let id=p.fields.part||p.fields.id||'';let desc=Object.prototype.hasOwnProperty.call(parts,id)?parts[id]:'';return {...r,parsed:p,description:typeof desc==='string'?desc:''}}
let loggedOperator='';
async function addRecord(r){
 if(r.raw.startsWith('op=')){let q=await fetch('/operator-scan',{method:'POST',body:new URLSearchParams({raw:r.raw})});if(!q.ok)$('info').textContent=await q.text();return}
 if(r.engine==='Browser'){r.operator=session.name;r.session=session;try{let q=await fetch('/record',{method:'POST',body:new URLSearchParams({raw:r.raw,format:r.format,time:r.time})});let outcome=await q.json();r.accepted=q.ok&&outcome.accepted;r.reason=outcome.message;r.run=outcome.run;await refreshWorkflow()}catch(e){r.accepted=false;r.reason='Record not saved: '+e.message}}
 r.deviceMac=deviceMac;logs.unshift(parseRecord(r));if(r.accepted!==false)scanTotal++;persistLogs();renderLog();
}
function item(r){let li=document.createElement('li'),head=document.createElement('strong');head.textContent=(r.time?new Date(r.time).toLocaleString():'Device uptime '+r.uptime+' ms')+' | '+r.engine+' | '+r.format;li.append(head);if(r.accepted===false){let w=document.createElement('div');w.textContent='Rejected — '+(r.reason||'Check workflow');w.style.color='#9b2727';li.append(w)}
 let p=r.parsed;for(const [k,v] of Object.entries(p.fields)){let row=document.createElement('div');row.textContent=k+' — '+v;li.append(row)}
 if(r.operator){let row=document.createElement('div');row.textContent='Operator — '+r.operator;li.append(row)}if(r.description){let row=document.createElement('div');row.textContent=r.description;li.append(row)}
 if(p.warnings.length){let w=document.createElement('div');w.style.color='#a65a00';w.textContent=p.warnings.join('; ');li.append(w)}
 if((p.fields.signature||p.fields.id)&&/^SIG-[a-f0-9]{16}$/i.test(p.fields.signature||p.fields.id)){let b=document.createElement('button');b.textContent='View sign-off';b.onclick=async()=>{let q=await fetch('/signature?id='+encodeURIComponent(p.fields.signature||p.fields.id));if(!q.ok){b.textContent=await q.text();return}let x=await q.json(),d=document.createElement('div');d.textContent=[x.operator,x.department||x.part,x.test||x.action,x.time].join(' | ');if(x.signature&&x.signature.startsWith('data:image/png;base64,')){let im=document.createElement('img');im.src=x.signature;im.style.maxWidth='100%';d.append(im)}li.append(d);b.disabled=true};li.append(b)}let details=document.createElement('details'),s=document.createElement('summary'),pre=document.createElement('pre');s.textContent=p.type+' · raw data';pre.style.whiteSpace='pre-wrap';pre.textContent=r.raw;details.append(s,pre);li.append(details);return li}
function renderStructured(){const q=$('searchLog').value.toLowerCase();$('scanTotal').textContent=scanTotal;$('scanLog').replaceChildren(...logs.filter(r=>JSON.stringify(r).toLowerCase().includes(q)).slice(0,100).map(item))}
function downloadFile(name,content,type){let u=URL.createObjectURL(new Blob([content],{type})),a=document.createElement('a');a.href=u;a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(u),1000)}
function syncNative(d){if(d.boot!==nativeBoot||d.count<nativeLastCount){nativeSeen.clear();nativeBoot=d.boot}nativeLastCount=d.count;nativeFrameCount=d.nativeFrames;
 for(const r of (d.records||[]).slice().reverse()){let k=d.boot+':'+r.id;if(nativeSeen.has(k)||logs.some(x=>x.recordKey===k))continue;nativeSeen.add(k);addRecord({recordKey:k,engine:'Embedded',format:r.format,raw:r.raw,time:r.time||null,uptime:r.uptime,operator:r.operator,accepted:r.accepted,reason:r.reason,run:r.run,session:{name:r.operator,department:r.department,test:r.test,startedAt:r.sessionStarted}})}
 if(nativeSeen.size>500){nativeSeen=new Set((d.records||[]).map(r=>d.boot+':'+r.id))}
}
function configureTrace(){configureWorkflows();
 $('searchLog').oninput=renderLog;
 $('operatorLogout').onclick=async()=>{await fetch('/operator-logout',{method:'POST'});sessionStorage.removeItem('scanner.session');location.replace('/sign')};
 logs=loadLocal('scanner.logs',[]).map(parseRecord);scanTotal=logs.filter(r=>r.accepted!==false).length;renderLog();
 $('initStorage').onclick=async()=>{if(!confirm('Initialize the unmounted archive partition? Existing data in that partition may be erased.'))return;let r=await fetch('/storage-init',{method:'POST',body:new URLSearchParams({confirm:'INITIALIZE'})});if(!r.ok)$('diagnostics').textContent=await r.text();else await controls()};
 $('autoStandby').onchange=()=>setStandby('auto='+($('autoStandby').checked?'1':'0'));

 $('sleepNow').onclick=async()=>{running=false;$('pause').textContent='Resume';await setStandby('mode=sleep')};
 $('wakeNow').onclick=async()=>{await setStandby('mode=wake');running=true;$('pause').textContent='Pause'};
 $('captureNow').onclick=async()=>{try{if($('engine').value==='device'){let r=await fetch('/scan-image',{cache:'no-store'});if(!r.ok)throw Error('Scan a code first');photoBlob=await r.blob()}if(!photoBlob)throw Error('No camera image yet');photoOriginal=photoBlob;let im=new Image();im.onload=()=>{let c=$('photo');c.width=im.width;c.height=im.height;photoImage=im;marks=[];redrawPhoto();URL.revokeObjectURL(im.src);$('editor').showModal()};im.src=URL.createObjectURL(photoBlob)}catch(e){$('info').textContent=e.message}};
 $('flip').onclick=()=>{flipped=!flipped;$('flip').textContent=flipped?'Unflip':'Flip';lastImageKey='';};
 $('closeEditor').onclick=()=>$('editor').close();
 $('rules').value=JSON.stringify(rules,null,2);$('parts').value=JSON.stringify(parts,null,2);
 $('saveRules').onclick=()=>{try{let r=JSON.parse($('rules').value),p=JSON.parse($('parts').value);if(!Array.isArray(r)||r.some(x=>!x||typeof x.prefix!=='string'||!x.prefix||typeof x.separator!=='string'||!x.separator||!Array.isArray(x.fields)||x.fields.some(k=>!T.fields.includes(k))))throw Error('Invalid custom rule');if(!p||typeof p!=='object'||Array.isArray(p)||Object.values(p).some(x=>typeof x!=='string'))throw Error('Part database must map identifiers to descriptions');r=[exampleRule,...r.filter(x=>x.prefix!==exampleRule.prefix)];p={...p,...examplePart};localStorage.setItem('scanner.rules',JSON.stringify(r));localStorage.setItem('scanner.parts',JSON.stringify(p));rules=r;parts=p;$('rules').value=JSON.stringify(rules,null,2);$('parts').value=JSON.stringify(parts,null,2);logs=logs.map(parseRecord);renderLog();$('rulesMsg').textContent='Saved locally'}catch(e){$('rulesMsg').textContent=e.message}};
 $('exportJSON').onclick=()=>downloadFile('scans.json',JSON.stringify({schemaVersion:1,device:{mac:deviceMac,id:'scanner-'+deviceMac.replace(/:/g,'')},operator:session,exportedAt:new Date().toISOString(),records:logs.slice().reverse(),inspectionImages:loadLocal('scanner.images',[])},null,2),'application/json');
 $('rescan').onclick=async()=>{lastCodes='';missingSince=0;lastSignal=0;let r=await fetch('/rearm',{method:'POST'});$('rescan').textContent=r.ok?'Ready to rescan':'Retry rescan';setTimeout(()=>$('rescan').textContent='Rescan',1200)};
 let c=$('photo'),start=null;function point(e){let b=c.getBoundingClientRect();return [(e.clientX-b.left)*c.width/b.width,(e.clientY-b.top)*c.height/b.height]}
 c.onpointerdown=e=>{if(!photoImage)return;start=point(e);c.setPointerCapture(e.pointerId)};
 c.onpointerup=e=>{if(!start)return;let end=point(e);marks.push({a:start,b:end,label:$('markLabel').value,shape:$('markShape').value});start=null;redrawPhoto()};
 $('undoMark').onclick=()=>{marks.pop();redrawPhoto()};$('savePhoto').onclick=async()=>{try{let original=await blobData(photoOriginal),annotated=c.toDataURL('image/jpeg',.85);let images=loadLocal('scanner.images',[]);images.push({id:Date.now(),time:new Date().toISOString(),operator:session,deviceMac,original,annotated,marks});localStorage.setItem('scanner.images',JSON.stringify(images));renderImages();$('photoMsg').textContent='Saved in this browser'}catch(e){$('photoMsg').textContent='Storage full or unavailable; export the image instead'}};
 $('exportImage').onclick=()=>c.toBlob(b=>{let a=document.createElement('a');a.href=URL.createObjectURL(b);a.download='inspection-annotated.png';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)});
 renderImages();
 $('saveOriginal').onclick=()=>{if(!photoOriginal)return;let u=URL.createObjectURL(photoOriginal),a=document.createElement('a');a.href=u;a.download='inspection-original.jpg';a.click();setTimeout(()=>URL.revokeObjectURL(u),1000)};
}
let photoBlob=null,photoOriginal=null,photoImage=null,marks=[];
function redrawPhoto(){if(!photoImage)return;let c=$('photo'),g=c.getContext('2d');g.drawImage(photoImage,0,0);g.strokeStyle='#ff3030';g.fillStyle='#ff3030';g.lineWidth=Math.max(3,c.width/300);g.font=Math.max(14,c.width/50)+'px sans-serif';for(const m of marks){let x=(m.a[0]+m.b[0])/2,y=(m.a[1]+m.b[1])/2;g.beginPath();if(m.shape==='square')g.rect(Math.min(m.a[0],m.b[0]),Math.min(m.a[1],m.b[1]),Math.abs(m.a[0]-m.b[0]),Math.abs(m.a[1]-m.b[1]));else g.ellipse(x,y,Math.max(1,Math.abs(m.a[0]-m.b[0])/2),Math.max(1,Math.abs(m.a[1]-m.b[1])/2),0,0,2*Math.PI);g.stroke();g.fillText(m.label,Math.min(m.a[0],m.b[0]),Math.max(20,Math.min(m.a[1],m.b[1])-5))}}
function trialObserve(engine,raw){if(!trial||trial.results[engine]||!raw)return;trial.results[engine]={outcome:T.normalize(raw)===trial.expected?'correct':'wrong',latency_ms:Math.round(performance.now()-trial.start)};let want=trial.engine==='compare'?['browser','native']:[trial.engine==='device'?'native':'browser'];if(want.every(k=>trial.results[k]))finishTrial()}
function finishTrial(){if(!trial)return;clearTimeout(trial.timer);let want=trial.engine==='compare'?['browser','native']:[trial.engine==='device'?'native':'browser'];for(let engine of want)trials.push({engine,symbol_id:trial.symbol_id,...(trial.results[engine]||{outcome:'timeout',latency_ms:''})});$('trialStatus').textContent=want.map(k=>k+': '+(trial.results[k]?.outcome||'timeout')).join(' | ')+' · '+trials.length+' observations';trial=null;$('startTrial').disabled=false}

let diagLoaded=false;
async function setStandby(q){try{let r=await fetch('/standby?'+q,{method:'POST'});if(!r.ok)throw Error(await r.text());diagLoaded=false;await controls()}catch(e){$('diagnostics').textContent=e.message}}
function updateDiagnostics(d){if(session&&!d.operator){sessionStorage.removeItem('scanner.session');location.replace('/sign');return}running=!d.paused;$('pause').textContent=running?'Pause':'Resume';deviceMac=d.deviceMac||'';loggedOperator=session?.name||'';$('operatorStatus').textContent=session?session.name+' · '+session.department:'No operator';$('deviceInfo').textContent='Device '+deviceMac+' | '+d.ip+' | '+d.appSlot+' | OTA '+(d.otaReady?'ready':'unavailable');$('initStorage').hidden=d.journalReady;if(!diagLoaded){$('autoStandby').checked=d.autoStandby;$('rate').value=d.nativePeriod?String(Math.round(1000/d.nativePeriod)):'0';diagLoaded=true}$('diagnostics').textContent=(d.asleep?'Standby':'Camera active')+' | Heap '+d.heap+' B | PSRAM '+d.psram+' B | Uptime '+Math.round(d.uptime/1000)+' s | '+(d.journalReady?'Archive ready':'Archive unavailable')+(d.journalError?' | '+d.journalError:'')}

async function readPolarities(im,effort){
 return ZXingWASM.readBarcodes(im,{formats:['QRCode','DataMatrix','Code128','Code39'],tryHarder:effort!=='fast',tryRotate:true,tryInvert:true,tryDownscale:effort==='deep',maxNumberOfSymbols:1});
}

function persistLogs(){try{localStorage.setItem('scanner.logs',JSON.stringify(logs))}catch(e){$('diagnostics').textContent='Browser storage full; export logs'}}
function blobData(b){return new Promise((resolve,reject)=>{let r=new FileReader();r.onload=()=>resolve(r.result);r.onerror=reject;r.readAsDataURL(b)})}
function renderImages(){let root=$('savedImages');root.replaceChildren();for(let im of loadLocal('scanner.images',[])){let row=document.createElement('p'),a=document.createElement('a');a.href=im.annotated;a.download='inspection-'+im.id+'.jpg';a.textContent=new Date(im.time).toLocaleString()+' · Export image';row.append(a);root.append(row)}}


let workflow=null,workflowPolling=false;
async function workflowRequest(action,id=''){
 const r=await fetch('/workflow',{method:'POST',body:new URLSearchParams({action,id})});
 if(!r.ok)throw Error(await r.text());renderWorkflow(await r.json());lastCodes='';missingSince=0;lastSignal=0;
}
function configureWorkflows(){
 const recipe=$('recipe');for(const [id,r] of Object.entries(Workflows.recipes)){let o=document.createElement('option');o.value=id;o.textContent=r.name;recipe.append(o)}
 function tab(work){$('scannerPanel').hidden=work;$('workflowPanel').hidden=!work;$('scanTab').setAttribute('aria-pressed',String(!work));$('workflowTab').setAttribute('aria-pressed',String(work))}
 $('scanTab').onclick=()=>tab(false);$('workflowTab').onclick=()=>tab(true);
 recipe.onchange=()=>{lastWorkflowRender='';renderWorkflow(workflow||{active:false})};renderWorkflow({active:false});
 $('startWorkflow').onclick=()=>workflowRequest('start',recipe.value).catch(e=>$('workflowStatus').textContent=e.message);
 $('resetWorkflow').onclick=()=>{if(workflow?.active&&confirm('Cancel active workflow?'))workflowRequest('cancel').catch(e=>$('workflowStatus').textContent=e.message)};
 $('exportWorkflow').onclick=async()=>{try{let r=await fetch('/journal');if(!r.ok)throw Error(await r.text());let records=(await r.text()).split('\n').filter(Boolean).map(JSON.parse).filter(x=>x.engine==='Workflow');downloadFile('workflow-runs.json',JSON.stringify({operator:session,deviceMac,records,active:workflow},null,2),'application/json')}catch(e){$('workflowStatus').textContent=e.message}};
 $('exportRunRecord').onclick=async()=>{try{let r=await fetch('/workflow-record',{cache:'no-store'});if(!r.ok)throw Error(await r.text());let record=await r.json(),id=String(record.runId||'run').replace(/[^A-Za-z0-9_-]/g,'_');downloadFile('traceability-'+id+'.json',JSON.stringify(record,null,2),'application/json')}catch(e){$('workflowStatus').textContent=e.message}};
 refreshWorkflow();setInterval(refreshWorkflow,750);
}
async function refreshWorkflow(){if(workflowPolling)return;workflowPolling=true;try{let r=await fetch('/workflow',{cache:'no-store'});if(r.ok)renderWorkflow(await r.json())}catch(e){}finally{workflowPolling=false}}
const workflowFieldNames={SN:'Serial number',ID:'Identifier',PN:'Part number',LOT:'Lot',REV:'Revision',DATE:'Date',MFG:'Manufactured',DEVICE:'Device serial',BENCH:'Equipment ID',DATA:'Data reference',CHAMBER:'Chamber ID',TYPE:'Label type',STATUS:'Test result',REASON:'Failure reason',V:'Output voltage',I:'Supply current',T:'Temperature'};
function workflowFields(raw){
 const fields=[];for(const segment of (raw||'').split(';')){const pos=segment.indexOf('=');if(pos<1)continue;const key=segment.slice(0,pos);if(key==='WF'||key==='STEP'||key==='TYPE')continue;fields.push([workflowFieldNames[key]||key,segment.slice(pos+1)])}return fields;
}
function fieldDetails(raw){
 const details=document.createElement('dl');details.className='workflow-fields';
 for(const [key,value] of workflowFields(raw)){const dt=document.createElement('dt'),dd=document.createElement('dd');dt.textContent=key;dd.textContent=value;if(key==='Test result'){dd.style.color=value==='FAIL'?'#a12424':'#176440'}details.append(dt,dd)}return details;
}
function renderComponentList(state){
 const id=state?.active?state.id:$('recipe').value;
 $('workflowParts').hidden=id!=='ASM-01';
 const list=$('componentList');list.replaceChildren();
 if(id!=='ASM-01')return;
 const names=['Main PCB','Battery','Sensor','Housing','Antenna','Seal'];
 Workflows.required.forEach((part,i)=>{
  const row=document.createElement('div'),head=document.createElement('div'),name=document.createElement('strong'),status=document.createElement('span');
  const done=!!((state?.partMask||0)&(1<<i));row.className='part-check';row.dataset.done=String(done);head.className='part-head';name.textContent=names[i]+' · '+part;status.className='workflow-tag';status.textContent=done?'Verified':'Required';head.append(name,status);row.append(head);
  if(done&&state.partLabels?.[i]){row.append(fieldDetails(state.partLabels[i]));const stamp=document.createElement('small');stamp.textContent=state.partTimes?.[i]?'Recorded '+new Date(state.partTimes[i]).toLocaleString():'Recorded without network time';row.append(stamp)}
  list.append(row);
 });
}
function renderTraceabilityRecord(state){
 const panel=$('workflowRecordPanel'),summary=$('workflowRecordSummary'),payloads=$('workflowRecordPayloads');
 panel.hidden=!state.run;
 if(panel.hidden)return;
 const rows=[
  ['Run ID',state.run],['Status',state.recordStatus],['Scanner',deviceMac],['Recipe',state.id+' · rev '+state.recipeRevision],
  ['Device',state.device],['Test bench / fixture',state.equipment],
  ['Operator',state.operator],['Department',state.department],['Test',state.test],
  ['Started',state.startedAt||'No synchronized clock'],['Start uptime',String(state.startedUptimeMs||0)+' ms'],['Clock synchronized',state.clockSynced?'Yes':'No'],
  ['Result',state.disposition||'Not recorded'],['Failure reason',state.reason],
  ['Result ID',state.resultId],['Measurement reference',state.dataReference]
 ].filter(([,value])=>value!==undefined&&value!==null&&value!=='');
 summary.replaceChildren(...rows.flatMap(([label,value])=>{const dt=document.createElement('dt'),dd=document.createElement('dd');dt.textContent=label;dd.textContent=String(value);return [dt,dd]}));
 const steps=(state.stepLabels||[]).map((payload,i)=>payload?{step:i+1,payload,recordedAt:state.stepTimes?.[i]||null,uptimeMs:state.stepUptimeMs?.[i]||0}:null).filter(Boolean);
 const components=(state.partLabels||[]).map((payload,i)=>payload?{payload,recordedAt:state.partTimes?.[i]||null,uptimeMs:state.partUptimeMs?.[i]||0}:null).filter(Boolean);
 payloads.textContent=JSON.stringify({steps,components},null,2);
}
let lastWorkflowRender='';
function renderWorkflow(state){
 const newRun=state.active&&state.run&&state.run!==workflow?.run;workflow=state;const recipe=$('recipe');
 if(newRun){recipe.value=state.id;$('workflowTab').click()}
 if((state.complete||state.expired)&&recipe.value!==state.id)state={active:false};
 if(state.active&&!state.complete&&!state.expired&&recipe.value!==state.id)recipe.value=state.id;
 recipe.disabled=state.active&&!state.complete&&!state.expired;
 $('startWorkflow').disabled=recipe.disabled;$('resetWorkflow').disabled=!state.active||state.complete;
 const mood=(state.error||state.disposition==='FAIL')?'error':state.complete?'done':state.active?'next':'pending';
 for(const id of ['workflowStatus','workflowBanner']){$(id).dataset.state=mood;$(id).dataset.error=String(state.error)}
 $('workflowStatus').textContent=state.active?state.message+(state.disposition==='FAIL'&&state.reason?' · '+state.reason:''):'Scan the workflow header or start a run.';
 $('workflowBanner').hidden=!state.active;$('workflowBanner').textContent=state.active?Workflows.recipes[state.id].name+' · '+state.message+(state.disposition==='FAIL'&&state.reason?' · '+state.reason:''):'';
 $('workflowTimer').textContent=[state.id==='AGE-01'&&state.active&&state.step>=3?'Elapsed exposure: '+Math.floor(state.dwellSeconds/3600)+'h '+Math.floor(state.dwellSeconds%3600/60)+'m '+state.dwellSeconds%60+'s':'',state.id==='AGE-01'&&state.step===3&&state.dwellSeconds<5?'Rescan chamber in '+(5-state.dwellSeconds)+' s':'',state.remainingMs?'Confirmation remaining: '+Math.ceil(state.remainingMs/1000)+' s':'',state.active?(state.clockSynced?'Clock synchronized':'Clock not yet synchronized'):''].filter(Boolean).join(' · ');
 const key=JSON.stringify([recipe.value,state.run,state.step,state.complete,state.error,state.partMask,state.stepLabels,state.partLabels,state.stepTimes,state.partTimes,state.disposition,state.expired,state.recordStatus,deviceMac]);
 if(key===lastWorkflowRender)return;lastWorkflowRender=key;
 renderTraceabilityRecord(state);
 renderComponentList(state);
 const root=$('workflowSteps');root.replaceChildren();
 const steps=Workflows.recipes[state.active?state.id:recipe.value]?.steps||[];
 $('workflowProgress').textContent=state.active?(state.complete?steps.length:state.step-1)+' / '+steps.length+' steps verified':'Not started';
 $('workflowProgressBar').value=state.active?(state.complete?steps.length:state.step-1):0;$('workflowProgressBar').max=steps.length||1;
 for(const [i,step] of steps.entries()){
  const n=i+1,done=state.active&&(state.complete||n<state.step),current=state.active&&!state.complete&&n===state.step;
  const row=document.createElement('li'),head=document.createElement('div'),label=document.createElement('strong'),tag=document.createElement('span');
  row.className='workflow-step';row.dataset.state=done?'done':current?(state.error?'error':'next'):'pending';
  head.className='workflow-step-head';label.textContent=step;tag.className='workflow-tag';tag.textContent=done?'Verified':current?(state.expired?'Timed out':state.error?'Correction required':'Next'):'Pending';head.append(label,tag);row.append(head);
  if(state.active&&state.id==='ASM-01'&&n===2){const p=document.createElement('small');p.textContent=state.partCount+' / 6 required components verified';row.append(p)}
  if(state.active&&state.stepLabels?.[i]){row.append(fieldDetails(state.stepLabels[i]));const stamp=document.createElement('small');stamp.textContent=state.stepTimes?.[i]?'Recorded '+new Date(state.stepTimes[i]).toLocaleString()+' (local time)':'Recorded without network time';row.append(stamp)}
  root.append(row);
 }
}
