#!/usr/bin/env python3
# (Bump It) one self-contained installer page: SLOOP's own web/index_pkg.html with the firmware packed inside.
# usage: make_bumpit_installer.py build/felucca.fwsc VERSION out.html
import base64, hashlib, json, re, sys

fw, version, out = sys.argv[1], sys.argv[2], sys.argv[3]
pkg = open(fw, 'rb').read()
t = open('web/index_pkg.html', encoding='utf-8').read()
lib = ''
for f in ('web/fm1pkg.js', 'web/fm1ota.js'):
    lib += re.sub(r'(?m)^export\s+', '', open(f, encoding='utf-8').read()) + '\n'
bars = ''.join('<rect x="%d" y="200" width="%d" height="12" rx="6" fill="%s"/>' % (60 + i * 160, 150, c)
               for i, c in enumerate(('#B45AFF', '#E040FB', '#7C4DFF', '#C77DFF')))
logo = ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 760 260" width="760" height="260" role="img" '
        'aria-label="BUMP IT"><rect width="760" height="260" fill="#000"/>'
        '<text x="380" y="165" text-anchor="middle" font-family="Arial Black,Arial,sans-serif" font-weight="900" '
        'font-size="150" fill="#B45AFF">BUMP IT</text>' + bars + '</svg>')
meta = {'version': version, 'product': 'FM-1_900',
        'pkg': 'data:application/octet-stream;base64,' + base64.b64encode(pkg).decode(),
        'sha256': hashlib.sha256(pkg).hexdigest()}
rep = [('<title>SLOOP installer</title>', '<title>Bump It installer (FM-1)</title>'),
       ('<!--LOGO-->', logo), ('<!--STUDIO_GUIDE-->', ''), ('<!--BANNER-->', ''),
       ('/*LIB*/', lib), ('/*META*/', json.dumps(meta)),
       ('say(`SLOOP ${meta.version}`);', 'say(`Bump It ${meta.version}`);')]
for a, b in rep:
    assert t.count(a) == 1, a
    t = t.replace(a, b)
open(out, 'w', encoding='utf-8').write(t)
print(out, len(t), 'B, firmware sha256', meta['sha256'])
