#pragma once
#include <LittleFS.h>
#include <time.h>
static bool journalReady=false;
static String journalError="";
static constexpr size_t journalLimit=192*1024;
static bool appendJournal(const String& line){
 if(!journalReady){journalError="Storage unavailable";return false;}
 if(line.length()>8192){journalError="Record too large";return false;}
 File f=LittleFS.open("/records.ndjson","a");
 if(!f){journalError="Open failed";return false;}
 if(f.size()+line.length()+1>journalLimit){f.close();journalError="Journal full; export before clearing";return false;}
 bool ok=f.println(line)==line.length()+2;f.close();
 journalError=ok?"":"Write failed";return ok;
}
static String utcNow(){
 time_t ts=time(nullptr);if(ts<1700000000)return "";
 struct tm tm;gmtime_r(&ts,&tm);char b[32];strftime(b,sizeof b,"%Y-%m-%dT%H:%M:%SZ",&tm);return String(b);
}
