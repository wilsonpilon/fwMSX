#!/usr/bin/env python3
"""Gera src/asm/cpp/tables_data.inc (nomes e valores da BIOS e das variaveis de sistema do MSX) a partir das
tabelas do manual do asMSX (resource/asmsx.md, secoes 2.2.1 e .BIOSVARS). Sao fatos do MSX (enderecos da
BIOS), nao codigo."""
import os
import re

root = os.path.join(os.path.dirname(__file__), "..", "..")
txt = open(os.path.join(root, "resource", "asmsx.md"), encoding="utf-8").read()
bios, seen = [], set()
for line in txt.split("\n"):
    if re.match(r"^\|\s*[A-Z0-9_]+\s*\|\s*[0-9a-f]+h\s*\|", line):
        for m in re.finditer(r"\|\s*([A-Z0-9_]+)\s*\|\s*([0-9a-f]+)h\s*\|", line):
            if m.group(1) not in seen:
                seen.add(m.group(1))
                bios.append((m.group(1), int(m.group(2), 16)))
vars_ = []
for line in txt.split("\n"):
    m = re.match(r"^\|\s*([A-Z0-9_]+)\s*\|\s*\$([0-9a-f]+)\s*\|", line)
    if m:
        vars_.append((m.group(1), int(m.group(2), 16)))
out = os.path.join(root, "src", "asm", "cpp", "tables_data.inc")
with open(out, "w", newline="\n") as f:
    f.write("// Gerado por tools/asmdiff/gen_tables.py a partir da tabela do manual do asMSX (secao 2.2.1 e .BIOSVARS).\n")
    f.write("static const BiosEntry kBiosCalls[] = {\n")
    for n, v in bios:
        f.write('    {"%s", 0x%04X},\n' % (n, v))
    f.write("};\nstatic const BiosEntry kBiosVars[] = {\n")
    for n, v in vars_:
        f.write('    {"%s", 0x%04X},\n' % (n, v))
    f.write("};\n")
print(len(bios), len(vars_))
