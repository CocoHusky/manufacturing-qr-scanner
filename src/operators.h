#pragma once
#include <mbedtls/sha256.h>
static String activeOperator="",activeOperatorName="";
static String badgeId="",badgeToken="";
static uint32_t badgeUntil=0;
static String hashToken(const String& token){
 unsigned char out[32];mbedtls_sha256_ret((const unsigned char*)token.c_str(),token.length(),out,0);
 String r;for(int i=0;i<32;i++){char b[3];snprintf(b,sizeof b,"%02x",out[i]);r+=b;}return r;
}
static bool validOperatorId(const String& id){if(!id.length()||id.length()>12)return false;for(unsigned i=0;i<id.length();i++)if(!isalnum((unsigned char)id[i])&&id[i]!='-')return false;return true;}
static String operatorName(const String& id){return validOperatorId(id)?prefs.getString(("n"+id).c_str(),""):"";}
static bool operatorAuth(const String& id,const String& token){if(!validOperatorId(id)||token.length()!=32)return false;String expected=prefs.getString(("k"+id).c_str(),"");return expected.length()==64&&expected==hashToken(token);}
static bool registerOperator(const String& id,const String& name){
 if(!validOperatorId(id)||!name.length()||name.length()>80)return false;
 char token[33];snprintf(token,sizeof token,"%08lx%08lx%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random());
 if(!prefs.putString(("n"+id).c_str(),name)||!prefs.putString(("k"+id).c_str(),hashToken(token)))return false;
 Serial.printf("OPERATOR %s TOKEN %s\n",id.c_str(),token);return true;
}
static String issueBadge(const String& id){char t[33];snprintf(t,sizeof t,"%08lx%08lx%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random());badgeId=id;badgeToken=t;badgeUntil=millis()+60000;return "op="+id+";token="+badgeToken;}
static bool identifyOperator(const String& raw){
 if(!raw.startsWith("op="))return false;int sep=raw.indexOf(";token=");if(sep<0)return false;
 String id=raw.substring(3,sep),token=raw.substring(sep+7);
 if(id!=badgeId||token!=badgeToken||!badgeToken.length()||(int32_t)(badgeUntil-millis())<=0||!operatorName(id).length())return false;badgeToken="";activeOperator=id;activeOperatorName=operatorName(id);return true;
}
