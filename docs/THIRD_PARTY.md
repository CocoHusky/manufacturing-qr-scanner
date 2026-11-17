# Third-party components

- OV5640AF: https://github.com/0015/ESP32-OV5640-AF — upstream commit b22b2bc1c1d3ac0f2f18c3f121f82b189889c79e. Local changes: signed register return codes and periodic delay during firmware loading. See vendored LICENSE.
- quirc: https://github.com/dlbeer/quirc — legacy decoder with PSRAM allocation changes, not active browser decoder. Exact revision not recorded. See vendored LICENSE.
- ZXing-WASM 3.1.7: https://github.com/Sec-ant/zxing-wasm — embedded JS and WASM assets. Package notice in lib/ZXing-LICENSE.txt; verify all transitive notices including ZXing-C++ before public distribution.
- Arduino ESP32 / esp32-camera: supplied by pinned PlatformIO package.

The root [project license](../LICENSE) covers original project software only. Third-party components and their applicable upstream portions, including embedded or generated assets, are excluded from the custom noncommercial restrictions. Preserve upstream copyright, license and notice requirements. A complete transitive dependency and redistribution compliance audit remains outstanding.

- Native ZXing-C++: https://github.com/zxing-cpp/zxing-cpp/tree/v2.2.1 (2.2.1, tag 99a83b3a6ac514d7f850dda7fa24cddb5120c7e2). Apache-2.0 license in lib/ZXingNative/LICENSE. Local MultiFormatReader change limits native readers to QR and Data Matrix; build filters omit unused format implementations. Retained upstream source notices.

- qrcode-generator 1.4.4, Kazuhiko Arase, MIT: web/qrcode-LICENSE.txt.
