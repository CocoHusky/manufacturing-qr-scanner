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
 Serial.begin(115200); pinMode(33,OUTPUT);digitalWrite(33,HIGH);ledcSetup(whitePWMChannel,20000,10);ledcAttachPin(4,whitePWMChannel);setLight(true);pinMode(13,INPUT_PULLUP);delay(1000);
 Serial.printf("\nQR scanner | PSRAM %u bytes\n",ESP.getPsramSize());
 camera_config_t c={};
 c.ledc_channel=LEDC_CHANNEL_0;c.ledc_timer=LEDC_TIMER_0;
 c.pin_d0=5;c.pin_d1=18;c.pin_d2=19;c.pin_d3=21;c.pin_d4=36;c.pin_d5=39;c.pin_d6=34;c.pin_d7=35;
 c.pin_xclk=0;c.pin_pclk=22;c.pin_vsync=25;c.pin_href=23;
 c.pin_sccb_sda=26;c.pin_sccb_scl=27;c.pin_pwdn=32;c.pin_reset=-1;
 c.xclk_freq_hz=20000000;c.pixel_format=PIXFORMAT_JPEG;
 c.frame_size=FRAMESIZE_UXGA;c.jpeg_quality=10;c.fb_count=2;c.fb_location=psramFound()?CAMERA_FB_IN_PSRAM:CAMERA_FB_IN_DRAM;
 c.grab_mode=CAMERA_GRAB_LATEST;
 esp_err_t e=esp_camera_init(&c);if(e!=ESP_OK){esp_camera_deinit();c.fb_count=1;c.grab_mode=CAMERA_GRAB_WHEN_EMPTY;e=esp_camera_init(&c);}
 if(e!=ESP_OK){camStatus="Camera failed: "+String(e,HEX);Serial.println(camStatus);networkSetup();return;}
 sensor_t *sensor=esp_camera_sensor_get(); sensor->set_vflip(sensor,1);sensor->set_hmirror(sensor,0); sensor->set_sharpness(sensor,2);sensor->set_framesize(sensor,FRAMESIZE_XGA);
 camSensor=sensor;
 Serial.printf("Sensor PID: 0x%x\n",sensor->id.PID);
 if(af.start(sensor)){focusState="Loading autofocus";afReady=af.focusInit()==0;focusState=afReady?"AF firmware ready":"AF firmware failed";}else focusState="Not OV5640";
 Serial.println(focusState);
 camStatus="Ready"; networkSetup(); Serial.println("READY: show QR code to camera");
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
void loop(){
 networkLoop();
 if(decodeMode=="device"&&camStatus=="Ready"&&!whiteUntil&&millis()-nativeLast>=nativePeriod){nativeLast=millis();auto *f=esp_camera_fb_get();if(f){nativeDecode(f);esp_camera_fb_return(f);}}
 if(redUntil&&(int32_t)(millis()-redUntil)>=0){digitalWrite(33,HIGH);redUntil=0;}
 if(whiteUntil&&(int32_t)(millis()-whiteUntil)>=0){whiteUntil=0;applyBrightness(ledOn?idleBrightness:0);}
 if(focusRequested){focusRequested=false;focusNow();}
 delay(2);
}
