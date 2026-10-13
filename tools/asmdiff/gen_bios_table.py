#!/usr/bin/env python3
"""Extrai do manual (resource/asmsx.md) as tabelas de rotulos predefinidos da BIOS e variaveis de sistema e
CONFERE cada valor no asMSX de verdade (`.bios` + `dw NOME`); gera src/asm/cpp/bios_tables.inc."""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(__file__))
from oracle import run_many

MANUAL = os.path.join(os.path.dirname(__file__), "..", "..", "resource", "asmsx.md")
text = open(MANUAL, encoding="utf-8").read()

bios = []
for m in re.finditer(r"\|\s*([A-Z][A-Z0-9]+)\s*\|\s*([0-9a-f]{4})h\s*\|", text):
    bios.append((m.group(1), int(m.group(2), 16)))
vars_ = []
for m in re.finditer(r"\|\s*([A-Z][A-Z0-9_]+)\s*\|\s*\$([0-9a-f]{4})\s*\|", text):
    vars_.append((m.group(1), int(m.group(2), 16)))
bios = list(dict.fromkeys(bios))
vars_ = list(dict.fromkeys(vars_))
print(len(bios), "rotulos da BIOS;", len(vars_), "variaveis", file=sys.stderr)


def check(items, directive):
    res = run_many(["%s\ndw %s\n" % (directive, n) for n, _ in items])
    bad = []
    for (n, v), r in zip(items, res):
        z = r["outs"].get("z80", b"")
        got = (z[0] | (z[1] << 8)) if len(z) >= 2 else None
        if r["rc"] != 0 or got != v:
            bad.append((n, v, got, r["msgs"][:1]))
    return bad


bad_b = check(bios, ".bios")
bad_v = check(vars_, ".biosvars")
print("divergencias BIOS:", bad_b, file=sys.stderr)
print("divergencias VARS:", bad_v, file=sys.stderr)

out = os.path.join(os.path.dirname(__file__), "..", "..", "src", "asm", "cpp", "bios_tables.inc")
with open(out, "w", newline="\n") as f:
    f.write("// GERADO por tools/asmdiff/gen_bios_table.py a partir do manual do asMSX (secoes 2.2.1 e .BIOSVARS) e\n")
    f.write("// conferido contra o asMSX de verdade. Nao edite a mao.\n")
    f.write("static const struct { const char *name; int value; } kBiosLabels[] = {\n")
    for n, v in bios:
        f.write('    {"%s", 0x%04X},\n' % (n, v))
    f.write("};\nstatic const struct { const char *name; int value; } kBiosVars[] = {\n")
    for n, v in vars_:
        f.write('    {"%s", 0x%04X},\n' % (n, v))
    f.write("};\n")
