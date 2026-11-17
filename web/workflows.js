(function(root){
'use strict';
const recipes={
 'ASM-01':{name:'Component assembly',steps:['Device traveler','Six unique components','Confirm device']},
 'TEST-01':{name:'Benchtop functional test',steps:['Device','Bench','Test result','Confirm device (60 s)']},
 'THERM-01':{name:'Thermal fixture setup',steps:['Fixture','Device','Bag','Power supply','Probe','Test result','Confirm device']},
 'AGE-01':{name:'Environmental aging',steps:['Chamber','Load device','Opening event','Unload device (60 s)']}
};
const required=['PCB-001','BAT-001','SNS-001','HSG-001','ANT-001','SEL-001'];
function labels(id){
 const d='TYPE=DEVICE;SN=DEV-0001;LOT=L261117;REV=A', f=x=>'WF='+id+';'+x;
 const rows=id==='ASM-01'?[d,...required.map((pn,i)=>'TYPE=PART;PN='+pn+';LOT=L261101;SN=C-'+(i+1))]:id==='TEST-01'?[d,'TYPE=BENCH;ID=BENCH-003','TYPE=RESULT;ID=RUN-0042;DEVICE=DEV-0001;BENCH=BENCH-003;DATA=RUN-0042']:id==='THERM-01'?['TYPE=FIXTURE;ID=THERM-003',d,'TYPE=BAG;ID=BAG-001','TYPE=POWER;ID=PSU-001','TYPE=PROBE;ID=TC-001','TYPE=RESULT;ID=RUN-0043;DEVICE=DEV-0001;BENCH=THERM-003;DATA=RUN-0043']:['TYPE=CHAMBER;ID=AGE-001',d,'TYPE=OPEN;ID=OPEN-001;CHAMBER=AGE-001'];
 return rows.map((raw,i)=>f('STEP='+(id==='ASM-01'?(i?2:1):i+1)+';'+raw))
}
const api={recipes,required,labels};if(typeof module!=='undefined')module.exports=api;else root.Workflows=api
})(typeof globalThis!=='undefined'?globalThis:this);
