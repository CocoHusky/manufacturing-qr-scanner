#include <Arduino.h>
#include "esp_camera.h"
#include "quirc.h"
#include <vector>
#include <string>
#include "network.h"
#include <ESP32_OV5640_AF.h>
static OV5640 af;
static bool afReady=false;
static sensor_t *camSensor=nullptr;
static camera_config_t cameraCfg={};
// Arduino's default early confirmation is disabled: validate after startup.
extern "C" bool verifyRollbackLater(){return true;}
static bool workerAvailable=false;
static bool otaTrial=false;
static uint32_t trialStart=0;
static void checkOtaTrial(){
 if(!otaTrial)return;
 if(millis()-trialStart<15000)return;
 if(camStatus=="Ready"&&WiFi.status()==WL_CONNECTED&&workerAvailable&&!workerBusy){
  String result=nativeSelfTest();
  if(result.indexOf("\"pass\":false")<0&&result.indexOf("\"pass\":true")>=0){
   if(esp_ota_mark_app_valid_cancel_rollback()==ESP_OK){otaTrial=false;Serial.println("OTA candidate confirmed");return;}
  }
 }
 if(millis()-trialStart>60000){Serial.println("OTA candidate failed checks");esp_ota_mark_app_invalid_rollback_and_reboot();otaTrial=false;}
}

static uint32_t nativeDoneAt=0;
struct Seen { String data; uint32_t last; };
static std::vector<Seen> seen;
static quirc *decoder;
static quirc_code code;
static quirc_data data;
static bool fresh(const String &s) {
  uint32_t now=millis();
  for(auto &v:seen) if(v.data==s) { bool ok=now-v.last>2500; v.last=now; return ok; }
  if(seen.size()>=100) seen.erase(seen.begin());
  seen.push_back({s,now}); return true;
}
void setup(){
 Serial.begin(115200);esp_ota_img_states_t state;if(esp_ota_get_state_partition(esp_ota_get_running_partition(),&state)==ESP_OK&&state==ESP_OTA_IMG_PENDING_VERIFY){otaTrial=true;trialStart=millis();} pinMode(33,OUTPUT);digitalWrite(33,HIGH);ledcSetup(whitePWMChannel,20000,10);ledcAttachPin(4,whitePWMChannel);setLight(true);digitalWrite(buzzerPin,LOW);pinMode(buzzerPin,OUTPUT);delay(1000);
 Serial.printf("\nQR scanner | PSRAM %u bytes\n",ESP.getPsramSize());
 auto &c=cameraCfg;
 c.ledc_channel=LEDC_CHANNEL_0;c.ledc_timer=LEDC_TIMER_0;
 c.pin_d0=5;c.pin_d1=18;c.pin_d2=19;c.pin_d3=21;c.pin_d4=36;c.pin_d5=39;c.pin_d6=34;c.pin_d7=35;
 c.pin_xclk=0;c.pin_pclk=22;c.pin_vsync=25;c.pin_href=23;
 c.pin_sccb_sda=26;c.pin_sccb_scl=27;c.pin_pwdn=32;c.pin_reset=-1;
 c.xclk_freq_hz=20000000;c.pixel_format=PIXFORMAT_JPEG;
 c.frame_size=FRAMESIZE_UXGA;c.jpeg_quality=10;c.fb_count=2;c.fb_location=psramFound()?CAMERA_FB_IN_PSRAM:CAMERA_FB_IN_DRAM;
 c.grab_mode=CAMERA_GRAB_LATEST;
 esp_err_t e=esp_camera_init(&c);if(e!=ESP_OK){esp_camera_deinit();c.fb_count=1;c.grab_mode=CAMERA_GRAB_WHEN_EMPTY;e=esp_camera_init(&c);}
 if(e!=ESP_OK){camStatus="Camera failed: "+String(e,HEX);Serial.println(camStatus);networkSetup();return;}
 sensor_t *sensor=esp_camera_sensor_get(); sensor->set_vflip(sensor,1);sensor->set_hmirror(sensor,0); sensor->set_sharpness(sensor,2);sensor->set_framesize(sensor,FRAMESIZE_VGA);
 camSensor=sensor;
 Serial.printf("Sensor PID: 0x%x\n",sensor->id.PID);
 if(af.start(sensor)){focusState="Loading autofocus";afReady=af.focusInit()==0;focusState=afReady?"AF firmware ready":"AF firmware failed";}else focusState="Not OV5640";
 Serial.println(focusState);
 camStatus="Ready";workerAvailable=startWorker(); networkSetup(); Serial.println("READY: show QR code to camera");
}
static void focusNow(){
 if(!afReady){focusState="Autofocus unavailable";return;}
 focusState="Focusing";
 if(camSensor->set_reg(camSensor,0x3023,0xff,1)<0 || camSensor->set_reg(camSensor,0x3022,0xff,3)<0){focusState="AF command failed";return;}
 uint32_t start=millis();
 while(millis()-start<3000){
  int status=camSensor->get_reg(camSensor,0x3029,0xff);
  if(status==0x10){focusState="Focused (sensor report)";return;}
  delay(40);
 }
 focusState="Focus timeout (check AF motor power)";
}
static void pollButton(){
 static bool raw=HIGH,stable=HIGH;static uint32_t changed=0,first=0;static unsigned clicks=0;
 uint32_t now=millis();bool r=digitalRead(13);
 if(r!=raw){raw=r;changed=now;}
 if(raw!=stable&&now-changed>=25){stable=raw;if(stable==LOW){if(clicks==1&&now-first<=350){setLight(!ledOn);clicks=0;}else{if(clicks==1)captureRequested=true;clicks=1;first=now;}}}
 if(clicks==1&&now-first>350){clicks=0;captureRequested=true;captureState="Capture queued";}
}
static bool cameraSleep(){
 if(asleep)return true;if(workerBusy||camStatus!="Ready")return false;
 if(esp_camera_deinit()!=ESP_OK)return false;
 pinMode(32,OUTPUT);digitalWrite(32,HIGH);asleep=true;afReady=false;camSensor=nullptr;whiteUntil=0;applyBrightness(0);return true;
}
static bool cameraWake(){
 if(!asleep){lastActivity=millis();return camStatus=="Ready";}
 digitalWrite(32,LOW);delay(10);
 if(esp_camera_init(&cameraCfg)!=ESP_OK)return false;
 camSensor=esp_camera_sensor_get();if(!camSensor){esp_camera_deinit();return false;}
 camSensor->set_vflip(camSensor,1);camSensor->set_hmirror(camSensor,0);camSensor->set_sharpness(camSensor,2);
 framesize_t sz=scanResolution=="qvga"?FRAMESIZE_QVGA:scanResolution=="cif"?FRAMESIZE_CIF:scanResolution=="vga"?FRAMESIZE_VGA:scanResolution=="svga"?FRAMESIZE_SVGA:scanResolution=="uxga"?FRAMESIZE_UXGA:FRAMESIZE_XGA;
 camSensor->set_framesize(camSensor,sz);camSensor->set_quality(camSensor,jpegQuality);
 afReady=af.start(camSensor)&&af.focusInit()==0;focusState=afReady?"AF firmware ready":"AF unavailable";
 asleep=false;lastActivity=millis();applyBrightness(ledOn?idleBrightness:0);return true;
}
static void applyPendingResolution(){
 if(workerBusy||!pendingResolution.length())return;
 String v=pendingResolution;
 if(asleep&&!cameraWake()){resolutionError="Camera wake failed";pendingResolution="";return;}
 framesize_t sz=v=="qvga"?FRAMESIZE_QVGA:v=="cif"?FRAMESIZE_CIF:v=="vga"?FRAMESIZE_VGA:v=="svga"?FRAMESIZE_SVGA:v=="uxga"?FRAMESIZE_UXGA:FRAMESIZE_XGA;
 auto *sensor=esp_camera_sensor_get();
 if(!sensor||sensor->set_framesize(sensor,sz)!=0)resolutionError="Camera setting failed";
 else{scanResolution=v;for(int i=0;i<2;i++){auto *f=esp_camera_fb_get();if(f)esp_camera_fb_return(f);}lastActivity=millis();}
 pendingResolution="";
}
void loop(){
 networkLoop();
 checkOtaTrial();
 static String cmd;while(Serial.available()){char c=Serial.read();if(c=='\n'||c=='\r'){if(cmd=="OTAKEY")Serial.println("OTAKEY="+otaPassword);else if(cmd.startsWith("OPADD ")){int split=cmd.indexOf(' ',6);if(split>6&&!registerOperator(cmd.substring(6,split),cmd.substring(split+1)))Serial.println("Operator registration failed");}else if(cmd.startsWith("OPDEL ")){String id=cmd.substring(6);if(validOperatorId(id)){prefs.remove(("n"+id).c_str());prefs.remove(("k"+id).c_str());if(activeOperator==id){activeOperator="";activeOperatorName="";}Serial.println("Operator revoked");}}cmd="";}else if(cmd.length()<120)cmd+=c;}

 uint8_t done=0;if(workerBusy&&xQueueReceive(decodeDone,&done,0)==pdTRUE){if(decodeMode=="device"&&!scanningPaused){if(asyncResult.value.length()&&asyncResult.value!=lastNative&&!asyncResult.value.startsWith("op=")&&completedFrame){auto *image=(uint8_t*)heap_caps_realloc(scanImage,completedFrame->len,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);if(image){scanImage=image;scanImageLength=completedFrame->len;memcpy(scanImage,completedFrame->buf,scanImageLength);scanImageSequence++;}}commitNative(asyncResult);}if(completedFrame){esp_camera_fb_return(completedFrame);completedFrame=nullptr;}workerBusy=false;nativeDoneAt=millis();}
 if(!otaActive)applyPendingResolution();
 if(!otaTrial&&!otaActive&&decodeMode=="device"&&!scanningPaused&&camStatus=="Ready"&&!asleep&&!workerBusy&&!whiteUntil&&millis()-nativeLast>=nativePeriod){nativeLast=millis();auto *f=esp_camera_fb_get();if(f){if(workerAvailable){workerBusy=true;if(xQueueSend(decodeJobs,&f,0)!=pdTRUE){workerBusy=false;esp_camera_fb_return(f);}}else{nativeDecode(f);esp_camera_fb_return(f);}}}
 if(!otaTrial&&!otaActive&&autoStandby&&decodeMode!="device"&&!workerBusy&&!asleep&&millis()-lastActivity>=standbyDelay)cameraSleep();
 if(buzzerUntil&&(int32_t)(millis()-buzzerUntil)>=0){digitalWrite(buzzerPin,LOW);buzzerUntil=0;}
 if(redUntil&&(int32_t)(millis()-redUntil)>=0){digitalWrite(33,HIGH);redUntil=0;}
 if(whiteUntil&&(int32_t)(millis()-whiteUntil)>=0){whiteUntil=0;applyBrightness(ledOn?idleBrightness:0);}
 if(focusRequested&&!workerBusy&&!asleep){focusRequested=false;focusNow();}
 delay(2);
}
