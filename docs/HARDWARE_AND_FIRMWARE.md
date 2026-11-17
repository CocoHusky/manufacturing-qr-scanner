# Hardware and firmware specification

| Item | Configuration |
|---|---|
| MCU | ESP32-D0WD-V3, 240 MHz maximum; AI Thinker-style ESP32-CAM |
| Memory | Detected 4 MB flash and approximately 4 MB PSRAM |
| Camera | OV5640 5 MP nominal; autofocus actuator movement is unverified |
| Programmer | ESP32-CAM-MB / USB serial adapter |
| Resolution | QVGA, CIF, VGA, SVGA, XGA default, UXGA 1600 × 1200 |
| Storage | Preferences, 50 recent native records, 192 KiB LittleFS archive |
| Toolchain | PlatformIO espressif32 6.13.0, Arduino ESP32 2.0.17 |

## Recognition pipeline

| Mode | Decoder | Scan area |
|---|---|---|
| Browser | ZXing-WASM 3.1.7; QR, Data Matrix, linear formats | Full frame by default; adjustable center region |
| Embedded | ZXing-C++ 2.2.1; QR and Data Matrix | Fixed center crop, at most 640 × 480 pixels |

The preview displays the full camera frame. Browser decoding defaults to the full frame. Embedded decoding does not: firmware selects the center 640 × 480 pixels at native resolution to bound grayscale-buffer memory. It does not downsample the whole frame.

At 1600 × 1200, the native decoder examines 40% of the frame width and height (16% of its area). At 1024 × 768, it examines 62.5% of the width and height (about 39% of the area). A code visible outside this crop will not be read in Embedded mode. The crop location and size are fixed in firmware.

Native target rate is 1–10 FPS; actual rate depends on acquisition and decode time. A measured example took 1,108 ms per frame at XGA. Physical native read limits remain unverified.

Browser controls include resolution, scan region, requested frame rate, and decoder effort. CPU speed (80/160/240 MHz), Wi-Fi power saving, and JPEG quality (8–30; higher values compress more) are configurable. The frame-rate limit controls requests, not necessarily sensor capture rate. Preview, transfer, and browser decoding share processing time.

## Camera and illumination

- OV5640 camera clock: 20 MHz; JPEG capture with two PSRAM frame buffers when available.
- White LED: GPIO4, 20 kHz PWM. Defaults to 10% scan brightness and 50% confirmation brightness for 80 ms.
- Red LED: GPIO33 is configured off; board wiring is unverified.
- Autofocus firmware initialization is observed; physical actuator movement and power are unverified.
- QVGA transfer averaged 161 ms over five frames in an earlier measurement. Later buffering changes have not been benchmarked.

## Board constraints

Camera signals use GPIOs 0, 5, 18, 19, 21–27, 32, 34–39. GPIO0 is a boot strap and camera clock output; the onboard GPIO0 button corrupted camera frames during operation and is not a runtime trigger. GPIO13 has a legacy button routine but is not wired as an implemented production trigger. A physical trigger and buzzer are not implemented. SD wiring may conflict with camera pins.

## Operational limits

Continuous operation caused excessive heating. Input current and component temperatures have not been measured. Do not operate unattended; stop if the board becomes excessively hot. Browser mode enters standby after 15 seconds without frame requests.

The device archive is bounded at 192 KiB and does not automatically overwrite old records. Export before it fills.

## Network and security

The local interface uses unencrypted HTTP. Test sign-in is self-reported; it does not authenticate identity or provide a protected audit record. Wi-Fi credentials are stored in Preferences, and the setup AP password is printed to the serial monitor. Use a trusted local network and synthetic data for demonstrations.

Before production, define and verify authenticated provisioning, credential protection, signed updates, secure boot, flash encryption, thermal limits, camera focus, and a recovery process.