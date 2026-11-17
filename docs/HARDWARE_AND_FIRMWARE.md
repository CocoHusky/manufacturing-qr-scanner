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
| Storage | Preferences for configuration; browser session log and 50 native records in RAM |
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

| Mode | Decoder |
|---|---|
| Browser | ZXing-WASM 3.1.7; QR, Data Matrix and linear formats |
| Standalone | ZXing-C++ 2.2.1; QR and Data Matrix |
| Compare | Both readers use the same JPEG |

Native processing converts JPEG to grayscale in PSRAM, cropping to the central 640 × 480 pixels maximum. Minimum interval between standalone scan starts: 500 ms. Native processing includes JPEG conversion; measured example: 1,108 ms/frame at XGA. No native successful read independently verified yet.

Browser controls: resolution, ROI, 1–30 FPS target or maximum, decoder effort. Hardware controls: CPU 80/160/240 MHz, Wi-Fi sleep and JPEG quality 8–30 (larger number means stronger compression). FPS limits requests, not sensor activity. Browser decode, preview and delivered-frame rates are coupled; HTTP residual excludes measured acquisition time. Duplicate suppression can admit repeats after missed detections. Both logs are volatile.

## Thermal / optical limitations

QVGA transfer averaged 161 ms over five frames (~6.2 requests/s before decoding). Later buffering changes are unbenchmarked. Autofocus firmware initializes; actuator movement and supply remain unverified.

Continuous operation caused excessive heating. Current and component temperatures remain unmeasured. Browser pause does not suspend the sensor. Stop if excessively hot; do not operate unattended.

## Security

HTTP is unauthenticated and unencrypted. Wi-Fi credentials use Preferences; the random setup AP password is printed in serial monitor. Use a trusted LAN only. Before production, review authenticated provisioning, credential storage, signed updates, secure boot and flash encryption.
