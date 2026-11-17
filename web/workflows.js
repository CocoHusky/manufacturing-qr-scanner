(function(root){
'use strict';
const recipes={
 'ASM-01':{start:'0101',name:'Component assembly',steps:['Device traveler','Six unique components','Confirm device']},
 'TEST-01':{start:'0201',name:'Benchtop functional test',steps:['Device','Bench','Test result','Confirm device (60 s)']},
 'THERM-01':{start:'0301',name:'Thermal fixture setup',steps:['Fixture','Device','Bag','Power supply','Probe','Test result','Confirm device']},
 'AGE-01':{start:'0401',name:'Environmental aging',steps:['Identify device','Load chamber — start timer','Rescan chamber — stop timer (min. 5 s)','Confirm removed device (60 s)']}
};
const required=['PCB-001','BAT-001','SNS-001','HSG-001','ANT-001','SEL-001'];
const api={recipes,required};if(typeof module!=='undefined')module.exports=api;else root.Workflows=api
})(typeof globalThis!=='undefined'?globalThis:this);
