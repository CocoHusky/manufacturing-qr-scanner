# Verification plan (proposed)

1. Record print model/DPI/media, measure 50 mm scale, inspect print under microscope. Record camera/lens, working distance, resolution, ROI, LED PWM, exposure, ambient lighting, firmware commit and browser.
2. Minimum marking: 0.5–5 mm square QR/ECC200, 1/4/8/16 characters. Maximum QR payload: 80/96/104/112/120/128 characters at usable 8–12 mm sizes. Retest exact failed 128-character samples. The final boundary PDF was not found in local Downloads; add original when available.
3. Dimensional tolerances: nominal and ±5% side length at 4,5,8,10,12 mm (3.8/4.2, 4.75/5.25, 7.6/8.4, 9.5/10.5, 11.4/12.6 mm).
4. Twenty independent presentations per condition; remove code between trials. Record payload, correct/incorrect, timeout, duplicate and latency. Proposed screen: at least 19/20 correct at nominal and each endpoint, zero incorrect reads. This is 95% observed, not a 95% confidence bound. Increase trials/sample count before production qualification.
5. Measure temperature and input current versus time, including idle, browser closed, resolution/rate/PWM sweeps, and continuous scanning. Stop on abnormal heating, brownouts or instability. Separate acquisition, transfer, decode and time-to-first-read. Quantify motion and angular tolerance; no guaranteed 60 FPS.
6. Repeat with actual production PCB silkscreen or laser marks. Select smallest configuration with sufficient payload and reliability margin. Record all failures and untested conditions separately.
