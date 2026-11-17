# Local operation

- Browser and Embedded modes are available in the dashboard. Embedded uses a FreeRTOS worker; target FPS is a ceiling.
- Standby shuts down the camera and LED. Browser mode enters standby after 15 seconds without frame requests. Wake restores the configured resolution and autofocus.
- Capture the current preview or last decoded Embedded image; annotate and export original or marked images. Saved inspection images are browser-local.
- Archive: LittleFS NDJSON, 192 KiB limit. Export before clearing; no automatic overwrite. Initialize only an empty or backed-up partition.
- Native timestamps use NTP when available; otherwise record uptime and boot ID. Browser timestamps come from the client.

## Operator sign-on and workflows

Open the scanner and enter name, department and test. Scan the four-digit Code 128 header on the printed instruction sheet to select and start its recipe, or start it from Workflows. Then present each required label individually. Remove a label from the camera view before rescanning it.

Correct scan: one white LED flash. Invalid workflow scan: one vibration and red status LED; correct the label and continue. Confirmation timeout: two vibrations and a failed run. Completed records: a slow white LED fade and final flash. A completed FAIL measurement is a valid traceability record; the dashboard and JSON export retain its result, failure reason and data reference. Scan the same device for final confirmation regardless of measurement disposition.

The benchtop and aging confirmation deadlines are 60 seconds. Aging timing begins at the first chamber scan; rescan the same chamber after at least five seconds. Network time is synchronized with NTP; elapsed time uses a monotonic clock. Export records before clearing the journal.

[Workflow diagrams](workflows/README.md) · [Test sheets](test-sheets/manufacturing-pipeline.pdf)

## OTA

The 4 MB flash has two 1,792 KiB application slots and a 384 KiB filesystem. The first installation of this layout requires USB. Browser decoder assets are gzip-compressed to fit; HTTP clients decompress them normally.

Use the USB console command `OTAKEY` to retrieve the device-specific OTA password. Build with `pio run`, then use the bundled Arduino ESP32 `espota.py` script with the device address, firmware.bin and password. Keep credentials out of Git and command history. ArduinoOTA writes the inactive slot and changes the boot selection after successful transfer. Preserve the confirmed slot; upload only to the inactive slot. A candidate is confirmed after camera/Wi-Fi/worker checks and native decoder self-tests; otherwise request rollback after 60 seconds. Slot roles alternate instead of copying images. Bootloader rollback and interruption recovery require device verification. USB remains a last-resort recovery path; signed firmware is not configured.

## Limitations

Trusted LAN only: HTTP is unencrypted, operator state is station-wide, and the archive is not an authenticated audit ledger. Sign-on is not an authenticated identity or regulated signature. Do not publish real operator records or credentials.

Native decoder self-test: GET `/selftest` with standalone scanning stopped. Synthetic matrices test the decoder, not lens performance. Optical repeatability, thermal performance, physical trigger and calibrated inspection remain hardware validation tasks.
