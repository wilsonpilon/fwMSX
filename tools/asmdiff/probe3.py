#!/usr/bin/env python3
"""Como probe.py, mas mostra de cada saida binaria so' as faixas nao-zero (offset:hex) e o tamanho total."""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from oracle import run_many


def runs(b):
    out = []
    i = 0
    n = len(b)
    while i < n:
        if b[i] == 0:
            i += 1
            continue
        j = i
        gap = 0
        while j < n and (b[j] != 0 or (gap < 8 and any(b[j:j + 8]))):
            j += 1
        out.append("%x:%s" % (i, b[i:j].hex()))
        i = j
    return out


def main():
    srcs = [s.strip("\n") + "\n" for s in sys.stdin.read().split("\n=====\n")]
    res = run_many(srcs)
    for s, r in zip(srcs, res):
        print("---", s.strip().replace("\n", " | ")[:110])
        print("  rc=%d" % r["rc"], " / ".join(r["msgs"]))
        for k, v in sorted(r["outs"].items()):
            if k == "sym":
                print("  sym:", " ".join(l.strip() for l in v.decode("latin1").splitlines() if l and not l.startswith(";")))
                continue
            if k == "txt":
                print("  txt:", " | ".join(l for l in v.decode("latin1").splitlines()[3:]))
                continue
            if len(v) <= 64:
                print("  %s[%d]=%s" % (k, len(v), v.hex()))
            else:
                print("  %s[%d] %s" % (k, len(v), " ".join(runs(v)[:12])))


if __name__ == "__main__":
    main()
