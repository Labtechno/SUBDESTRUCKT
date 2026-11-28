"""Component Lab : signale les littéraux C++ non ASCII qui ne passent pas par juce::String::fromUTF8 (ou l'alias u8 du
laboratoire). JUCE interprète un const char* comme ASCII : un littéral UTF-8 non converti s'affiche mal.
Usage : py -3.11 lab/tools/check_literals.py   (code de sortie 1 si un littéral fautif est trouvé)"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
LIT = re.compile(r'(u8\(|fromUTF8\()?"((?:[^"\\]|\\.)*)"')
bad = 0
for p in sorted(list(ROOT.glob('r2/*.cpp')) + list(ROOT.glob('app/*.cpp'))):
    for i, line in enumerate(p.read_text(encoding='utf-8').splitlines(), 1):
        if 'utf8-ok' in line or line.strip().startswith('"'):  # suite d'un littéral concaténé, déjà passé à u8( / fromUTF8(
            continue
        code = line.split('//')[0]
        for m in LIT.finditer(code):
            if m.group(1) is None and any(ord(c) > 127 for c in m.group(2)):
                print(f'{p.relative_to(ROOT)}:{i}: {m.group(2)[:70]}')
                bad += 1
print(f'{bad} literal(s) to fix')
sys.exit(1 if bad else 0)
