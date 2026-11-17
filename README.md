# Manufacturing Test Scanner

A camera-based reader for QR and Data Matrix labels on components and test articles. It captures an identifier, parses supported label data, and keeps a local scan record so operators do not have to retype identifiers.

<p align="center">
<img src="docs/media/prototype-front.jpg" alt="Manufacturing scanner prototype" width="300">
</p>

[Watch the compact-marking scan demo](docs/media/demo-compact-marking.mp4) · [See the prototype](docs/media/prototype-side.jpg)

## What works today

- Browser decoding: QR, Data Matrix, and linear barcodes.
- ESP32 decoding: QR and Data Matrix.
- Browser-side parsing for ISO/IEC 15434 Format 06, JSON, key/value labels, and configured custom formats.
- Local scan history, searchable records, CSV/JSON export, and a bounded device archive.
- Full-resolution still capture with simple image annotation.

The browser preview shows the full camera frame. Browser decoding defaults to the full frame and also supports a smaller center region. Native ESP32 decoding currently checks only a fixed center crop.

![Browser and native decoder scan areas](docs/figures/scan-area.svg)

## What the prototype does not do

It does not start a test, control a fixture, collect instrument measurements, decide pass/fail, or upload records to a cloud or MES. Those are the next product steps. The sign-in page and image annotation are prototype utilities; sign-in is self-reported and is not authenticated or a regulated electronic signature.

## Bench-read observations

These are qualitative reads from printed targets, not validated operating limits. The tests did not consistently record independent repeats, print settings, camera distance, lighting, or latency. Treat the figures as promising examples, not guaranteed limits.

| Marking | Bench observation | Evidence status |
|---|---|---|
| QR | 5 mm square, 16 alphanumeric characters | Reported read; repeatability unknown |
| Data Matrix ECC 200 | 4 mm square, 16 characters | Reported read; repeatability unknown |
| Data Matrix ECC 200 | 8 mm square, 96 characters | Reported read; strongest dense result |
| QR | 10–12 mm, up to 96 characters | Candidate inferred from the test grid; not a confirmed maximum |
| QR, compact PCB label | 15 mm square, 7 characters | Reported read; does not establish a general maximum |

Reported footprints include the code quiet zone. Character counts describe the tested ASCII samples. A later 128-character QR failure was reported, but its exact size and encoding were not recorded. The maximum dense QR payload is therefore unknown. Physical read limits depend on symbol type, encoding, print quality, focus, working distance, lighting, and framing. See the [full test report](docs/TEST_REPORT.md) and [per-target evidence](docs/MARKED_RESULTS.md).

The sensor can capture up to 1600 × 1200 pixels. That is image resolution, not a promise that a symbol of a given size will decode. The minimum pixels per module, working distance, and reliable scan envelope have not been measured.

## Proposed test-bench workflow

1. Scan the unit serial or component label.
2. Scan the fixture or test-recipe identifier.
3. The fixture validates the pairing and runs the selected procedure.
4. The fixture records measurements, result, calibration state, image references, and a unique run ID.
5. Save locally first, then sync the record to the manufacturing system when available.

This flow is a product direction, not an implemented capability. The fixture should own test execution and measurements; the scanner should provide reliable unit identification and operator feedback.

## Illustrative time model

For a seven-item record (unit, fixture, test, three measurements, and outcome), planning assumptions are 30–60 seconds to write on paper, 20–40 seconds to type, and 4–8 seconds for two scans plus review after automatic fixture capture is implemented.

| Entry method | Assumed operator entry time per unit | 100 units |
|---|---:|---:|
| Paper record | 30–60 s | 50–100 min |
| Manual computer entry | 20–40 s | 33–67 min |
| Two scans plus automatic capture | 4–8 s | 7–13 min |

This is a modeled comparison, not a measured speed result. It excludes test duration, fixture setup, exceptions, failed scans, and retries. The current prototype has not been timed end to end and does not perform automatic fixture capture. A timed study is required before claiming a throughput improvement.

## Hardware and limits

- ESP32-CAM with OV5640 camera; maximum capture size 1600 × 1200.
- Browser decoding runs on the connected computer or phone.
- Native decoding uses a 640 × 480 center crop to limit memory use. At 1600 × 1200 this is 40% of the image width and height, or 16% of its area. A code outside that center crop can remain visible in the preview but will not be read by the native decoder.
- Device archive capacity is 192 KiB. Export records before the archive fills.
- Continuous operation has caused excessive heating; current and component temperatures are not yet characterized.
- Autofocus actuator movement, scan repeatability, false-read rate, and end-to-end latency are unverified.
- The local web interface uses unencrypted HTTP. Use a trusted local network and synthetic data for demos.

See [hardware and firmware](docs/HARDWARE_AND_FIRMWARE.md), [operations](docs/OPERATIONS.md), and the [verification plan](docs/VERIFICATION_PLAN.md).

## Build

1. Install PlatformIO Core.
2. Connect the USB serial programmer and run pio run.
3. Upload with pio run -t upload --upload-port PORT.
4. Configure Wi-Fi at 192.168.4.1, then open the scanner's assigned address in a browser.

## Project status

Engineering prototype for a manufacturing test workflow. It is not qualified or approved for production use. A project-level license has not been selected; third-party notices are listed in [THIRD_PARTY.md](docs/THIRD_PARTY.md).