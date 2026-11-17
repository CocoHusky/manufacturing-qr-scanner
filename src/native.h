#pragma once
#include <ReadBarcode.h>
#include "native_fixtures.h"
#include <ReaderOptions.h>
#include <esp_jpg_decode.h>
#include <esp_heap_caps.h>
static String decodeMode="browser", nativeValue="", nativeFormat="", nativeError="";
static uint32_t nativeMs=0,nativeFrames=0,nativeLast=0,nativePeriod=500;
static String lastNative="";
static uint32_t nativeMissing=0;
static uint8_t *nativeGray=nullptr;
static size_t nativeCapacity=0;
struct GrayJPEG {camera_fb_t *f;uint8_t *gray;int x,y,w,h;};
static size_t jpegRead(void *arg,size_t i,uint8_t *buf,size_t n){
 auto *a=(GrayJPEG*)arg;if(i>=a->f->len)return 0;n=min(n,a->f->len-i);if(buf)memcpy(buf,a->f->buf+i,n);return n;
}
static bool jpegWrite(void *arg,uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint8_t *data){
 if(!data)return true;auto *a=(GrayJPEG*)arg;
 for(int j=0;j<h;j++)for(int i=0;i<w;i++){
 int xx=x+i,yy=y+j;if(xx>=a->x||yy>=a->y)continue;
 int dx=xx*a->w/a->x,dy=yy*a->h/a->y;
 auto *p=data+3*(j*w+i);a->gray[dy*a->w+dx]=(77*p[0]+150*p[1]+29*p[2])>>8;
 }return true;
}
struct NativeResult{String value,format,error;uint32_t ms=0;};
static ZXing::Result decodePolarities(uint8_t *g,int w,int h,const ZXing::ReaderOptions& opts){
 return ZXing::ReadBarcode(ZXing::ImageView(g,w,h,ZXing::ImageFormat::Lum),opts);
}
static NativeResult decodeFrame(camera_fb_t *f){NativeResult result;
 uint32_t t=millis();
 // Preserve full field of view; downsample to bound decoder memory.
 int w=min((int)f->width,640),h=min((int)f->height,480);
 if(nativeCapacity<(size_t)(w*h)){free(nativeGray);nativeGray=(uint8_t*)heap_caps_malloc(w*h,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);nativeCapacity=nativeGray?w*h:0;}uint8_t *g=nativeGray;
 if(!g){result.error="PSRAM allocation failed";result.ms=millis()-t;return result;}
 GrayJPEG a{f,g,(int)f->width,(int)f->height,w,h};memset(g,255,w*h);
 if(esp_jpg_decode(f->len,JPG_SCALE_NONE,jpegRead,jpegWrite,&a)!=ESP_OK)result.error="JPEG conversion failed";
 else try{
 auto opts=ZXing::ReaderOptions().setFormats(ZXing::BarcodeFormat::QRCode|ZXing::BarcodeFormat::DataMatrix).setTryHarder(true).setTryRotate(true).setTryDownscale(false).setMaxNumberOfSymbols(1);
 auto r=decodePolarities(g,w,h,opts);
 if(r.isValid()){
 result.format=ZXing::ToString(r.format()).c_str();auto b=r.bytes();for(auto c:b)result.value+=(char)c;
 }
 }catch(const std::exception &e){result.error=e.what();}catch(...){result.error="Native decode exception";}
 result.ms=millis()-t;return result;
}

static void commitNative(const NativeResult& r){
 nativeFrames++;nativeValue=r.value.startsWith("op=")?"":r.value;nativeFormat=r.format;nativeError=r.error;nativeMs=r.ms;
 if(r.value.startsWith("op=")){if(r.value!=lastNative){lastNative=r.value;if(identifyOperator(r.value))signalScan();else nativeError="Invalid or expired operator badge";}return;}
 if(r.value.length()){nativeMissing=0;if(r.value!=lastNative){lastNative=r.value;recordNative(r.format,r.value);if(decodeMode=="device")signalScan();}}
 else{if(!nativeMissing)nativeMissing=millis();if(millis()-nativeMissing>750)lastNative="";}
}
static void nativeDecode(camera_fb_t *f){commitNative(decodeFrame(f));}
static QueueHandle_t decodeJobs=nullptr,decodeDone=nullptr;
static NativeResult asyncResult;
static void decodeWorker(void*){camera_fb_t *f;for(;;){if(xQueueReceive(decodeJobs,&f,portMAX_DELAY)==pdTRUE){asyncResult=decodeFrame(f);esp_camera_fb_return(f);uint8_t done=1;xQueueSend(decodeDone,&done,portMAX_DELAY);}}}
static bool startWorker(){decodeJobs=xQueueCreate(1,sizeof(camera_fb_t*));decodeDone=xQueueCreate(1,sizeof(uint8_t));return decodeJobs&&decodeDone&&xTaskCreatePinnedToCore(decodeWorker,"native",12288,nullptr,1,nullptr,0)==pdPASS;}

static String nativeSelfTest(){
 String out="[";for(int j=0;j<4;j++){int i=j%2;bool inverse=j>=2;
 const char *matrix=i?dmMatrix:qrMatrix;int w=i?dmW:qrW,h=i?dmH:qrH;
 const char *expected=i?"LOT-TEST-02":"PCB-TEST-01";
 int width=w*5+40,height=h*5+40;
 uint8_t *buf=(uint8_t*)heap_caps_malloc(width*height,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
 bool pass=false;uint32_t t=millis();if(buf){memset(buf,255,width*height);for(int y=0;y<h*5;y++)for(int x=0;x<w*5;x++)buf[(y+20)*width+x+20]=matrix[(y/5)*w+x/5]=='1'?0:255;
 if(inverse)for(int k=0;k<width*height;k++)buf[k]=255-buf[k];try{auto r=decodePolarities(buf,width,height,ZXing::ReaderOptions().setFormats(i?ZXing::BarcodeFormat::DataMatrix:ZXing::BarcodeFormat::QRCode));auto b=r.bytes();pass=r.isValid()&&b.size()==strlen(expected)&&memcmp(b.data(),expected,b.size())==0;}catch(...){}free(buf);}
 if(j)out+=",";out+="{\"format\":"+jsString(i?"DataMatrix":"QRCode")+",\"inverse\":"+(inverse?"true":"false")+",\"pass\":"+(pass?"true":"false")+",\"ms\":"+String(millis()-t)+"}";
 }return out+"]";
}
