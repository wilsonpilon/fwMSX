#!/usr/bin/env python3
"""Gera src/editor/cpp/syntax_words_gen.inc: as palavras de destaque de sintaxe do editor tiradas do man do fwMSX.

Fontes (so' leitura): src/man/cpp/man_pages_basic.cpp, man_pages_tools_help.cpp, man_pages_manuals.cpp.
  - MSX-BASIC: nomes dos topicos da categoria "MSX-BASIC" (uma palavra, com $ opcional; "a/b/c" vira 3; "call x" vira extensao CALL).
  - MSXBAS2ROM: "CMD PALAVRA" do guia de referencia.
  - NestorBASIC: os apelidos "nb_xxx" (nomes de funcao).
Uso: python tools/editor/gen_syntax_words.py   (da raiz do repositorio). Nao edite o .inc a mao.
"""
import io
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
man = ROOT + "/src/man/cpp/"

def read(name):
    with io.open(man + name, encoding="utf-8", errors="replace") as f:
        return f.read()

basic_src = read("man_pages_basic.cpp") + read("man_pages_manuals.cpp") + read("man_pages_books.cpp")
tools_src = read("man_pages_tools_help.cpp")

add_re = re.compile(r'add\("([^"]*)",\s*"([^"]*)",\s*"([^"]*)"')
words = set()
call_ext = set()
nb_funcs = set()
for name, aliases, cat in add_re.findall(basic_src + tools_src):
    if cat == "MSX-BASIC":
        n = name.lower()
        if n.startswith("call "):
            ext = n[5:].strip()
            if re.fullmatch(r"[a-z0-9]+", ext):
                call_ext.add(ext)
            continue
        for part in n.split("/"):
            part = part.strip().rstrip("?=")
            if re.fullmatch(r"[a-z]+\$?", part):
                words.add(part)
            elif re.fullmatch(r"[a-z]+-[a-z]+", part):  # for-next, gosub-return, if-then-else, on-goto...
                for p in part.split("-"):
                    words.add(p)
    for a in aliases.split(","):
        a = a.strip().lower()
        if a.startswith("nb_") and re.fullmatch(r"nb_[a-z0-9_]+", a):
            nb_funcs.add(a)

# palavras que a lista de topicos nao traz como topico proprio
words |= set("to then else step next goto gosub return and or xor eqv imp mod not off on using as tab spc fn error resume sub".split())
words -= set(["sub", "operators", "variables"])
words |= set(["def", "date", "random"])

bas2rom = set(re.findall(r"CMD\s+([A-Z][A-Z0-9]+)", tools_src))

def emit(name, items, wrap=100):
    out = ["static const char *const %s =" % name]
    line = ""
    for w in sorted(items):
        if len(line) + len(w) + 1 > wrap:
            out.append('    "%s "' % line.rstrip())
            line = ""
        line += w + " "
    if line:
        out.append('    "%s ";' % line.rstrip())
    else:
        out[-1] += ";"
    out[-1] = out[-1] if out[-1].endswith(";") else out[-1] + ";"
    return "\n".join(out)

with io.open(ROOT + "/src/editor/cpp/syntax_words_gen.inc", "w", encoding="utf-8", newline="\n") as f:
    f.write("// GERADO por tools/editor/gen_syntax_words.py a partir do man do fwMSX. Nao edite a mao.\n")
    f.write("// (so' arquivo de dados incluido por syntax_langs.cpp)\n\n")
    f.write(emit("kGenBasicWords", words) + "\n\n")
    f.write(emit("kGenCallExt", call_ext) + "\n\n")
    f.write(emit("kGenBas2RomCmds", set(w.lower() for w in bas2rom)) + "\n\n")
    f.write(emit("kGenNestorFuncs", nb_funcs) + "\n")
print("basic=%d call_ext=%d bas2rom=%d nestor=%d" % (len(words), len(call_ext), len(bas2rom), len(nb_funcs)))
