#pragma once
#include <ReadBarcode.h>
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
 int xx=x+i,yy=y+j;if(xx<a->x||yy<a->y||xx>=a->x+a->w||yy>=a->y+a->h)continue;
 auto *p=data+3*(j*w+i);a->gray[(yy-a->y)*a->w+xx-a->x]=(77*p[0]+150*p[1]+29*p[2])>>8;
 }return true;
}
struct NativeResult{String value,format,error;uint32_t ms=0;};
static NativeResult decodeFrame(camera_fb_t *f){NativeResult result;
 uint32_t t=millis();
 // Center crop at most 640x480 to bound memory, retaining native pixel pitch.
 int w=min((int)f->width,640),h=min((int)f->height,480);
 if(nativeCapacity<(size_t)(w*h)){free(nativeGray);nativeGray=(uint8_t*)heap_caps_malloc(w*h,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);nativeCapacity=nativeGray?w*h:0;}uint8_t *g=nativeGray;
 if(!g){result.error="PSRAM allocation failed";result.ms=millis()-t;return result;}
 GrayJPEG a{f,g,((int)f->width-w)/2,((int)f->height-h)/2,w,h};memset(g,255,w*h);
 if(esp_jpg_decode(f->len,JPG_SCALE_NONE,jpegRead,jpegWrite,&a)!=ESP_OK)result.error="JPEG conversion failed";
 else try{
 auto opts=ZXing::ReaderOptions().setFormats(ZXing::BarcodeFormat::QRCode|ZXing::BarcodeFormat::DataMatrix).setTryHarder(true).setTryRotate(true).setTryDownscale(false).setMaxNumberOfSymbols(1);
 auto r=ZXing::ReadBarcode(ZXing::ImageView(g,w,h,ZXing::ImageFormat::Lum),opts);
 if(r.isValid()){
 result.format=ZXing::ToString(r.format()).c_str();auto b=r.bytes();for(auto c:b)result.value+=(char)c;
 }
 }catch(const std::exception &e){result.error=e.what();}catch(...){result.error="Native decode exception";}
 result.ms=millis()-t;return result;
}

static void commitNative(const NativeResult& r){
 nativeFrames++;nativeValue=r.value;nativeFormat=r.format;nativeError=r.error;nativeMs=r.ms;
 if(r.value.length()){nativeMissing=0;if(r.value!=lastNative){lastNative=r.value;recordNative(r.format,r.value);if(decodeMode=="device")signalScan();}}
 else{if(!nativeMissing)nativeMissing=millis();if(millis()-nativeMissing>750)lastNative="";}
}
static void nativeDecode(camera_fb_t *f){commitNative(decodeFrame(f));}
static QueueHandle_t decodeJobs=nullptr,decodeDone=nullptr;
static NativeResult asyncResult;
static void decodeWorker(void*){camera_fb_t *f;for(;;){if(xQueueReceive(decodeJobs,&f,portMAX_DELAY)==pdTRUE){asyncResult=decodeFrame(f);esp_camera_fb_return(f);uint8_t done=1;xQueueSend(decodeDone,&done,portMAX_DELAY);}}}
static bool startWorker(){decodeJobs=xQueueCreate(1,sizeof(camera_fb_t*));decodeDone=xQueueCreate(1,sizeof(uint8_t));return decodeJobs&&decodeDone&&xTaskCreatePinnedToCore(decodeWorker,"native",12288,nullptr,1,nullptr,0)==pdPASS;}
