# Comparative workflow validation protocol

## Objective

Measure the two intended benefits: less operator time spent recording information and fewer manual-entry errors.

## Design

Use the same six-component assembly and identical ground-truth labels for handwritten travelers, manual computer entry and embedded scanning. Include device identification, six components, visual inspection and final confirmation. Use a comparable validated computer entry form rather than intentionally weakening the baseline. Counterbalance method order, use a practice run, and record operator experience. A pilot with several repeated trials per operator can estimate variability before determining sample size.

Run normal trials separately from fault-injection trials (wrong component, duplicate, wrong device, missing part). Include comparable prompts and error correction opportunities across methods. For each method, retain the original record and compare against ground truth.

| Metric | Definition |
| --- | --- |
| Total cycle time | Start to complete record, including corrections |
| Documentation time | Time spent recording identifiers or results, separate from physical assembly/test |
| Manual entries | Number of identifiers manually transcribed |
| Incorrect records | Incorrect identifier values persisted at closure |
| Duplicate/missing items | Duplicate or omitted required components persisted at closure |
| Detection rate | Injected errors detected before closure / injected errors |
| Completeness | Required fields and relations present / total required |
| Rework | Time and actions needed to correct records |

Record per-trial data in [trial template](trial-template.csv). Report sample counts, median and spread of cycle times, and error counts with denominators. Include failed and incomplete trials; do not discard them to improve apparent speed.

No comparative results have been collected. Host regression tests validate workflow logic, not human productivity or production reliability.

## Documentation time estimates

For a six-component assembly traveler, compare manual documentation with our system’s scan-driven approach:

| Documentation activity | Handwritten | Manual computer entry | **Our system (estimated)** |
| --- | ---: | ---: | ---: |
| Record and check identifiers | 3–5 min | 2–4 min | **10–60 s** |
| Investigate a missing or incorrect identifier (per incident) | 10–60 min | 1–30 min | 0 min* |

### Estimated documentation effort — 100 devices

| Activity | Handwritten | Manual computer entry | **Our system (estimated)** |
| --- | ---: | ---: | ---: |
| Record and check identifiers (100 devices) | 5–8 hr 20 min | 3 hr 20 min–6 hr 40 min | **17 min–1 hr 40 min** |
| Investigate 10 manual-entry incidents | 1 hr 40 min–10 hr | 10 min–5 hr | 0 min* |
| **Combined estimated time** | **6 hr 40 min–18 hr 20 min** | **3 hr 30 min–11 hr 40 min** | **17 min–1 hr 40 min** |

Engineering estimates assume 10 manual-entry incidents per 100 devices. *Zero minutes models elimination of transcription-related investigations only; other errors and rework remain possible. These are planning assumptions, not measured results.
