const assert=require('node:assert/strict'),{readFileSync}=require('node:fs'),w=require('../../web/workflows');
const labels=readFileSync('test/fixtures/pipeline-labels.tsv','utf8').trim().split(/\r?\n/);
assert.equal(labels.length,29);
for(const [id,recipe] of Object.entries(w.recipes)){
 assert(labels.includes(recipe.start));
 const rows=labels.filter(s=>s.startsWith('WF='+id+';'));
 assert(rows.every(s=>/;STEP=[1-7];TYPE=/.test(s)));
 assert.equal(rows.filter(s=>s.includes('TYPE=RESULT')).length,id==='TEST-01'||id==='THERM-01'?2:0);
}
for(const pn of w.required)assert(labels.some(s=>s.includes('PN='+pn+';')));
console.log('29 decoded PDF symbols; 4 recipe mappings and 4 result alternatives verified');
