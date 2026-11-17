# Supported barcode formats and recorded data

The symbol format determines how information is encoded; the payload determines what the scanner records. The same structured manufacturing payload can be encoded in QR or Data Matrix.

| Format | Example payload | Browser | Embedded |
| --- | --- | --- | --- |
| Data Matrix | `LOT-TEST-02` | Yes | Yes |
| QR | `PCB-TEST-01` | Yes | Yes |
| Code 128 | `0101` | Yes | Workflow headers while idle |
| Code 39 | `01` | Yes | No |

![Barcode examples](examples.svg)

## Structured label examples

| Record | Example encoded payload |
| --- | --- |
| Component | `PN=PCB-001;LOT=PCB-261101;SN=PCB-0025` |
| Device | `SN=ASM-DEV-01;LOT=ASM-261117;REV=A` |
| Equipment | `ID=BENCH-007` |
| Electrical result | `ID=RUN-0262P;STATUS=PASS;V=5.02V;I=62mA;DATA=RUN-0262P` |
| Thermal failure | `ID=RUN-0263F;STATUS=FAIL;T=38.4C;REASON=TEMP_HIGH` |

These compact examples illustrate the general parser. A controlled workflow additionally requires recipe, step, label type and the appropriate identity fields. For complete scannable workflow labels, use the [shared instruction PDF](../workflows/instructions.pdf).

The browser parser also supports plain identifiers, flat JSON, semicolon-separated key/value fields, ISO/IEC 15434 format 06 with supported data identifiers, and configurable prefix/separator rules. Recognized fields include part, lot, serial, quantity, date code, country, supplier, revision, device, equipment, chamber, test result, measurement values and data reference. Unknown formats can be retained as raw identifiers or payloads; recognition is not certification of compliance with a barcode standard.

The scanner adds operator context, timestamps and workflow events to its records. It does not retrieve external measurement files or automatically link inspection images to a workflow.

[Original test sheets](../test-sheets/) · [Characterization](../characterization/README.md)
