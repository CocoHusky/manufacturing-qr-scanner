# Local operation

- Browser and Embedded modes are available in the dashboard. Embedded uses a FreeRTOS worker; target FPS is a ceiling.
- Standby shuts down the camera and LED. Browser mode enters standby after 15 seconds without frame requests. Wake restores the configured resolution and autofocus.
- Capture UXGA for a full-resolution JPEG. Export the unmodified original separately from annotated PNG.
- Archive: LittleFS NDJSON, 192 KiB limit. Export before clearing; no automatic overwrite. Initialize only an empty or backed-up partition.
- Native timestamps use NTP when available; otherwise record uptime and boot ID. Browser timestamps come from the client.

## Test sign-in

Open `/sign` on a phone or computer. Enter user name, department and test, draw a signature, and accept the test declaration. The scanner stores a timestamped record and returns a compact record QR with a nominal 15 mm display footprint including quiet zone. Scan the QR to retrieve the record from the same device archive. The reference is not a self-contained copy of the record; export and retain the archive.

The archive must be initialized before creating a sign-in. Screen scaling affects the actual physical QR size. No password is required for this self-reported form.

## OTA

The 4 MB flash has two 1,792 KiB application slots and a 384 KiB filesystem. The first installation of this layout requires USB. Browser decoder assets are gzip-compressed to fit; HTTP clients decompress them normally.

Use the USB console command `OTAKEY` to retrieve the device-specific OTA password. Build with `pio run`, then use the bundled Arduino ESP32 `espota.py` script with the device address, firmware.bin and password. Keep credentials out of Git and command history. ArduinoOTA writes the inactive slot and changes the boot selection after successful transfer. Preserve the confirmed slot; upload only to the inactive slot. A candidate is confirmed after camera/Wi-Fi/worker checks and native decoder self-tests; otherwise request rollback after 60 seconds. Slot roles alternate instead of copying images. Bootloader rollback and interruption recovery require device verification. USB remains a last-resort recovery path; signed firmware is not configured.

## Limitations

Trusted LAN only: HTTP is unencrypted, operator state is station-wide, and the archive is not an authenticated audit ledger. Handwritten sign-off is not a cryptographic signature. Do not publish real operator records or credentials.

Native decoder self-test: GET `/selftest` with standalone scanning stopped. Synthetic matrices test the decoder, not lens performance. Optical repeatability, thermal performance, physical trigger and calibrated inspection remain hardware validation tasks.
