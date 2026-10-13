#!/usr/bin/env python3
"""Mostra um .golden de forma compacta: caso, rc, mensagens e saidas (txt em texto)."""
import sys

def main():
    path = sys.argv[1]
    only = sys.argv[2] if len(sys.argv) > 2 else ""
    case = None
    for line in open(path, encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("@case "):
            case = {"name": line[6:], "src": [], "msgs": [], "outs": [], "args": "", "in_src": False, "rc": 0}
        elif case is None:
            continue
        elif line == "@src":
            case["in_src"] = True
        elif case["in_src"] and not line.startswith("@rc "):
            case["src"].append(line)
        elif line.startswith("@rc "):
            case["in_src"] = False
            case["rc"] = int(line[4:])
        elif line.startswith("@args "):
            case["args"] = line[6:]
        elif line.startswith("@msg "):
            case["msgs"].append(line[5:])
        elif line.startswith("@out "):
            _, name, hx = (line.split(" ", 2) + [""])[:3]
            name = name.replace("%20", " ")
            b = bytes.fromhex(hx)
            if name.endswith("sym") or name.endswith("txt"):
                txt = b.decode("latin-1").replace("\r\n", "\r\n ").replace("\n", "\n ")
                case["outs"].append("%s=[%s]" % (name, txt))
            else:
                case["outs"].append("%s=%s" % (name, hx if len(hx) <= 80 else hx[:76] + "...(%d)" % len(b)))
        elif line == "@end":
            if only in case["name"]:
                print("## %s %s rc=%d" % (case["name"], case["args"], case["rc"]))
                print("   src: " + " | ".join(case["src"])[:150])
                for m in case["msgs"]:
                    print("   msg: " + m[:150])
                for o in case["outs"]:
                    print("   out: " + o[:230])

main()
