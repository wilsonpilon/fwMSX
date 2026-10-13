#!/usr/bin/env python3
"""Roda varios fontes (separados por uma linha '=====') no asMSX de verdade e mostra o resultado de cada um.
Uso: probe.py < arquivo   |   probe.py "fonte1" "fonte2"   (barra invertida literal, sem interpretar)."""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from oracle import run_many


def main():
    if len(sys.argv) > 1:
        srcs = [a.replace("\n", "\n") if False else a for a in sys.argv[1:]]
    else:
        srcs = [s.strip("\n") + "\n" for s in sys.stdin.read().split("\n=====\n")]
    res = run_many(srcs)
    for s, r in zip(srcs, res):
        head = s.strip().replace("\n", " | ")[:70]
        outs = " ".join("%s=%s" % (k, v.hex() if len(v) <= 40 else "<%dB>" % len(v)) for k, v in sorted(r["outs"].items()) if k not in ("sym",))
        print("%-72s rc=%d %s %s" % (head, r["rc"], " / ".join(m[:60] for m in r["msgs"]), outs))


if __name__ == "__main__":
    main()
