"""Generate small deterministic barcode matrices using zxing-cpp."""
import zxingcpp
from pathlib import Path
root=Path(__file__).resolve().parents[1]
samples=[('qr',zxingcpp.BarcodeFormat.QRCode,'PCB-TEST-01'),('dm',zxingcpp.BarcodeFormat.DataMatrix,'LOT-TEST-02')]
out=['#pragma once']
for name,fmt,payload in samples:
    img=zxingcpp.write_barcode(fmt,payload,0,0)
    v=memoryview(img);h,w=v.shape;pixels=v.tobytes()
    out.append(f'static constexpr int {name}W={w}, {name}H={h};')
    out.append(f'static const char {name}Matrix[] PROGMEM="'+''.join('1' if x<128 else '0' for x in pixels)+'";')
(root/'src/native_fixtures.h').write_text('\n'.join(out)+'\n')
