#!/usr/bin/env python3
"""Gera tests/asm/golden/instr.golden: o conjunto de instrucoes do Z80 (oficiais e nao oficiais) visto
pelo asMSX de verdade. Candidatos = produto cartesiano dos mnemonicos pelos operandos; guarda os que o
asMSX aceita e uma amostra dos que ele recusa."""
import itertools
import os
import random
import sys

sys.path.insert(0, os.path.dirname(__file__))
from golden import write_golden
from oracle import run_many

R8 = ["a", "b", "c", "d", "e", "h", "l", "ixh", "ixl", "iyh", "iyl", "i", "r", "f"]
R16 = ["af", "af'", "bc", "de", "hl", "sp", "ix", "iy"]
IND = ["[hl]", "[bc]", "[de]", "[sp]", "[ix]", "[iy]", "[ix+5]", "[iy-3]", "[ix+127]", "[ix-128]", "[ix+200]", "[c]", "[1234h]", "[5]", "[lbl]", "[lblf]"]
IMM = ["0", "5", "7", "8", "56", "255", "256", "-1", "1234h", "lbl", "lblf", "lbl+2", "$", "$+3"]
CONDS = ["nz", "z", "nc", "c", "po", "pe", "p", "m"]

ZERO = ("nop halt di ei exx ex_ ldi ldir ldd lddr cpi cpir cpd cpdr ini inir ind indr outi otir outd otdr daa cpl neg ccf scf "
        "rlca rrca rla rra rld rrd reti retn ret ei ").split()
ZERO = [z for z in ZERO if z != "ex_"]
ONE_OP = "inc dec push pop sub and or xor cp jp call djnz jr rst im rlc rrc rl rr sla sra sll srl neg out in ret".split()
TWO_OP = "ld add adc sbc ex jp jr call ret bit res set out in rlc rrc rl rr sla sra sll srl im".split()
THREE_OP = "bit res set".split()

OPS_ALL = R8 + R16 + IND + IMM + CONDS
BITS = ["0", "3", "7", "8"]


def gen():
    cands = []
    for m in ZERO:
        cands.append(m)
    for m in ONE_OP:
        for a in OPS_ALL:
            cands.append("%s %s" % (m, a))
    for m in TWO_OP:
        for a in OPS_ALL:
            for b in OPS_ALL:
                if m in ("bit", "res", "set"):
                    if a not in BITS:
                        continue
                elif m in ("ld", "add", "adc", "sbc", "ex", "out", "in"):
                    pass
                elif m in ("jp", "jr", "call", "ret"):
                    if a not in CONDS + ["lbl", "lblf", "5", "1234h"]:
                        continue
                elif a in BITS and False:
                    continue
                cands.append("%s %s,%s" % (m, a, b))
    # 3 operandos (nao oficiais): bit/set/res n,[ix+d],r ; rotacoes [ix+d],r ja' sao 2 operandos acima
    for m in THREE_OP:
        for bt in BITS:
            for ind in ["[ix+5]", "[iy-3]", "[hl]"]:
                for r in R8[:7] + ["ixh", "[hl]"]:
                    cands.append("%s %s,%s,%s" % (m, bt, ind, r))
    seen = set()
    out = []
    for c in cands:
        if c not in seen:
            seen.add(c)
            out.append(c)
    return out


def wrap(line):
    return "org 100h\nlbl: nop\n%s\nlblf: nop\n" % line


def main():
    cands = gen()
    print("candidatos:", len(cands), file=sys.stderr)
    cache = os.environ.get("ASMDIFF_CACHE", "instr_cache.pkl")
    if os.path.exists(cache):
        import pickle
        with open(cache, "rb") as f:
            results = pickle.load(f)
    else:
        results = run_many([wrap(c) for c in cands], workers=16)
        import pickle
        with open(cache, "wb") as f:
            pickle.dump(results, f)
    ok, bad = [], []
    for c, r in zip(cands, results):
        (ok if r["rc"] == 0 else bad).append((c, r))
    print("aceitos:", len(ok), "recusados:", len(bad), file=sys.stderr)
    random.seed(7)
    # Recusados: TODOS os que nao sao "syntax error" (erros semanticos tem mensagem propria) + uma amostra dos
    # erros de sintaxe.
    semantic = [b for b in bad if b[1]["rc"] != 1]
    syntax = [b for b in bad if b[1]["rc"] == 1]
    sample = semantic + random.sample(syntax, min(400, len(syntax)))
    cases = []
    for i, (c, r) in enumerate(ok):
        cases.append(("ok%05d" % i, wrap(c), r))
    for i, (c, r) in enumerate(sample):
        cases.append(("bad%04d" % i, wrap(c), r))
    out = os.path.join(os.path.dirname(__file__), "..", "..", "tests", "asm", "golden", "instr.golden")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    write_golden(out, cases)
    print("escrito", os.path.abspath(out), file=sys.stderr)


if __name__ == "__main__":
    main()
