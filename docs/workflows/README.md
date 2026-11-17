# Workflow verification examples

The ESP32 checks label sequence and identity in both decoder modes. Select **Workflows**, start a run, and scan the labels. Rejected scans retain the current step and trigger a warning. Cancel an expired run before restarting. The device records accepted and rejected events in its local journal; export runs as JSON before clearing records.

[Print the four-page mixed-format test PDF](../test-sheets/manufacturing-pipeline.pdf) at 100% scale. Linear codes are capability checks only.

All examples use semicolon-separated fields: `WF` (recipe ID), `STEP` (label entry step, starting at 1), `TYPE`, and `SN` or `ID`. The same device label is reused at final confirmation. Use the printable labels externally; the dashboard displays process progress rather than QR examples.

## ASM-01 — component assembly

[Printable QR labels](ASM-01.html)

```mermaid
flowchart TD
 A[1: Scan device traveler] --> B[2: Scan required component]
 B --> C{Required part, lot and unique serial?}
 C -->|No| W[Warn; retain current step]
 W --> B
 C -->|Yes| D{Six distinct required parts?}
 D -->|No| B
 D -->|Yes| E[3: Rescan original device]
 E --> F{Same serial?}
 F -->|No| X[Warn; rescan correct device]
 X --> E
 F -->|Yes| G[Complete assembly record]
```

Required part IDs: PCB-001, BAT-001, SNS-001, HSG-001, ANT-001, SEL-001. Scan them in any order within step 2. An early device confirmation or duplicate part is rejected.

## TEST-01 — benchtop functional test

[Printable QR labels](TEST-01.html)

```mermaid
flowchart TD
 A[1: Scan device] --> B[2: Scan bench]
 B --> C[Run experiment externally]
 C --> D[3: Scan result and data reference]
 D --> E{Device and bench match?}
 E -->|No| W[Warn; scan matching result]
 W --> D
 E -->|Yes| F[4: Rescan device within 60 seconds]
 F --> G{Identity and deadline valid?}
 G -->|Wrong identity| F
 G -->|Timeout| X[Warn; cancel or reconcile run]
 G -->|Yes| H[Complete traceability record]
```

The result label links the equipment, device and run ID. The scanner does not execute the experiment, fetch cloud data or validate measurement accuracy.

## THERM-01 — fixture-mounted thermal testing

[Printable QR labels](THERM-01.html)

```mermaid
flowchart TD
 A[1: Fixture] --> B[2: Device]
 B --> C[3: Test bag]
 C --> D[4: Power supply]
 D --> E[5: Temperature probe]
 E --> F[Run external thermal experiment]
 F --> G[6: Scan result]
 G --> H[7: Confirm original device]
 H --> I[Complete]
 J[Incorrect label or order] --> K[Warn; retain expected step]
```

Scan the fixture and each accessory in order before presenting a matching result. Physical fixture interlocks and automatic instrument control are not implemented.

## AGE-01 — environmental aging

[Printable QR labels](AGE-01.html)

```mermaid
flowchart TD
 A[1: Scan chamber] --> B[2: Scan loaded device]
 B --> C[Track elapsed time]
 C --> D[3: Scan matching door-open event]
 D --> E[4: Rescan device within 60 seconds]
 E --> F{Same device and dwell at least 24 hours?}
 F -->|Yes| G[Complete dwell record]
 F -->|No| W[Warn; correct identity or reconcile]
```

One device per active run. The 24-hour minimum is an example, not a validated product requirement. Timing uses the ESP32 monotonic clock; uninterrupted power is required. A reboot loses active workflow state but leaves persisted journal events. Door opening is manually represented by a label, not a sensor.

## Validation status

Host regression tests cover all four workflows, step skipping, duplicate parts, mismatched identity, timeout, premature removal and storage failure. Physical scanning, motor response, power interruption recovery and production reliability require hardware qualification.
