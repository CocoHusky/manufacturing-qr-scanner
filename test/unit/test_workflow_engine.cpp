#include <Arduino.h>
#include <cassert>
#include <iostream>
uint64_t fakeTime=10000;
static String activeOperator="Tester",sessionStarted="2026-11-17",sessionDepartment="QA",sessionTest="Demo";
static bool journalReady=true,writeOK=true;
static int warnings=0;
static String jsString(const String& s){return String("\"")+s+"\"";}
static String utcNow(){return "2026-11-17T00:00:00Z";}
static bool appendJournal(const String&){return writeOK;}
static void signalWarning(){warnings++;}
#include "../../src/workflow_engine.h"
static void start(const char* id){workflow=WorkflowState();workflow.recipe=id;workflow.active=true;workflow.run="test";assert(workflow.step==1);}
static bool scan(const String& label){return workflowScan("WF="+workflow.recipe+";"+label);}
static void accept(const String& s){assert(scan(s));}
static void reject(const String& s){auto step=workflow.step;int n=warnings;assert(!scan(s));assert(workflow.step==step&&warnings==n+1);}
int main(){
 start("ASM-01");reject("STEP=0;TYPE=DEVICE;SN=D");reject("STEP=2;TYPE=PART;PN=PCB-001;LOT=L;SN=C1");accept("STEP=1;TYPE=DEVICE;SN=D");
 reject("STEP=1;TYPE=DEVICE;SN=D");
 for(int i=0;i<6;i++){String row=String("STEP=2;TYPE=PART;PN=")+assemblyParts[i]+";LOT=L;SN=C"+String(i);accept(row);if(i==0)reject(row);}
 assert(partCount()==6);reject("STEP=1;TYPE=DEVICE;SN=OTHER");accept("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.complete);
 start("TEST-01");accept("STEP=1;TYPE=DEVICE;SN=D");reject("STEP=3;TYPE=RESULT;ID=R;DEVICE=D;BENCH=B;DATA=R");accept("STEP=2;TYPE=BENCH;ID=B");reject("STEP=3;TYPE=RESULT;ID=R;DEVICE=OTHER;BENCH=B;DATA=R");accept("STEP=3;TYPE=RESULT;ID=R;DEVICE=D;BENCH=B;DATA=R");fakeTime+=60001;workflowTick();assert(workflow.expired);reject("STEP=1;TYPE=DEVICE;SN=D");
 start("TEST-01");accept("STEP=1;TYPE=DEVICE;SN=D");accept("STEP=2;TYPE=BENCH;ID=B");accept("STEP=3;TYPE=RESULT;ID=R;DEVICE=D;BENCH=B;DATA=R");accept("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.complete);
 start("THERM-01");accept("STEP=1;TYPE=FIXTURE;ID=F");accept("STEP=2;TYPE=DEVICE;SN=D");reject("STEP=4;TYPE=POWER;ID=P");accept("STEP=3;TYPE=BAG;ID=B");accept("STEP=4;TYPE=POWER;ID=P");accept("STEP=5;TYPE=PROBE;ID=T");accept("STEP=6;TYPE=RESULT;ID=R;DEVICE=D;BENCH=F;DATA=R");accept("STEP=2;TYPE=DEVICE;SN=D");assert(workflow.complete);
 start("AGE-01");accept("STEP=1;TYPE=CHAMBER;ID=A");accept("STEP=2;TYPE=DEVICE;SN=D");reject("STEP=3;TYPE=OPEN;ID=O;CHAMBER=X");accept("STEP=3;TYPE=OPEN;ID=O;CHAMBER=A");reject("STEP=2;TYPE=DEVICE;SN=D");
 start("AGE-01");accept("STEP=1;TYPE=CHAMBER;ID=A");accept("STEP=2;TYPE=DEVICE;SN=D");fakeTime+=86400000;accept("STEP=3;TYPE=OPEN;ID=O;CHAMBER=A");accept("STEP=2;TYPE=DEVICE;SN=D");assert(workflow.complete&&workflow.dwell==86400000);
 start("ASM-01");writeOK=false;reject("STEP=1;TYPE=DEVICE;SN=D");assert(workflow.device.empty());writeOK=true;reject("STEP=5;TYPE=DEVICE;SN=D");
 std::cout<<"4 firmware workflows passed: sequence, duplicate, wrong identity, timeout, premature removal and storage failure\n";
}
