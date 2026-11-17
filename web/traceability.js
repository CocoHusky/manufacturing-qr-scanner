(function(root){
'use strict';
const ids={'1P':'part','P':'part','1T':'lot','T':'lot','S':'serial','Q':'quantity','10D':'dateCode','4L':'country','V':'supplier','1V':'supplier'};
const keys={part:'part',pn:'part',lot:'lot',batch:'lot',sn:'serial',serial:'serial',qty:'quantity',quantity:'quantity',date:'dateCode',mfg:'dateCode',datecode:'dateCode',country:'country',rev:'revision',revision:'revision',supplier:'supplier',operator:'operator',department:'department',test:'test',action:'action',timestamp:'timestamp',signature:'signature',id:'id',wf:'workflow',step:'step',type:'labelType',device:'device',bench:'equipment',chamber:'chamber',data:'dataReference',status:'resultStatus',reason:'failureReason',v:'voltage',i:'current',t:'temperature'};
const fields=['part','lot','serial','quantity','dateCode','country','revision','supplier','operator','department','test','action','timestamp','signature','id','workflow','step','labelType','device','equipment','chamber','dataReference','resultStatus','failureReason','voltage','current','temperature'];
function normalize(raw){return String(raw).replace(/␞/g,'\x1e').replace(/␝/g,'\x1d').replace(/␄/g,'\x04')}
function parse(raw,rules=[]){
 raw=String(raw);const s=normalize(raw),f={},unknown=[],warnings=[];let type='Plain identifier';
 function set(k,v){if(!fields.includes(k))return;v=String(v);if(k in f&&f[k]!==v)warnings.push('Conflicting '+k);else f[k]=v}
 if(s.startsWith('[)>\x1e')){
 type='ISO/IEC 15434';
 if(!s.startsWith('[)>\x1e06\x1d'))warnings.push('Unsupported format header');
 else{type='ISO/IEC 15434 / 06';let body=s.slice(7);if(!body.endsWith('\x1e\x04'))warnings.push('Missing message terminator');else body=body.slice(0,-2);
 for(const token of body.split('\x1d')){let m=token.match(/^([0-9]*[A-Z])(.*)$/s);if(!m||!ids[m[1]])unknown.push(token);else set(ids[m[1]],m[2]);}}
 }else if(s.startsWith('{')){try{let obj=JSON.parse(s);if(!obj||Array.isArray(obj)||typeof obj!=='object')throw Error();type='JSON';for(const [k,v] of Object.entries(obj)){let field=keys[k.toLowerCase()];if(field&&['string','number'].includes(typeof v))set(field,v);else unknown.push(k)}}catch(e){warnings.push('Invalid JSON')}}
 else if(/^[A-Za-z][\w-]*=/.test(s)){type='Key/value';for(const token of s.split(';')){let i=token.indexOf('=');if(i<1){unknown.push(token);continue}let k=keys[token.slice(0,i).toLowerCase()];if(k)set(k,token.slice(i+1));else unknown.push(token)}}
 else{for(const r of rules){if(!r||typeof r.prefix!=='string'||!r.prefix||!Array.isArray(r.fields)||typeof r.separator!=='string'||!r.separator)continue;if(!s.startsWith(r.prefix))continue;type='Custom: '+r.prefix;let parts=s.slice(r.prefix.length).split(r.separator);if(parts.length!==r.fields.length)warnings.push('Unexpected field count');r.fields.forEach((k,i)=>{if(i<parts.length)set(k,parts[i])});break}if(type==='Plain identifier')set('id',s)}
 if(type!=='Plain identifier'&&!f.part&&!f.id&&!f.serial&&!f.operator)warnings.push('Missing part or identifier');if(type==='ISO/IEC 15434 / 06'&&!f.lot&&!f.serial)warnings.push('Missing lot or serial');
 if('quantity' in f&&!/^\d+(\.\d+)?$/.test(f.quantity))warnings.push('Invalid quantity');
 if('country' in f&&!/^[A-Z]{2}$/.test(f.country))warnings.push('Invalid country code');
 if('dateCode' in f&&/^\d{4}$/.test(f.dateCode)){let week=Number(f.dateCode.slice(2));if(week<1||week>53)warnings.push('Invalid YYWW date code')}
 for(const [k,v] of Object.entries(f))if(!v.trim())warnings.push('Empty '+k);
 if(unknown.length)warnings.push('Unrecognized fields');
 return {type,fields:f,unknown,warnings};
}
function csvCell(x){let s=String(x??'');if(/^[=+\-@\t\r\n]/.test(s))s="'"+s;return '"'+s.replace(/"/g,'""')+'"'}
function csv(rows){let header=['time','engine','format','raw',...fields,'description','warnings'];return [header.join(','),...rows.map(r=>header.map(k=>csvCell(k==='warnings'?r.parsed.warnings.join('; '):r.parsed.fields[k]??r[k])).join(','))].join('\r\n')}
const api={parse,normalize,csv,fields};if(typeof module!=='undefined')module.exports=api;else root.Traceability=api;
})(typeof globalThis!=='undefined'?globalThis:this);
