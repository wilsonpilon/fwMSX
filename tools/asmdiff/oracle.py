#!/usr/bin/env python3
"""Utilitarios para falar com o asMSX de verdade (o ORACULO de teste do montador do fwMSX).

O asMSX (GPLv3) NAO faz parte do fwMSX: e' so' um programa externo que rodamos para ver o que ele
produz (caixa-preta). O montador do fwMSX foi escrito do zero a partir do manual. Aqui ficam as
funcoes comuns dos geradores de casos de teste (tools/asmdiff/gen_*.py).
"""
import os
import re
import shutil
import subprocess
import tempfile
from concurrent.futures import ThreadPoolExecutor

ORACLE = os.environ.get("ASMSX_ORACLE") or os.path.join(os.path.dirname(__file__), "..", "..", "resource", "asmsx.exe")
ORACLE = os.path.abspath(ORACLE)

# Linhas de progresso do asMSX que nao interessam ao teste (so' as mensagens de erro/aviso).
_NOISE = re.compile(r"^(-{10,}|\s*asMSX v.*|Parsing file.*|Expanding.*|DONE Expanding.*|Assembling.*|Removed temporary.*|"
                    r"Binary file .* saved|.* file .* saved|Completed in .*|\s*)$")
OUTPUT_EXTS = ["z80", "bin", "com", "rom", "cas", "wav", "sym", "txt", "tap"]


def run_case(source, extra_args=(), files=None):
    """Monta `source` (texto) num diretorio temporario proprio. Devolve dict com rc, mensagens e saidas.

    files: dict nome->bytes de arquivos auxiliares (para .include / .incbin).
    """
    d = tempfile.mkdtemp(prefix="asmdiff_")
    try:
        with open(os.path.join(d, "x.asm"), "w", newline="\n") as f:
            f.write(source)
        for name, data in (files or {}).items():
            p = os.path.join(d, name)
            os.makedirs(os.path.dirname(p), exist_ok=True)
            with open(p, "wb") as f:
                f.write(data if isinstance(data, bytes) else data.encode("latin-1"))
        proc = subprocess.run([ORACLE, "-s", *extra_args, "x.asm"], cwd=d, capture_output=True, timeout=60)
        text = (proc.stdout + proc.stderr).decode("latin-1")
        msgs = [l for l in text.replace("\r", "").split("\n") if not _NOISE.match(l)]
        outs = {}
        # Todos os arquivos novos viram saidas (o nome base pode mudar com .FILENAME e -o); a chave e' o
        # caminho relativo ao diretorio de trabalho, com o nome-base trocado por "@" quando e' x.* .
        known = {"x.asm"} | set((files or {}).keys())
        for root, _dirs, fnames in os.walk(d):
            for fn in fnames:
                rel = os.path.relpath(os.path.join(root, fn), d).replace("\\", "/")
                if rel in known or fn.startswith("~tmppre"):
                    continue
                with open(os.path.join(root, fn), "rb") as f:
                    data = f.read()
                key = rel
                if rel.startswith("x.") and rel.count(".") == 1:
                    key = rel[2:]
                outs[key] = data
        return {"rc": proc.returncode, "msgs": msgs, "outs": outs}
    finally:
        shutil.rmtree(d, ignore_errors=True)


def run_many(sources, workers=12):
    with ThreadPoolExecutor(max_workers=workers) as ex:
        return list(ex.map(run_case, sources))
