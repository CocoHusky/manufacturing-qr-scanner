#include <Arduino.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
uint64_t fakeTime=10000;
static String activeOperator="Tester",activeOperatorName="",sessionStarted="2026-11-17",sessionDepartment="QA",sessionTest="Demo";
struct FakeWiFi { String macAddress() const { return "02:00:00:00:00:01"; } };
static FakeWiFi WiFi;
static bool journalReady=true,writeOK=true;
static uint32_t bootId=1;
static int warnings=0,completed=0,timeouts=0;
static String jsString(const String& s){return String("\"")+s+"\"";}
static String utcNow(){return "2026-11-17T00:00:00Z";}
static bool appendJournal(const String&){return writeOK;}
static void signalStart(){}
static void signalWarning(){warnings++;}
static void signalTimeout(){timeouts++;}
static void signalComplete(bool=false){completed++;}
#include "../../src/workflow_engine.h"
static void start(const char* id){workflow=WorkflowState();workflow.recipe=id;workflow.active=true;workflow.run="test";assert(workflow.step==1);}
static bool scan(const String& label){return workflowScan("WF="+workflow.recipe+";"+label);}
static void accept(const String& s){assert(scan(s));}
static void reject(const String& s){auto step=workflow.step;int n=warnings;assert(!scan(s));assert(workflow.step==step&&warnings==n+(workflow.expired?0:1));}
static void testPdf(){
 std::ifstream file("test/fixtures/pipeline-labels.tsv");assert(file);
 std::vector<String> labels;std::string line;while(std::getline(file,line))labels.emplace_back(line);
 assert(labels.size()==29);
 const int begins[]={0,9,15,24},ends[]={9,15,24,29};
 for(int p=0;p<4;p++)for(int variant=0;variant<(p==1||p==2?2:1);variant++){
  workflow=WorkflowState();fakeTime+=1;
  assert(workflowScan(labels[begins[p]],"Code128"));
  for(int i=begins[p]+1;i<ends[p];i++){
   if((p==1&&(i==(variant?12:13)))||(p==2&&(i==(variant?21:22))))continue;
   if(p==3&&i==27)fakeTime+=5000;
   assert(workflowScan(labels[i],"QRCode"));
   fakeTime+=1;
  }
  assert(workflow.complete);
  if(p==1||p==2){assert(workflow.disposition==(variant?"FAIL":"PASS"));assert(workflow.reason==(variant?(p==1?"OUTPUT_LOW":"TEMP_HIGH"):"NONE"));}
 }
 workflow=WorkflowState();assert(workflowScan("0101","QRCode"));assert(!workflow.active);
 assert(!workflowScan("0102","Code128"));assert(!workflow.active);
 workflow=WorkflowState();
 std::cout<<"All six positive paths from decoded PDF labels completed (including FAIL dispositions)\n";
}
int main(){testPdf();
 assert(workflowScan("0201","Code128"));assert(workflow.recipe=="TEST-01"&&workflow.step==1);
 assert(!workflowScan("0301","Code128"));assert(workflow.recipe=="TEST-01");
 accept("STEP=1;TYPE=DEVICE;SN=TEST-DEV-02");accept("STEP=2;TYPE=BENCH;ID=BENCH-007");
 reject("STEP=3;TYPE=RESULT;ID=RUN-0262F;DEVICE=TEST-DEV-02;BENCH=BENCH-007;DATA=RUN-0262F;STATUS=FAIL");
 accept("STEP=3;TYPE=RESULT;ID=RUN-0262F;DEVICE=TEST-DEV-02;BENCH=BENCH-007;DATA=RUN-0262F;STATUS=FAIL;V=4.51V;I=62mA;REASON=OUTPUT_LOW");
 accept("STEP=1;TYPE=DEVICE;SN=TEST-DEV-02");assert(workflow.complete&&workflow.disposition=="FAIL"&&workflow.reason=="OUTPUT_LOW");
 assert(workflowScan("0301","Code128"));assert(workflow.recipe=="THERM-01");

 start("ASM-01");reject("STEP=0;TYPE=DEVICE;SN=D");reject("STEP=2;TYPE=PART;PN=PCB-001;LOT=L;SN=C1");accept("STEP=1;TYPE=DEVICE;SN=D");
 reject("STEP=1;TYPE=DEVICE;SN=D");
 for(int i=0;i<6;i++){String row=String("STEP=2;TYPE=PART;PN=")+assemblyParts[i]+";LOT=L;SN=C"+String(i);accept(row);if(i==0)reject(row);}
 assert(partCount()==6);assert(workflow.partLabels[0].find("SN=C0")!=String::npos);assert(workflow.stepLabels[0].find("SN=D")!=String::npos);reject("STEP=1;TYPE=DEVICE;SN=OTHER");accept("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.complete);
 start("TEST-01");accept("STEP=1;TYPE=DEVICE;SN=D");reject("STEP=3;TYPE=RESULT;ID=R;DEVICE=D;BENCH=B;DATA=R;STATUS=PASS;REASON=NONE;V=5.02V;I=62mA;T=36.2C");accept("STEP=2;TYPE=BENCH;ID=B");reject("STEP=3;TYPE=RESULT;ID=R;DEVICE=OTHER;BENCH=B;DATA=R;STATUS=PASS;REASON=NONE;V=5.02V;I=62mA;T=36.2C");accept("STEP=3;TYPE=RESULT;ID=R;DEVICE=D;BENCH=B;DATA=R;STATUS=PASS;REASON=NONE;V=5.02V;I=62mA;T=36.2C");fakeTime+=60001;workflowTick();assert(workflow.expired);reject("STEP=1;TYPE=DEVICE;SN=D");
 start("TEST-01");accept("STEP=1;TYPE=DEVICE;SN=D");accept("STEP=2;TYPE=BENCH;ID=B");accept("STEP=3;TYPE=RESULT;ID=R;DEVICE=D;BENCH=B;DATA=R;STATUS=PASS;REASON=NONE;V=5.02V;I=62mA;T=36.2C");accept("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.complete);
 start("THERM-01");accept("STEP=1;TYPE=FIXTURE;ID=F");accept("STEP=2;TYPE=DEVICE;SN=D");reject("STEP=4;TYPE=POWER;ID=P");accept("STEP=3;TYPE=BAG;ID=B");accept("STEP=4;TYPE=POWER;ID=P");accept("STEP=5;TYPE=PROBE;ID=T");accept("STEP=6;TYPE=RESULT;ID=R;DEVICE=D;BENCH=F;DATA=R;STATUS=PASS;REASON=NONE;V=5.02V;I=62mA;T=36.2C");accept("STEP=2;TYPE=DEVICE;SN=D");assert(workflow.complete);assert(workflow.stepLabels[3].find("TYPE=POWER")!=String::npos);assert(workflow.stepLabels[5].find("DATA=R")!=String::npos);
 start("AGE-01");reject("STEP=2;TYPE=CHAMBER;ID=A");accept("STEP=1;TYPE=DEVICE;SN=D");fakeTime+=10000;accept("STEP=2;TYPE=CHAMBER;ID=A");reject("STEP=2;TYPE=CHAMBER;ID=A");fakeTime+=5000;reject("STEP=2;TYPE=CHAMBER;ID=X");accept("STEP=2;TYPE=CHAMBER;ID=A");assert(workflow.dwell==5000);reject("STEP=1;TYPE=DEVICE;SN=OTHER");fakeTime+=1000;accept("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.complete&&workflow.dwell==5000);
 start("AGE-01");accept("STEP=1;TYPE=DEVICE;SN=D");accept("STEP=2;TYPE=CHAMBER;ID=A");fakeTime+=5000;accept("STEP=2;TYPE=CHAMBER;ID=A");fakeTime+=60000;workflowTick();assert(workflow.expired);reject("STEP=1;TYPE=DEVICE;SN=D");
 start("ASM-01");writeOK=false;reject("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.device.empty());writeOK=true;reject("STEP=5;TYPE=DEVICE;SN=D");
 std::cout<<"4 firmware workflows passed: sequence, duplicate, wrong identity, timeout, elapsed exposure and storage failure\n";
}
