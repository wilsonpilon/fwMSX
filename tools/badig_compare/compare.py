#!/usr/bin/env python3
"""
Compara o clone do Basic Dignified do fwMSX (badigcli) com o badig.py ORIGINAL (resource/basic-dignified), byte a byte.

Uso:
    python tools/badig_compare/compare.py <badigcli.exe> <arquivo.dmx> [mais.dmx ...] [-- argumentos do badig]
    python tools/badig_compare/compare.py <badigcli.exe> --golden <pasta_destino> <arquivo.dmx> ...   (grava os arquivos de referencia)

Cada fonte e' copiado (com os .include ao lado) para duas pastas temporarias: uma roda o `python badig.py`, a outra o clone. Confere o .amx, o .bmx, o .lmx
(sem a linha "' Date:"), e os relatorios -lex -par -lnr -var -lbr. Termina com codigo 1 se algo difere.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
BADIG = os.path.join(ROOT, "resource", "basic-dignified")

REPORT_ARGS = ["-lex", "-par", "-lnr", "-var", "-lbr", "--tk_tokenize", "--tk_list", "16"]


def norm_lmx(data: bytes) -> bytes:
    return re.sub(rb"' Date: [^\r\n]*", b"' Date: -", data)


def run(cmd, cwd):
    p = subprocess.run(cmd, cwd=cwd, capture_output=True)
    return p.returncode, (p.stdout + p.stderr).decode("latin1", "replace")


def sanitize(data: bytes) -> bytes:
    # o exemplo do Change Graph Kit traz argumentos de uma versao mais nova (-of ta -el) que o badig.py 2.2.1 nao conhece
    def fix(m):
        line = m.group(0)
        line = re.sub(rb"\s-of\s+\S+", b"", line)
        line = re.sub(rb"\s-el(\s+\d+)?", b"", line)
        return line
    return re.sub(rb"(?im)^##BB:arguments=.*$", fix, data)


def prepare(src, dest):
    os.makedirs(dest, exist_ok=True)
    src_dir = os.path.dirname(os.path.abspath(src))
    for name in os.listdir(src_dir):
        p = os.path.join(src_dir, name)
        if os.path.isfile(p):
            shutil.copy(p, os.path.join(dest, name))
    # a copia do fonte principal e' limpa dos argumentos desconhecidos
    main = os.path.join(dest, os.path.basename(src))
    with open(main, "rb") as f:
        data = f.read()
    with open(main, "wb") as f:
        f.write(sanitize(data))


def collect(folder, stem):
    out = {}
    for name in sorted(os.listdir(folder)):
        low = name.lower()
        if not low.startswith(stem.lower()):
            continue
        if low.endswith((".amx", ".bmx", ".lmx", ".txt")):
            with open(os.path.join(folder, name), "rb") as f:
                data = f.read()
            if low.endswith(".lmx"):
                data = norm_lmx(data)
            out[name] = data
    return out


def main():
    args = sys.argv[1:]
    if len(args) < 2:
        print(__doc__)
        return 2
    cli = os.path.abspath(args.pop(0))
    golden = None
    if args[0] == "--golden":
        args.pop(0)
        golden = os.path.abspath(args.pop(0))
    extra = []
    if "--" in args:
        i = args.index("--")
        extra = args[i + 1:]
        args = args[:i]
    bad = 0
    for src in args:
        stem = os.path.splitext(os.path.basename(src))[0]
        with tempfile.TemporaryDirectory() as tmp:
            pa, pb = os.path.join(tmp, "py"), os.path.join(tmp, "cpp")
            prepare(src, pa)
            prepare(src, pb)
            # o python precisa do pacote de apoio ao lado do badig.py; roda de la
            rc_a, out_a = run([sys.executable, "-B", os.path.join(BADIG, "badig.py"), os.path.join(pa, os.path.basename(src))] + REPORT_ARGS + extra, pa)
            rc_b, out_b = run([cli, os.path.join(pb, os.path.basename(src))] + REPORT_ARGS + extra, pb)
            fa, fb = collect(pa, stem), collect(pb, stem)
            # o badig.py sai com 0 mesmo quando a conversao para com erro ("Conversion stopped.")
            fail_a = rc_a != 0 or "Conversion stopped." in out_a
            fail_b = rc_b != 0
            ok = fail_a == fail_b
            if fail_a != fail_b:
                print(f"  falha: python={fail_a} clone={fail_b}")
            for name in sorted(set(fa) | set(fb)):
                if name not in fb:
                    print(f"  falta no clone: {name}")
                    ok = False
                elif name not in fa:
                    if not name.endswith("_lines.txt") or True:
                        print(f"  a mais no clone: {name}")
                    ok = False
                elif fa[name] != fb[name]:
                    print(f"  DIFERE: {name} ({len(fa[name])} x {len(fb[name])} bytes)")
                    ok = False
            if fail_a and fail_b:
                # as duas falharam: compara a mensagem
                ma = [l.replace(pa + os.sep, "") for l in out_a.splitlines() if l.startswith("***") or l.startswith("    ")]
                mb = [l.replace(pb + os.sep, "").replace(pb + "/", "") for l in out_b.splitlines() if l.startswith("***") or l.startswith("    ")]
                if ma != mb:
                    print("  mensagens de erro diferem:\n    python:", ma, "\n    clone :", mb)
                    ok = False
            print(("OK   " if ok else "FALHA"), src)
            if not ok:
                bad += 1
            if golden and ok:
                dst = os.path.join(golden, stem)
                os.makedirs(dst, exist_ok=True)
                if fail_a:
                    # so' a mensagem de erro (sem o caminho temporario)
                    ma = [l.replace(pa + os.sep, "") for l in out_a.splitlines() if l.startswith("***") or l.startswith("    ")]
                    with open(os.path.join(dst, "golden_" + stem + ".err"), "w", encoding="latin1", newline="") as f:
                        f.write("\n".join(ma) + "\n")
                for name, data in fa.items():
                    with open(os.path.join(dst, "golden_" + name), "wb") as f:
                        f.write(data)
                if extra:
                    with open(os.path.join(dst, "args.txt"), "w") as f:
                        f.write(" ".join(extra))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
