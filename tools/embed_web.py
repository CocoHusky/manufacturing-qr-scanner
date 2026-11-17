from pathlib import Path
root=Path(__file__).resolve().parents[1]
with (root/'src/web_assets.h').open('w') as out:
    out.write('#pragma once\n')
    for name,file in [('TRACE_JS','traceability.js'),('DASH_JS','dashboard.js')]:
        out.write('static const char '+name+'[] PROGMEM=R"WEB('+ (root/'web'/file).read_text()+')WEB";\n')
