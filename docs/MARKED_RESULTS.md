# Per-target results — Rev 03

The original test PDFs are the source for symbol IDs, encoding, footprint and payload lengths. Physical results come from the operator's narration and screenshot. Do not interpret a reported cutoff as proof of every individual read.

**Legend:** P = reported read; I = inside reported cutoff (inferred); F = reported failed read; C = conditional, above cutoff, conflicting or FOV limited; ? = unknown. The later 128-character failure had no exact sample ID; no 128-character success is confirmed.

## Original mixed test

- P:05, P:06, P:08, P:10 shown in scan log (P).
- P:04 shown not recognized in current frame (F for that attempt only).
- Other QR ladder cells, dense payloads, batch and linear barcode targets: outcomes not recorded per sample (?).
- Original fixed 44×44 Data Matrix was superseded by compact ECC200 test; original PCB-002 was reported too large for the window.

## QR density grid

Columns: 4 / 16 / 32 / 64 / 96 / 128 ASCII characters. Rows: square footprint including quiet zone (mm). Cell status refers to original ID prefix M, L or B and sequential column index. L-prefixed IDs anchor the first narrated group; V6 and V8 locate the other groups. 15 mm group attribution remains uncertain.

| Size | Alpha M (M) | Alpha L (L) | Byte M (B) |
|---|---|---|---|
| 4 | ? ? ? ? ? ? | ? ? ? ? ? ? | ? ? ? ? ? ? |
| 5 | I I C C C C | P P C C C C | I C C C C C |
| 6 | I I I C C C | P P C C C C | I I C C C C |
| 8 | I I I I C C | I I I I I F | I I I I C C |
| 10 | I I I I I C | I I I I I C | I I I I I C |
| 12 | I I I I I C | I I I I I C | I I I I I C |
| 15 | ? ? ? ? ? ? | C C C C C C | ? ? ? ? ? ? |

At 8 mm L086 V5 explicitly failed. C in other rows may mean above a reported cutoff, not independently observed failure. At 10–12 mm the 96-character QR values remain candidates; 128 is unresolved per cell after the reported failure.

## Data Matrix density grid

Columns: 7 / 16 / 32 / 64 / 96 characters.

| Size | D-prefix cell status |
|---|---|
| 4 | I P C C C |
| 5 | I I P C C |
| 6 | I I P C C |
| 8 | I I I I P |
| 10 | C C C C C |
| 12 | C C C C C |
| 15 | ? ? ? ? ? |

At 10 mm, 96 characters reportedly read, but 10 mm was also described as too large for the camera window; keep conditional. At 12 mm FOV limited. Compact PCB-002 Data Matrix at 5/8/10 mm P, 15 mm C (FOV). Compact QR-M at 5/8/10/15 mm P. Printed 10 mm PCB mockup at 6/8/10 mm P.

## Boundary test

Final boundary sheet has no reported results. Candidate ranges are 0.5–5.25 mm short QR/ECC200, 80–128 character high-density QR, and ±5% nominal size checks. Mark all pending, not failed. Boundary and annotated report PDFs pending inclusion.
