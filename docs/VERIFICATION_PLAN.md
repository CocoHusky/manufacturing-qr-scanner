# Verification plan

This plan separates optical feasibility from production qualification. The current bench results are qualitative observations. No production acceptance limits or production validation have been completed.

## 1. Freeze the test setup

For every trial, record the firmware commit, decoder mode, camera resolution, scan region, focus state, LED level, exposure, ambient lighting, working distance, angle, marking material, printer or marking process, print DPI, measured symbol size, payload, and time to result. Verify printed dimensions against a measured scale.

Use the same physical targets in Browser and Embedded modes. Test center, edge and corner positions separately in both full-frame modes.

## 2. Map optical limits

Use QR and Data Matrix ECC 200 targets with known payloads and exact sample IDs. Include the previously ambiguous 128-character QR samples. Sweep symbol sizes and payloads separately; include quiet zones in measured footprint and record encoding and error-correction settings.

Test nominal sizes and ±5% dimensional variation. Include actual production PCB silkscreen or laser markings before selecting a minimum. Record the smallest reliably readable symbol and the largest symbol that fits the intended fixture field of view. Do not infer a universal maximum code size from one camera position.

### Separate printer and reader constraints

Print identical vector targets at actual size using available 300, 600 and 1200 DPI settings, or comparable verified marking processes. Measure the actual symbol and quiet zone with a calibrated scale; inspect module geometry, edge spread and contrast under magnification. Keep camera settings and working distance fixed for the printer comparison, then vary reader settings using the same target. Include 4 mm Data Matrix, 5 mm QR V1/M and 15 mm short QR as reference conditions. Separately sweep larger payloads through 15 mm, including 96 and 128 characters; do not extrapolate from short-code success. Repeat on intended PCB materials and electronic displays.

## 3. Engineering screen

For each selected size, payload, position, and decoder mode, make at least 20 independent presentations. Remove and replace the target between trials. Record correct reads, wrong reads, timeouts, duplicates, and time-to-result.

A screen result of 19/20 correct with no wrong reads is only an engineering screen. It is not evidence of production reliability or an acceptance criterion for release. Define production sampling and acceptance criteria from the product risk and required error rate before qualification.

## 4. Measure operator time

Time the same representative task using paper, manual computer entry, and the proposed two-scan fixture workflow. Include setup, scan retries, corrections, and record review. Report median and range across repeated runs, and keep actual test duration separate from operator entry time.

The two-scan case is not measurable with the current prototype because automatic fixture capture is not implemented. Treat any time comparison before that integration as a planning model, not a product result.

## 5. Verify the fixture record

Before a pilot, demonstrate that a single run record binds the unit ID, fixture ID, recipe/version, operator, calibration state, run ID, timestamps, raw measurements, pass/fail decision, and relevant image references. Verify duplicate prevention, interrupted runs, rework/retest handling, local persistence, export, and cloud-sync recovery.

## 6. Characterize the device

Measure input current and component temperature during idle, normal scanning, high LED output, maximum resolution, and sustained operation. Measure camera acquisition, transfer, decode, and end-to-end scan latency separately. Test focus, motion, angle, illumination variation, low contrast, damaged codes, duplicate suppression, and recovery after power or network loss.

## 7. Close production gaps

Use production markings and the intended fixture geometry. Set and document optical, thermal, security, data-retention, recovery, and throughput requirements. Complete verification against those requirements before describing the system as production-ready.