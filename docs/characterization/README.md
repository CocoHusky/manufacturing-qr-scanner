# Optical characterization

The initial goal is a practical marking envelope for prototype assembly and test: QR at 5–15 mm and compact Data Matrix down to 4 mm for identifier marking on PCBs.

| Format | Footprint including quiet zone | Reported result |
| --- | --- | --- |
| Data Matrix | 4 mm | 16 characters |
| Data Matrix | 5–6 mm | 32 characters |
| Data Matrix | 8 mm | 96 characters |
| Data Matrix | 10 mm | Conditional 96-character report; field-of-view conflict |
| QR | 5 mm | 16 characters (alphanumeric) |
| QR | 8 mm | Up to 96 characters reported for alphanumeric L; inferred cutoff |
| QR | 10–12 mm | 96-character candidates; 128-character failure not isolated |
| QR | 15 mm | Short PCB identifier read in separate comparison |

![Payload capacity](../figures/capacity.svg)

These are qualitative operator-reported observations, not guaranteed limits. Exact encoding, error correction, quiet zone and print quality affect payload density. A compact identity code can reference a richer external record rather than embedding every field.

The current laser printer constrains module fidelity; 4 mm Data Matrix with 16 ASCII characters is the smallest demonstrated compact result in this setup. This does not establish a camera-only resolution limit. Contrast, print resolution, focus distance, full-frame field of view, exposure and motion have not been separated. Direct marking on manufactured PCBs remains unqualified.

## Evidence and reproduction

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
