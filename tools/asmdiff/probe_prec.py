#!/usr/bin/env python3
"""Descobre a precedencia e a associatividade dos operadores binarios do asMSX (caixa-preta)."""
import itertools
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from oracle import run_many

OPS = ["+", "-", "*", "/", "%", "<<", ">>", "|", "&", "^", "&&", "||", "==", "!=", "<", "<=", ">", ">="]


def ev(op, a, b):
    if op == "+": return a + b
    if op == "-": return a - b
    if op == "*": return a * b
    if op == "/": return int(a / b) if b else None
    if op == "%": return a - int(a / b) * b if b else None
    if op == "<<": return (a << b) if 0 <= b < 32 else None
    if op == ">>": return (a >> b) if 0 <= b < 32 else None
    if op == "|": return a | b
    if op == "&": return a & b
    if op == "^": return a ^ b
    if op == "&&": return int(bool(a) and bool(b))
    if op == "||": return int(bool(a) or bool(b))
    if op == "==": return int(a == b)
    if op == "!=": return int(a != b)
    if op == "<": return int(a < b)
    if op == "<=": return int(a <= b)
    if op == ">": return int(a > b)
    if op == ">=": return int(a >= b)


TRIPLES = [t for t in itertools.product([0, 1, 2, 3, 5, 6, 7, 8, 12], [0, 1, 2, 3, 4, 5], [0, 1, 2, 3, 5, 6])]


def main():
    cases = []
    for o1, o2 in itertools.product(OPS, OPS):
        for (x, y, z) in TRIPLES:
            left = ev(o2, ev(o1, x, y), z) if ev(o1, x, y) is not None else None
            right = ev(o1, x, ev(o2, y, z)) if ev(o2, y, z) is not None else None
            if left is None or right is None or left == right or not (-30000 < left < 30000) or not (-30000 < right < 30000):
                continue
            cases.append((o1, o2, x, y, z, left, right))
            break
    srcs = ["org 100h\ndw %d%s%d%s%d\n" % (c[2], c[0], c[3], c[1], c[4]) for c in cases]
    res = run_many(srcs, workers=16)
    out = {}
    for c, r in zip(cases, res):
        o1, o2, x, y, z, left, right = c
        z80 = r["outs"].get("z80", b"")
        v = (z80[0] | (z80[1] << 8)) if len(z80) >= 2 else None
        if v is not None and v >= 0x8000:
            v -= 0x10000
        out[(o1, o2)] = "L" if v == left else ("R" if v == right else "?%s" % v)
    print("linha = op1, coluna = op2 ; L = (a op1 b) op2 c ; R = a op1 (b op2 c)")
    print("     " + " ".join("%4s" % o for o in OPS))
    for o1 in OPS:
        print("%4s " % o1 + " ".join("%4s" % out.get((o1, o2), ".") for o2 in OPS))


if __name__ == "__main__":
    main()
