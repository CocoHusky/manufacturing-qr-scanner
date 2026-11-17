"""Run after the first USB installation of the dual-slot layout."""
import argparse
import getpass
import subprocess
import sys
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('host')
p.add_argument('--firmware', default='.pio/build/esp32cam/firmware.bin')
a = p.parse_args()
script = Path.home()/'.platformio/packages/framework-arduinoespressif32/tools/espota.py'
if not script.is_file():
    p.error('Arduino ESP32 framework is missing; run pio run first')
password = getpass.getpass('Device OTA password: ')
if not password:
    p.error('Password required')
sys.exit(subprocess.call([sys.executable, str(script), '-i', a.host, '-p', '3232', '-a', password, '-f', a.firmware]))
