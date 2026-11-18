# Optical characterization

The initial goal is a practical marking envelope for prototype assembly and test: QR at 5–15 mm and compact Data Matrix down to 4 mm for identifier marking on PCBs.

| Format | Footprint including quiet zone | Reported result |
| --- | --- | --- |
| Data Matrix | 4 mm | 16 characters |
| Data Matrix | 5–6 mm | 32 characters |
| Data Matrix | 8 mm | 96 characters |
| Data Matrix | 10 mm | Conditional 96-character report; field-of-view conflict |
| QR V1/M | 5 mm | P:05 (4 characters) size-ladder read; new successful recognition screenshot |
| QR | 5 mm | 16 characters (alphanumeric L reported; M inferred) |
| QR | 8 mm | Up to 96 characters reported for alphanumeric L; inferred cutoff |
| QR | 10–12 mm | 96-character candidates; 128-character failure not isolated |
| QR-M | 15 mm | PCB-002 (7 characters) reported read in separate comparison; maximum unknown |

![Payload capacity](../figures/capacity.svg)

These are qualitative operator-reported observations, not guaranteed limits. Exact encoding, error correction, quiet zone and print quality affect payload density. A compact identity code can reference a richer external record rather than embedding every field.

The camera and lens were modified for close-range macro imaging to make compact printed symbols occupy more of the sensor frame. This is a low-cost optical approach rather than digital enlargement alone. Working distance, optical magnification and depth of field remain uncharacterized. The test sheets were printed using a Canon PIXMA MG3620 inkjet at the high-quality computer print setting, rated up to 600 × 600 DPI black and 4800 × 1200 DPI color ([Canon PIXMA MG3620 specifications](https://www.usa.canon.com/support/p/pixma-mg3620)). Print fidelity is a potential constraint; 4 mm Data Matrix with 16 ASCII characters is the smallest demonstrated compact result in this setup. This does not establish a camera-only resolution limit. Contrast, print resolution, focus distance, full-frame field of view, exposure and motion have not been separated. Direct marking on manufactured PCBs remains unqualified.

[Plotted observations (CSV)](optical-results.csv). The chart shows the four density series; short-code observations remain in the table and CSV. Coincident markers are offset horizontally for readability only; lines use the exact coordinates. Evidence status is retained in the CSV.

## Printing and module resolution

The latest 5 mm QR V1/M size-ladder screenshot shows a recognized QR with visible edge irregularity; it is not a calibrated printer-resolution measurement. Version 1 has 21 active modules per side, plus four quiet-zone modules per edge. Assuming nominal footprint includes quiet zone, module pitch is 5 mm / 29 = 172.4 µm.

| Nominal printer DPI | Dot pitch | Dots per 5 mm QR module |
| ---: | ---: | ---: |
| 300 | 84.7 µm | 2.04 |
| 600 | 42.3 µm | 4.07 |
| 1200 | 21.2 µm | 8.15 |

Four dots per module would require approximately 589 DPI; 600 DPI therefore matches the nominal black printer rating, but is not a measured effective resolution or validated minimum. The 4 mm ECC200 example (18 active modules plus two quiet-zone modules, if that symbol size applies) has 200 µm pitch, or 4.72 dots per module at 600 DPI. Ink spread, camera blur and compression cannot be separated from the screenshots alone. Measure a calibrated printed target to determine effective resolution; the color DPI rating does not establish monochrome detail.

## Theoretical print-limited envelope

At effective resolution R DPI and four printer dots per module, module pitch p = 101.6 / R mm. Footprints include quiet zones: (N + 2)p for square ECC200 Data Matrix and (25 + 4V)p for QR version V.

| Effective DPI (both axes) | Minimum DM (1 byte) | DM (16 bytes) | QR V1/M (14 bytes) | QR V2/M (26 bytes) |
| ---: | ---: | ---: | ---: | ---: |
| 600 | 2.032 mm | 3.387 mm | 4.911 mm | 5.588 mm |
| 1200 | 1.016 mm | 1.693 mm | 2.455 mm | 2.794 mm |
| 2400 | 0.508 mm | 0.847 mm | 1.228 mm | 1.397 mm |
| 4800 | 0.254 mm | 0.423 mm | 0.614 mm | 0.699 mm |

For a fixed 15 × 15 mm footprint (225 mm²), choose the largest standard symbol that fits. QR uses byte mode and correction M; Data Matrix uses conservative Base256 capacities.

| Effective DPI | QR version / bytes | QR B/mm² | DM modules / bytes | DM B/mm² |
| ---: | --- | ---: | --- | ---: |
| 600 | V15 / 412 | 1.83 | 80 × 80 / 453 | 2.01 |
| 1200 | V38 / 2,099 | 9.33 | 144 × 144 / 1,555 | 6.91 |
| 2400 | V40 / 2,331 | 10.36 | 144 × 144 / 1,555 | 6.91 |
| 4800 | V40 / 2,331 | 10.36 | 144 × 144 / 1,555 | 6.91 |

Capacity references: [QR byte capacity](https://qexqr.com/qr-code-versions) and [Data Matrix ECC200 symbol attributes](https://www.onbarcode.com/kb/barcode-faq/data-matrix/).

Reported densities: 4 mm DM / 16 ASCII characters = 1.00 B/mm²; 8 mm DM / 96 = 1.50 B/mm²; 5 mm QR / 16 = 0.64 B/mm²; 15 mm QR / 7 = 0.031 B/mm². A theoretical 16-byte DM at 1200 DPI yields 5.58 B/mm², or 22.32 B/mm² at 2400 DPI.

These are printability and encoding estimates, not scanner performance guarantees. The Canon's anisotropic color rating is not equivalent to 4800 DPI in both axes. Camera sampling, focus, contrast and field of view must also support the marking. Test higher-fidelity targets, independently varying size and payload, and measure successful reads within a specified time. The 15 mm short-code read is a demonstrated footprint, not a proven maximum size or capacity.

## Combined camera and printer estimate

### Image-based spatial sampling

![Recognized 5 mm QR target](../media/screenshots/scanner-detected.png)

The recognized QR screenshot is 1600 × 1104 pixels. The detected green outline spans approximately 274 × 269 screenshot pixels; the displayed camera image spans approximately 1173 pixels horizontally. Assuming a full VGA (640 × 480) camera frame, the following estimates apply:

| Parameter | Value |
| --- | ---: |
| Nominal QR footprint including quiet zone | 5 × 5 mm |
| QR version / active modules | V1 / 21 × 21 |
| Total modules per side with quiet zone | 29 |
| Nominal module pitch | 172.4 µm |
| Native pixels across active symbol | 274 × 640 / 1173 = 149.5 |
| Native pixels per module | 7.12 |
| Object-space sampling | 24.2 µm/pixel (rounded to 25 for design) |

The separate [original macro PCB image](../inspection/macro-original.jpg) is 640 × 480 and demonstrates visible fine PCB detail, but does not establish a physical resolution without a calibrated feature dimension. The native resolution of the QR screenshot is assumed, and the green outline is an approximate active-symbol boundary. Sampling is not equivalent to optical resolving power.

### Combined design constraint

Use four camera pixels per module plus a 25% engineering margin, or five pixels per module. With the rounded sampling estimate, the camera design pitch is 125 µm. The margin is a design assumption, not a measured tolerance or confidence interval.

Combined module pitch (mm) = max(101.6 / effective printer DPI, 5 × 0.025).

| Effective printer DPI (both axes) | Printer pitch | Camera pitch | Combined pitch |
| ---: | ---: | ---: | ---: |
| 600 | 169.3 µm | 125 µm | 169.3 µm |
| 1200 | 84.7 µm | 125 µm | 125 µm |
| 2400 | 42.3 µm | 125 µm | 125 µm |
| 4800 | 21.2 µm | 125 µm | 125 µm |

The crossover is approximately 813 DPI. Further increases in print resolution alone would not improve this modeled sampling-limited minimum.

| Format | Binary payload | 600 DPI | 1200 DPI or higher |
| --- | ---: | ---: | ---: |
| Data Matrix 10 × 10 | 1 byte | 2.03 mm | 1.50 mm |
| Data Matrix 14 × 14 | 6 bytes | 2.71 mm | 2.00 mm |
| Data Matrix 18 × 18 | 16 bytes | 3.39 mm | 2.50 mm |
| Data Matrix 20 × 20 | 20 bytes | 3.73 mm | 2.75 mm |
| QR V1/M | 14 bytes | 4.91 mm | 3.63 mm |
| QR V2/M | 26 bytes | 5.59 mm | 4.13 mm |

The 16-byte Data Matrix changes from 16 mm² demonstrated footprint to 6.25 mm² modeled footprint: 60.9% less area and 2.56 B/mm² versus 1.00 B/mm².

### Capacity at a 15 mm footprint

At 25 µm/pixel, the estimated VGA field of view is 16 × 12 mm. A 15 mm square cannot fit vertically at that sampling. At an increased working distance, reserving 10% of the frame height for clearance leaves 432 of 480 pixels for the entire symbol including quiet zones. Sampling becomes 15/432 = 34.72 µm/pixel, and a five-pixel design target requires 173.61 µm per module. At 600 DPI or higher the camera sampling constraint dominates. At most 86 complete modules including quiet zones fit within this budget.

| Format | Largest fitting symbol | Binary payload | Density |
| --- | --- | ---: | ---: |
| QR, correction M | V15 (85 modules including quiet zone) | 412 bytes | 1.83 B/mm² |
| Data Matrix ECC200 | 80 × 80 (82 including quiet zone) | 453 bytes | 2.01 B/mm² |

These are geometric and encoding estimates only. The 15 mm QR demonstration carried seven ASCII characters; a dense symbol at this footprint has not been validated. Increased distance must retain focus and contrast. Test calibrated targets at fixed working distances, verify native frame resolution, and measure decode success and latency across repeated trials before establishing a production marking envelope.

## Recommended marking choices

| Location | Initial design | Reason |
| --- | --- | --- |
| Compact PCB or small component | 4–5 mm Data Matrix; short unique identifier | Smaller footprint; 4 mm / 16 characters reported read |
| Traveler, packaging or equipment | 5–15 mm QR, sized for actual payload | Larger labels provide more module area |
| Instrument result display | QR with unique run reference; optional summary fields | Link to authoritative measurement files without encoding entire datasets |

Use a nominal 600 DPI or better print process as a starting point, actual-size output, intact quiet zones and high contrast. At 4 mm QR V1, four dots per module would require about 737 DPI; denser codes may need more. A 1200 DPI process is a useful comparison, not a proven solution. Small ID-only labels require an external lookup for richer metadata; the current workflow parser expects its defined fields, so this lookup design is proposed rather than implemented.

[Full module-pitch calculations and marking recommendations](../TEST_REPORT.md#print-resolution-and-practical-marking-design).

## Evidence and reproduction

| Compact marking | QR density | Size ladder |
| :---: | :---: | :---: |
| [![Compact marking](../media/demo-compact-marking.gif)](../media/demo-compact-marking.mp4) | [![Density scanning](../media/demo-density-scanning.gif)](../media/demo-density-scanning.mp4) | [![Size ladder](../media/demo-qr-ladder.gif)](../media/demo-qr-ladder.mp4) |

Compact markings, information density and QR size-ladder testing.

- [Compact marking demonstration](../media/demo-compact-marking.mp4)
- [QR density demonstration](../media/demo-density-scanning.mp4)
- [Size ladder demonstration](../media/demo-qr-ladder.mp4)
- [Original mixed test sheet](../test-sheets/manufacturing_barcode_test.pdf)
- [PCB marking sheet](../test-sheets/PCB_Data_Matrix_and_QR_Test.pdf)
- [QR density sheet](../test-sheets/QR_information_density_test.pdf)
- [Detailed report and methodology](../TEST_REPORT.md)
- [Per-target observations and uncertainty](../MARKED_RESULTS.md)
- [Verification plan](../VERIFICATION_PLAN.md)

Print at 100% scale and present one symbol at a time. Record actual dimensions, printer settings, working distance, resolution, illumination, decode mode, time to read, failures and repetitions. Repeat with higher-quality marking to separate printer from optical constraints. No statistical maximum payload, repeatability or throughput has been established.
