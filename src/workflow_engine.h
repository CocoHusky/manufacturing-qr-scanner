#pragma once
#include <Arduino.h>
#include <esp_timer.h>

// One authoritative state machine for browser and embedded scan events.
struct WorkflowState {
 String recipe,run,device,equipment,message;
 String serials[6];
 uint8_t step=1,partMask=0;
 bool active=false,complete=false,expired=false,error=false;
 uint64_t deadline=0,loaded=0,dwell=0;
};
static WorkflowState workflow;
static uint64_t workflowNow(){return (uint64_t)esp_timer_get_time()/1000;}
static const char* assemblyParts[]={"PCB-001","BAT-001","SNS-001","HSG-001","ANT-001","SEL-001"};
static bool validRecipe(const String& s){return s=="ASM-01"||s=="TEST-01"||s=="THERM-01"||s=="AGE-01";}
static uint8_t workflowLength(){return workflow.recipe=="ASM-01"?3:workflow.recipe=="THERM-01"?7:4;}
static unsigned partCount(){unsigned n=0;for(int i=0;i<6;i++)if(workflow.partMask&(1<<i))n++;return n;}
static String workflowField(const String& raw,const String& key){
 String v;bool found=false;int from=0;
 while(from<(int)raw.length()){int end=raw.indexOf(';',from);if(end<0)end=raw.length();int eq=raw.indexOf('=',from);
 if(eq>=from&&eq<end&&raw.substring(from,eq)==key){if(found)return "";v=raw.substring(eq+1,end);found=true;}from=end+1;}
 return v;
}
static String workflowJson(){
 uint64_t now=workflowNow();uint32_t remaining=workflow.deadline>now?(uint32_t)(workflow.deadline-now):0;
 return "{\"id\":"+jsString(workflow.recipe)+",\"run\":"+jsString(workflow.run)+",\"device\":"+jsString(workflow.device)+",\"equipment\":"+jsString(workflow.equipment)+",\"step\":"+String(workflow.step)+",\"active\":"+(workflow.active?"true":"false")+",\"complete\":"+(workflow.complete?"true":"false")+",\"expired\":"+(workflow.expired?"true":"false")+",\"error\":"+(workflow.error?"true":"false")+",\"partMask\":"+String(workflow.partMask)+",\"partCount\":"+String(partCount())+",\"remainingMs\":"+String(remaining)+",\"dwellSeconds\":"+String((unsigned long)(workflow.dwell/1000))+",\"message\":"+jsString(workflow.message)+"}";
}
static bool workflowLog(const String& event,const String& raw,const String& reason){
 return appendJournal("{\"engine\":\"Workflow\",\"event\":"+jsString(event)+",\"run\":"+jsString(workflow.run)+",\"recipe\":"+jsString(workflow.recipe)+",\"step\":"+String(workflow.step)+",\"raw\":"+jsString(raw)+",\"message\":"+jsString(reason)+",\"time\":"+jsString(utcNow())+",\"operator\":"+jsString(activeOperator)+",\"department\":"+jsString(sessionDepartment)+",\"test\":"+jsString(sessionTest)+",\"sessionStarted\":"+jsString(sessionStarted)+",\"uptimeMs\":"+String((unsigned long)workflowNow())+"}");
}
static bool workflowReject(const String& raw,const String& message){
 workflow.error=true;workflow.message=message;workflowLog("rejected",raw,message);signalWarning();return false;
}
static void workflowTick(){
 if(workflow.active&&!workflow.complete&&!workflow.expired&&workflow.deadline&&workflowNow()>workflow.deadline){
 workflow.expired=true;workflowReject("","Confirmation deadline expired. Cancel and restart the run.");
 }
}
static bool workflowScan(const String& raw){
 if(!workflow.active||workflow.complete)return true;
 workflowTick();
 if(workflow.expired)return workflowReject(raw,"Confirmation deadline expired");
 if(!journalReady)return workflowReject(raw,"Device log unavailable");
 if(workflowField(raw,"WF")!=workflow.recipe)return workflowReject(raw,"Wrong workflow label");
 const String type=workflowField(raw,"TYPE"),sn=workflowField(raw,"SN"),id=sn.length()?sn:workflowField(raw,"ID"),step=workflowField(raw,"STEP");
 if(!id.length())return workflowReject(raw,"Missing unique serial or identifier");
 const uint8_t at=workflow.step;String expected;
 if(workflow.recipe=="ASM-01"){expected=at==2?"PART":"DEVICE";}
 else if(workflow.recipe=="TEST-01"){const char* kinds[]={"DEVICE","BENCH","RESULT","DEVICE"};expected=kinds[at-1];}
 else if(workflow.recipe=="THERM-01"){const char* kinds[]={"FIXTURE","DEVICE","BAG","POWER","PROBE","RESULT","DEVICE"};expected=kinds[at-1];}
 else {const char* kinds[]={"CHAMBER","DEVICE","OPEN","DEVICE"};expected=kinds[at-1];}
 if(type!=expected)return workflowReject(raw,"Expected "+expected+" at step "+String(at));
 const bool closing=at==workflowLength();
 // A device label is reusable: its entry STEP remains encoded on final confirmation.
 const unsigned labelStep=closing?(workflow.recipe=="THERM-01"||workflow.recipe=="AGE-01"?2:1):at;
 if(step!=String(labelStep))return workflowReject(raw,"Wrong encoded step");
 if(closing&&id!=workflow.device)return workflowReject(raw,"Rescan the original device");
 WorkflowState next=workflow;
 if(type=="DEVICE"&&!closing)next.device=id;
 if(type=="BENCH"||type=="FIXTURE"||type=="CHAMBER")next.equipment=id;
 if(type=="PART"){
  String pn=workflowField(raw,"PN");int i=-1;for(int j=0;j<6;j++)if(pn==assemblyParts[j])i=j;
  if(i<0)return workflowReject(raw,"Unexpected component "+pn);
  if(!sn.length()||!workflowField(raw,"LOT").length())return workflowReject(raw,"Component lot and serial required");
  for(int j=0;j<6;j++)if((workflow.partMask&(1<<j))&&(j==i||workflow.serials[j]==sn))return workflowReject(raw,"Duplicate component "+pn);
  next.partMask|=(1<<i);next.serials[i]=sn;
 }
 if(type=="RESULT"){
  if(workflowField(raw,"DEVICE")!=workflow.device||workflowField(raw,"BENCH")!=workflow.equipment||!workflowField(raw,"DATA").length())return workflowReject(raw,"Result must match device, equipment and data run");
  if(workflow.recipe=="TEST-01")next.deadline=workflowNow()+60000;
 }
 if(workflow.recipe=="AGE-01"){
  if(at==2)next.loaded=workflowNow();
  if(at==3){if(workflowField(raw,"CHAMBER")!=workflow.equipment)return workflowReject(raw,"Wrong chamber opening event");next.deadline=workflowNow()+60000;}
  if(closing){next.dwell=workflowNow()-workflow.loaded;if(next.dwell<86400000ULL)return workflowReject(raw,"Minimum aging duration of 24 hours not met");}
 }
 if(!workflowLog(closing?"completed":"accepted",raw,closing&&workflow.recipe=="AGE-01"?"Dwell seconds: "+String((unsigned long)(next.dwell/1000)):String("")))return workflowReject(raw,"Cannot save workflow event; export/clear records");
 next.step=at+1;if(type=="PART"&&next.partMask!=63)next.step=at;
 next.error=false;next.complete=closing;if(closing)next.step=at;if(next.complete)next.deadline=0;
 next.message=next.complete?"Workflow complete":"Accepted; scan next required label";
 workflow=next;return true;
}
