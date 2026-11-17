#pragma once
#include <Arduino.h>
#include <esp_timer.h>

// One authoritative state machine for browser and embedded scan events.
struct WorkflowState {
 String recipe,run,device,equipment,message,disposition,reason,resultId,dataReference;
 String stepTimes[7],partTimes[6];
 String serials[6];
 String stepLabels[7],partLabels[6];
 uint8_t step=1,partMask=0;
 bool active=false,complete=false,expired=false,error=false;
 uint64_t deadline=0,loaded=0,dwell=0;
};
static WorkflowState workflow;
static uint64_t workflowNow(){return (uint64_t)esp_timer_get_time()/1000;}
static const char* assemblyParts[]={"PCB-001","BAT-001","SNS-001","HSG-001","ANT-001","SEL-001"};
static bool validRecipe(const String& s){return s=="ASM-01"||s=="TEST-01"||s=="THERM-01"||s=="AGE-01";}
static String workflowStartCode(const String& raw){if(raw=="0101")return "ASM-01";if(raw=="0201")return "TEST-01";if(raw=="0301")return "THERM-01";if(raw=="0401")return "AGE-01";return "";}
// Reserve four-digit Code 128 recipe headers: two recipe digits, two revision digits.
static bool isWorkflowHeader(const String& raw,const String& fmt){
 return fmt=="Code128"&&raw.length()==4&&raw[0]=='0'&&raw[1]>='1'&&raw[1]<='4'&&raw[2]>='0'&&raw[2]<='9'&&raw[3]>='0'&&raw[3]<='9';
}
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
 String details="[",components="[",times="[",partTimes="[";for(int i=0;i<7;i++){if(i){details+=",";times+=",";}details+=jsString(workflow.stepLabels[i]);times+=jsString(workflow.stepTimes[i]);}details+="]";times+="]";for(int i=0;i<6;i++){if(i){components+=",";partTimes+=",";}components+=jsString(workflow.partLabels[i]);partTimes+=jsString(workflow.partTimes[i]);}components+="]";partTimes+="]";
 return "{\"id\":"+jsString(workflow.recipe)+",\"recipeRevision\":\"01\",\"disposition\":"+jsString(workflow.disposition)+",\"reason\":"+jsString(workflow.reason)+",\"resultId\":"+jsString(workflow.resultId)+",\"dataReference\":"+jsString(workflow.dataReference)+",\"run\":"+jsString(workflow.run)+",\"device\":"+jsString(workflow.device)+",\"equipment\":"+jsString(workflow.equipment)+",\"step\":"+String(workflow.step)+",\"active\":"+(workflow.active?"true":"false")+",\"complete\":"+(workflow.complete?"true":"false")+",\"expired\":"+(workflow.expired?"true":"false")+",\"error\":"+(workflow.error?"true":"false")+",\"partMask\":"+String(workflow.partMask)+",\"partCount\":"+String(partCount())+",\"remainingMs\":"+String(remaining)+",\"dwellSeconds\":"+String((unsigned long)((workflow.recipe=="AGE-01"&&workflow.step==3?now-workflow.loaded:workflow.dwell)/1000))+",\"message\":"+jsString(workflow.message) +",\"clockSynced\":"+(utcNow().length()?"true":"false")+",\"stepTimes\":"+times+",\"partTimes\":"+partTimes+",\"stepLabels\":"+details+",\"partLabels\":"+components+"}";
}
static bool workflowLog(const String& event,const String& raw,const String& reason,const WorkflowState* candidate=nullptr){
 const WorkflowState& record=candidate?*candidate:workflow;
 return appendJournal("{\"engine\":\"Workflow\",\"event\":"+jsString(event)+",\"run\":"+jsString(workflow.run)+",\"recipeRevision\":\"01\",\"disposition\":"+jsString(record.disposition)+",\"failureReason\":"+jsString(record.disposition=="FAIL"?record.reason:String(""))+",\"resultId\":"+jsString(record.resultId)+",\"dataReference\":"+jsString(record.dataReference)+",\"device\":"+jsString(record.device)+",\"equipment\":"+jsString(record.equipment)+",\"recipe\":"+jsString(workflow.recipe)+",\"step\":"+String(workflow.step)+",\"raw\":"+jsString(raw)+",\"message\":"+jsString(reason)+",\"time\":"+jsString(utcNow())+",\"operator\":"+jsString(activeOperator)+",\"department\":"+jsString(sessionDepartment)+",\"test\":"+jsString(sessionTest)+",\"sessionStarted\":"+jsString(sessionStarted)+",\"uptimeMs\":"+String((unsigned long)workflowNow())+"}");
}
static bool startWorkflow(const String& id,const String& raw){
 if(!sessionStarted.length()){workflow.message="Sign on first";return false;}
 if(workflow.active&&!workflow.complete&&!workflow.expired){workflow.message="Active run: finish or cancel before starting another";return false;}
 WorkflowState prior=workflow;workflow=WorkflowState();workflow.recipe=id;workflow.run=String((unsigned long)workflowNow())+"-"+String(bootId);workflow.active=true;workflow.message="Scan first required label";
 if(!workflowLog("started",raw,"")){workflow=prior;workflow.message="Unable to save new run";return false;}return true;
}
static bool workflowReject(const String& raw,const String& message){
 workflow.error=true;workflow.message=message;workflowLog("rejected",raw,message);signalWarning();return false;
}
static void workflowTick(){
 if(workflow.active&&!workflow.complete&&!workflow.expired&&workflow.deadline&&workflowNow()>=workflow.deadline){
 workflow.expired=true;workflow.error=true;workflow.message="Run failed: confirmation deadline expired";workflowLog("timeout","",workflow.message);signalTimeout();
 }
}
static bool workflowScan(const String& raw,const String& fmt=""){
 if(isWorkflowHeader(raw,fmt)){
  String recipe=workflowStartCode(raw);
  if(!recipe.length())return workflowReject(raw,"Unsupported recipe revision");
  if(startWorkflow(recipe,raw))return true;
  if(workflow.active&&!workflow.complete&&!workflow.expired)return workflowReject(raw,workflow.message);
  return false;
 }
 if(!workflow.active||workflow.complete)return true;
 workflowTick();
 if(workflow.expired)return false;
 if(!journalReady)return workflowReject(raw,"Device log unavailable");
 if(raw.length()>1024)return workflowReject(raw,"Workflow label exceeds 1024 bytes");
 if(workflowField(raw,"WF")!=workflow.recipe)return workflowReject(raw,"Wrong workflow label");
 const String type=workflowField(raw,"TYPE"),sn=workflowField(raw,"SN"),id=sn.length()?sn:workflowField(raw,"ID"),step=workflowField(raw,"STEP");
 if(!id.length())return workflowReject(raw,"Missing unique serial or identifier");
 const uint8_t at=workflow.step;String expected;
 if(workflow.recipe=="ASM-01"){expected=at==2?"PART":"DEVICE";}
 else if(workflow.recipe=="TEST-01"){const char* kinds[]={"DEVICE","BENCH","RESULT","DEVICE"};expected=kinds[at-1];}
 else if(workflow.recipe=="THERM-01"){const char* kinds[]={"FIXTURE","DEVICE","BAG","POWER","PROBE","RESULT","DEVICE"};expected=kinds[at-1];}
 else {const char* kinds[]={"DEVICE","CHAMBER","CHAMBER","DEVICE"};expected=kinds[at-1];}
 if(type!=expected)return workflowReject(raw,"Expected "+expected+" at step "+String(at));
 const bool closing=at==workflowLength();
 // A device label is reusable: its entry STEP remains encoded on final confirmation.
 const unsigned labelStep=closing?(workflow.recipe=="THERM-01"?2:1):(workflow.recipe=="AGE-01"&&at==3?2:at);
 if(step!=String(labelStep))return workflowReject(raw,"Wrong encoded step");
 if(closing&&id!=workflow.device)return workflowReject(raw,"Rescan the original device");
 WorkflowState next=workflow;
 if(type=="DEVICE"&&!closing)next.device=id;
 if(type=="BENCH"||type=="FIXTURE"||(type=="CHAMBER"&&at==2))next.equipment=id;
 if(type=="PART"){
  String pn=workflowField(raw,"PN");int i=-1;for(int j=0;j<6;j++)if(pn==assemblyParts[j])i=j;
  if(i<0)return workflowReject(raw,"Unexpected component "+pn);
  if(!sn.length()||!workflowField(raw,"LOT").length())return workflowReject(raw,"Component lot and serial required");
  for(int j=0;j<6;j++)if((workflow.partMask&(1<<j))&&(j==i||workflow.serials[j]==sn))return workflowReject(raw,"Duplicate component "+pn);
  next.partMask|=(1<<i);next.serials[i]=sn;next.partLabels[i]=raw;next.partTimes[i]=utcNow();
 }
 if(type=="RESULT"){
  if(workflowField(raw,"DEVICE")!=workflow.device||workflowField(raw,"BENCH")!=workflow.equipment||!workflowField(raw,"DATA").length())return workflowReject(raw,"Result must match device, equipment and data run");
  String status=workflowField(raw,"STATUS"),reason=workflowField(raw,"REASON");
  if(status!="PASS"&&status!="FAIL")return workflowReject(raw,"Result must specify PASS or FAIL");
  if(status=="FAIL"&&(!reason.length()||reason=="NONE"))return workflowReject(raw,"Failure reason required");
  if(status=="PASS"&&reason.length()&&reason!="NONE")return workflowReject(raw,"PASS result cannot include a failure reason");
  if(workflow.recipe=="TEST-01"&&(!workflowField(raw,"V").length()||!workflowField(raw,"I").length()))return workflowReject(raw,"Voltage and current required");
  if(workflow.recipe=="THERM-01"&&!workflowField(raw,"T").length())return workflowReject(raw,"Temperature required");
  next.disposition=status;next.reason=reason;next.resultId=id;next.dataReference=workflowField(raw,"DATA");
  if(workflow.recipe=="TEST-01")next.deadline=workflowNow()+60000;
 }
 if(workflow.recipe=="AGE-01"){
  if(at==2)next.loaded=workflowNow();
  if(at==3){if(id!=workflow.equipment)return workflowReject(raw,"Rescan the original chamber");uint64_t elapsed=workflowNow()-workflow.loaded;if(elapsed<5000)return workflowReject(raw,"Wait at least 5 seconds after chamber loading");next.dwell=elapsed;next.deadline=workflowNow()+60000;}
 }
 if(!workflowLog(closing?"completed":"accepted",raw,(closing||at==3)&&workflow.recipe=="AGE-01"?"Dwell seconds: "+String((unsigned long)(next.dwell/1000)):String(""),&next))return workflowReject(raw,"Cannot save workflow event; export/clear records");
 if(type!="PART"){next.stepLabels[at-1]=raw;next.stepTimes[at-1]=utcNow();}
 next.step=at+1;if(type=="PART"&&next.partMask!=63)next.step=at;
 next.error=false;next.complete=closing;if(closing)next.step=at;if(next.complete)next.deadline=0;
 next.message=next.complete?(next.disposition.length()?"Workflow complete — test "+next.disposition:String("Workflow complete")):("Accepted; scan next required label");
 workflow=next;if(closing)signalComplete();return true;
}
