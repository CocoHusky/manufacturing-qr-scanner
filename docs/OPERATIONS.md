# Local operation

- Standby deinitializes the camera and turns off the LED. Browser mode sleeps after 15 s without frame requests; standalone mode scans continuously unless manually suspended. Wake reinitializes the camera and autofocus.
- Standalone decode runs in a FreeRTOS task; comparison runs synchronously on the requested frame. The native rate is a target, not guaranteed throughput.
- Archive: LittleFS NDJSON, maximum 192 KiB, no automatic overwrite. Export using Device archive. If mount fails, initialization requires explicit confirmation and may erase previous partition content. Never initialize a partition containing needed records.
- Native timestamps use NTP when synchronized, otherwise uptime and boot ID. Browser timestamps come from the client.
- Operator page: /sign. Enter name, part/lot, action and draw signature. The QR contains a record ID; signature image and fields reside in the same scanner archive. Self-asserted identity, not an authenticated or cryptographic signature.
- Capture UXGA temporarily changes camera resolution. Original JPEG and annotated PNG exports remain separate.
- Diagnostics report free heap, PSRAM, uptime and archive status. Physical current/temperature require external instruments.
- OTA blocked: current firmware exceeds 2 MiB, the 4 MiB device has one 3 MiB app slot. Use USB; dual-slot OTA needs a smaller image or larger flash.
- Trusted LAN only. No authenticated identities, TLS or audit-grade access control. Do not expose signature images or records publicly.

## Verification

Test idle/wake cycles, native frame count with browser disconnected, archive persistence across restart, QR sign-off lookup, UXGA JPEG dimensions and separate original/annotated exports. Measure current and temperature in each mode and perform the optical validation plan.
