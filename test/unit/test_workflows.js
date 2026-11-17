const assert=require('node:assert/strict'),w=require('../../web/workflows');
for(const [id,recipe] of Object.entries(w.recipes)){
 const labels=w.labels(id);assert(labels.length>=3);
 for(const raw of labels){assert(raw.startsWith('WF='+id+';STEP='));assert(raw.includes('TYPE='));assert(raw.includes('SN=')||raw.includes('ID='))}
}
console.log('4 example label sets passed');
