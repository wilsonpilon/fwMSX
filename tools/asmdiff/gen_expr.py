#!/usr/bin/env python3
"""Gera tests/asm/golden/expr.golden: expressoes numericas vistas pelo asMSX de verdade. Cada caso monta
`dw <expr>` (16 bits, com aviso de corte quando passa) para registrar o valor e a aceitacao/recusa."""
import itertools
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from golden import write_golden
from oracle import run_many


def candidates():
    c = []
    # ---- formatos numericos
    for t in ["0", "1", "10", "255", "65535", "65536", "4294967295", "4294967296", "2147483647", "2147483648", "-1", "-32768",
              "-32769", "0x1f", "0X1F", "0xffff", "0x", "0xg", "$1f", "$1F", "$", "$ff", "1fh", "1FH", "0ffh", "0FFH", "ffh", "1ah",
              "10h", "0h", "h", "1010b", "1010B", "0b", "b", "102b", "101b", "0b101", "077", "0777", "08", "09", "010", "10o", "10O",
              "17o", "18o", "0o", "o", "1_000", "12h34", "1.0", ".5", "5.", "1e2", "'a'", "'A'", "' '", "'ab'", "''", "'\\n'", "'\\''",
              "'\"'", "\"a\"", "\"\"", "'a'+'b'", "'a'*2", "1 2", "1,2", "(1)", "((1))", "(1", "1)", "()", "( 1 )", "  7  "]:
        c.append(t)
    # ---- binarios
    vals = ["0", "1", "5", "7", "255", "-3", "65535"]
    for op in ["+", "-", "*", "/", "%", "<<", ">>", "|", "&", "^", "&&", "||", "==", "!=", "<", "<=", ">", ">=", "=", "<>", "**", "~", "!"]:
        for a, b in itertools.product(["0", "1", "5", "-3", "255", "7"], ["0", "1", "5", "-3", "2", "3"]):
            c.append("%s%s%s" % (a, op, b))
    for op in ["/", "%"]:
        c += ["5%s0" % op, "0%s0" % op, "-5%s2" % op, "5%s-2" % op, "-5%s-2" % op]
    # ---- unarios
    for t in ["-5", "- 5", "--5", "---5", "-(-5)", "~5", "~~5", "~-5", "-~5", "!5", "!0", "!!5", "!-1", "+5", "+ 5", "++5", "-+5", "not 5", "NOT 5", "not(5)", "~0", "~255", "~65535", "~(1<<16)", "-0", "- -5", "-(3+4)", "-3*2", "3*-2", "3 - -2", "3--2", "3+-2", "3-+2"]:
        c.append(t)
    # ---- precedencia
    for t in ["2+3*4", "(2+3)*4", "2*3+4", "2+3<<1", "2<<1+3", "1|2&3", "1|2^3&4", "1&&0||1", "1||0&&0", "1==1&&2==2", "1<2==1", "5>3>1", "8>>1>>1", "16>>2<<1", "2*3%4", "10-3-2", "10/3/2", "2+3==5", "1+1<<2", "1<<2+1", "5&3==3", "5|3==3", "!1+1", "~1+1", "-2*3", "- 2 * 3", "7 & 3 | 8", "1 + 2 * 3 - 4 / 2 % 3"]:
        c.append(t)
    # ---- deslocamentos
    for t in ["1<<15", "1<<16", "1<<31", "1<<32", "1<<33", "1<<63", "1<<64", "1<<-1", "-8>>1", "-1>>1", "-1>>15", "-1>>31", "0x8000>>15", "0xffff>>8", "65535<<1", "1<<0", "5>>0", "5>>-1", "-1<<1"]:
        c.append(t)
    # ---- grandes
    for t in ["100000*100000", "65536*65536", "2147483647+1", "-2147483648-1", "4294967295+1", "4294967296*4294967296", "99999999999", "0x7fffffff", "0x80000000", "0xffffffff", "0x100000000", "123456789*987654321"]:
        c.append(t)
    # ---- funcoes
    flo = ["0.0", "1.0", "2.0", "0.5", "-1.5", "3.14159", "100.0", "16.0", "pi", "pi/2", "pi*2", "-pi", "2", "0", "1"]
    for f in ["sin", "cos", "tan", "acos", "asin", "atan", "sqr", "sqrt", "exp", "log", "ln", "abs"]:
        for a in flo:
            c.append("int(%s(%s))" % (f, a))
            c.append("fix(%s(%s))" % (f, a))
    for a, b in itertools.product(["2.0", "0.5", "-2.0", "10.0", "0.0", "2", "pi"], ["2.0", "3.0", "0.5", "-1.0", "0.0", "2", "3"]):
        c.append("int(pow(%s,%s))" % (a, b))
        c.append("fix(pow(%s,%s))" % (a, b))
    for t in ["int(5)", "int(5.0)", "int(-5.9)", "int(5.9)", "int(0.0)", "int(pi)", "int(-pi)", "int(1e2)", "int(100000.0)", "int(70000.0)", "int(-70000.0)", "int(4294967296.0)", "int(1.0e10)",
              "fix(1.5)", "fix(-1.5)", "fix(0.0)", "fix(127.99)", "fix(128.0)", "fix(255.0)", "fix(255.996)", "fix(256.0)", "fix(-128.0)", "fix(-128.5)", "fix(-129.0)", "fix(0.003)", "fix(0.004)", "fix(1)", "fix(1+1.0)", "fix(1.0+1)",
              "fixmul(384,512)", "fixmul(fix(1.5),fix(2.0))", "fixmul(-384,512)", "fixmul(256,256)", "fixmul(65535,65535)", "fixmul(1,1)", "fixmul(1.0,1)", "fixmul(1)", "fixmul(1,2,3)",
              "fixdiv(768,512)", "fixdiv(fix(3.0),fix(2.0))", "fixdiv(1,0)", "fixdiv(-768,512)", "fixdiv(256,3)", "fixdiv(1,3)",
              "int(1.5+1.5)", "int(1.5*2.0)", "int(1.5*2)", "int(2*1.5)", "int(3/2.0)", "int(3.0/2)", "int(3.0/0.0)", "int(3.0/0)", "int(1.5%1.0)", "int(1.5<<1)", "int(1.5&1)", "int(1.5>1.0)", "int(1.5==1.5)", "int(-1.5)", "int(--1.5)", "int(+1.5)", "int(~1.5)", "int(!1.5)", "int((1.5))", "int((1.5)+1.0)", "int(1.5)*2", "int(1.5)+int(2.5)", "int(int(1.5))", "int(fix(1.5))", "fix(int(1.5))", "fix(fix(1.5))", "int(1.5,2)", "int()", "fix()", "sin()", "sin(1.0,2.0)", "pow(2.0)", "pow()", "pow(2.0,3.0,4.0)", "int(sin)", "int(pi)+pi", "pi", "pi+1", "int(PI)", "int(Pi)", "INT(1.5)", "Int(1.5)", "FIX(1.5)", "SIN(0.0)", "int(SIN(0.0))", "int(Sin(1.0))",
              "int(abs(-2.5))", "int(abs(2.5))", "int(abs(-pi))", "int(sqrt(2.0)*100.0)", "int(sqr(1.5)*100.0)", "int(ln(10.0)*100.0)", "int(log(2.0)*1000.0)", "int(exp(1.0)*100.0)", "int(exp(0.0))", "int(exp(-1.0)*100.0)", "int(sqrt(-1.0))", "int(ln(0.0))", "int(ln(-1.0))", "int(log(0.0))", "int(acos(2.0))", "int(asin(2.0))", "int(tan(pi/2))", "int(1.0/0.0)",
              "int(sin(pi/6)*1000.0)", "int(cos(pi/3)*1000.0)", "int(atan(1.0)*4.0*1000.0)", "int(1.0e-3)", "int(1.0E3)", "int(0.1+0.2)", "int(0.1*3.0*10.0)", "int(2.0*0.5)"]:
        c.append(t)
    # ---- $ e rotulos
    for t in ["$", "$+1", "$-1", "$*2", "lbl", "lbl+1", "lblf", "lblf-lbl", "lblf-$", "$-lbl", "lbl*2", "lbl&0ffh", "lbl>>8", "-lbl", "~lbl", "!lbl", "lbl==lbl", "lbl==lblf", "lbl<lblf", "undefined", "undefined+1", "int(lbl)", "fix(lbl)", "int(lbl*1.0)", "fix(lbl*1.0)"]:
        c.append(t)
    seen, out = set(), []
    for t in c:
        if t not in seen:
            seen.add(t)
            out.append(t)
    return out


def wrap(e):
    return "org 100h\nlbl: nop\ndw %s\nlblf: nop\n" % e


def main():
    cands = candidates()
    print("candidatos:", len(cands), file=sys.stderr)
    results = run_many([wrap(e) for e in cands], workers=16)
    cases = [("expr%04d" % i, wrap(e), r) for i, (e, r) in enumerate(zip(cands, results))]
    out = os.path.join(os.path.dirname(__file__), "..", "..", "tests", "asm", "golden", "expr.golden")
    write_golden(out, cases)
    ok = sum(1 for r in results if r["rc"] == 0)
    print("aceitos:", ok, "recusados:", len(results) - ok, file=sys.stderr)


if __name__ == "__main__":
    main()
