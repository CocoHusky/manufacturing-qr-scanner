# Hardware and firmware specification

| Item | Configuration |
|---|---|
| MCU | ESP32-D0WD-V3, 240 MHz maximum; AI Thinker-style ESP32-CAM |
| Memory | Detected 4 MB flash and approximately 4 MB PSRAM |
| Camera | OV5640 5 MP nominal; physical autofocus actuator unverified |
| Programmer | ESP32-CAM-MB / USB serial adapter |
| Camera clock | 20 MHz |
| Frame buffers | Two JPEG PSRAM buffers, latest-frame mode; fallback to one buffer |
| Resolution | QVGA, CIF, VGA, SVGA, XGA default, UXGA 1600×1200 |
| White LED | GPIO4, 20 kHz 10-bit PWM, defaults 10% scanning → 50% confirmation → 10%, 80 ms |
| Red LED | GPIO33 configured off, board wiring not verified |
| Wi-Fi | 2.4 GHz STA, fallback provisioning AP, modem sleep enabled |
| Storage | Preferences for configuration; browser session log only; SD not required |
| Toolchain | PlatformIO espressif32 6.13.0, Arduino ESP32 2.0.17; upload 460800 baud, monitor 115200 |

## Pin assignments

| Function | GPIO |
|---|---|
| Camera D0–D7 | 5,18,19,21,36,39,34,35 |
| XCLK / PCLK | 0 / 22 |
| VSYNC / HREF | 25 / 23 |
| SCCB SDA / SCL | 26 / 27 |
| Camera PWDN | 32 |
| White LED | 4 |
| Red LED (board dependent) | 33 |
| Proposed switch | 13, momentary to GND, internal pull-up |
| Proposed buzzer | 14, transistor-driven 3.3 V-compatible module |

IO0 is a boot strap and active camera clock output. Pressing the onboard IO0 button during camera operation corrupted frames; do not use as runtime trigger. GPIO13 legacy routine exists but is not called in loop. Trigger and buzzer not implemented. SD use could conflict with these pins. Do not directly drive a high-current buzzer.

## Recognition pipeline

ZXing-WASM 3.1.7 is hosted by the ESP32 and executed in the browser. All supported readable formats are enabled, one symbol per frame. QR and Data Matrix physically tested; linear codes not qualified. Decoder effort, region, rate and resolution adjustable. Center ROI reduces browser processing, not necessarily sensor power. Frame rates are target ceilings. Camera delivered-frame timestamps, display FPS and decode FPS are reported but coupled sequentially; camera FPS is not independently measured maximum sensor FPS. HTTP residual is estimated as request latency minus frame acquisition. Same-code suppression may still admit duplicates after missed detections. Browser logs have no persistent storage.

## Thermal / optical limitations

Earlier QVGA transfer sample averaged 161 ms for five frames; ~6.2 requests/s before browser decode. Later buffering improvements are not benchmarked under comparable conditions; no 30/60 FPS claim. Focus firmware initialized and reported success, but actual lens movement/VCM supply not verified. Verify exact module before hardware modifications. Prefer a fixed macro focus and stable lighting.

The board was reported very hot during continuous operation. PWM reduces illumination demand, but input current and temperature are unmeasured; browser pause does not guarantee sensor standby. Stop operation if excessively hot. Evaluate LED, regulator, ESP32 and camera individually. Do not operate unattended.

## Security

Prototype HTTP is unauthenticated and unencrypted; ordinary Preferences stores network settings. Repository removes machine-specific upload path and replaces the prototype's fixed AP password with a random persisted per-unit setup password printed in serial monitor. Repository change requires hardware retest, and does not change the already-installed firmware until flashed. Do not expose server to internet. Public/production release requires credential protection, authenticated provisioning, signed updates, flash encryption, secure boot, threat model and third-party notice review.
