#!/usr/bin/env python3
"""Compara quadros do fwMSX com os do fmsxgo (port em Go do fMSX) para uma lista de casos.

Uso (a partir da raiz do repo):
    python tools/fmsxgo_ref/compare.py [--fmsxgo E:/fmsxgo] [--fw dist/fwMSXc.exe] [--png saida/] [--min 0.98] [caso ...]

O gerador de referencia (main.go) e' compilado uma vez para um diretorio temporario; o fmsxgo entra por
"replace" no go.mod e NAO e' alterado. Duas metricas por caso, na janela comum depois de alinhar os quadros (as bordas dos dois emuladores diferem):
  exato  = fracao de pixels identicos (sensivel a diferenca de paleta/cor);
  forma  = fracao de pixels iguais depois de mapear cada cor da referencia para a cor mais comum do fwMSX naquela
           regiao (ignora paletas diferentes e olha so' a estrutura da imagem).
Um caso passa se algum dos seus pontos (quadros) atingir --min na metrica "forma"; sai com codigo 1 se algum caso falhar.
"""
import argparse, collections, os, shutil, struct, subprocess, sys, tempfile, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))

# nome, modelo (0/1/2), rom (relativo a --fmsxgo/media) ou None, pontos de comparacao (quadros).
# Os dois emuladores nao ficam sincronizados ao quadro (fades, animacoes de abertura): o caso passa se ALGUM
# ponto bater, e todos os pontos sao listados.
CASES = [
    ("bios_msx1", 0, None, (400, 500)),
    ("bios_msx2", 1, None, (400, 500)),
    ("bios_msx2p", 2, None, (400, 500)),
    ("kings_valley", 0, "King's Valley 1 - Konami (1985) [a1] [RC-727] [GoodMSX] [1207].rom", (500, 600, 700)),
    ("f1_spirit", 0, "F1 Spirit - The Way To Formula 1 - Konami (1987) [a1] [RC-752] [GoodMSX] [666].rom", (500, 600, 700)),
    ("lode_runner_scc", 0, "Lode_Runner_+_Konami_SCC.rom", (500, 600, 700)),
    ("firebird_msx2", 1, "Firebird - Hi no Tori Hououhen - Konami (1987) [a1] [RC-747] [GoodMSX] [989].rom", (500, 600, 800)),
]


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    parts, pos = [], 0
    while len(parts) < 4:  # P6 w h maxval
        while data[pos:pos + 1].isspace():
            pos += 1
        s = pos
        while not data[pos:pos + 1].isspace():
            pos += 1
        parts.append(data[s:pos])
    pos += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[pos:pos + w * h * 3]


def halve(img):
    """O fwMSX grava as telas MSX2 em dobro (544x456, ou 512 de largura em SCREEN 6/7): reduz para a escala 1x."""
    w, h, d = img
    if w < 400:
        return img
    ys = 2 if h >= 400 else 1
    rows = []
    for y in range(0, h, ys):
        row = d[y * w * 3:(y + 1) * w * 3]
        rows.append(b"".join(row[x * 6:x * 6 + 3] for x in range(w // 2)))
    return w // 2, len(rows), b"".join(rows)


def overlap(ref, fw, dx, dy):
    """Janela comum quando a origem do fw cai em (dx, dy) dentro da ref."""
    x0, y0 = max(0, dx), max(0, dy)
    x1, y1 = min(ref[0], dx + fw[0]), min(ref[1], dy + fw[1])
    return x0, y0, x1, y1


def score_shape(ref, fw, dx, dy, step=1):
    x0, y0, x1, y1 = overlap(ref, fw, dx, dy)
    if x1 - x0 < 64 or y1 - y0 < 64:
        return 0.0, 0.0, 0
    votes = collections.defaultdict(collections.Counter)
    exact = n = 0
    for y in range(y0, y1, step):
        rr, fr = y * ref[0] * 3, (y - dy) * fw[0] * 3
        for x in range(x0, x1, step):
            r = ref[2][rr + x * 3:rr + x * 3 + 3]
            f = fw[2][fr + (x - dx) * 3:fr + (x - dx) * 3 + 3]
            votes[r][f] += 1
            exact += r == f
            n += 1
    score_shape.last_map = {r: c.most_common(1)[0][0] for r, c in votes.items() if r != c.most_common(1)[0][0]}
    return exact / n, sum(c.most_common(1)[0][1] for c in votes.values()) / n, n


def align(ref, fw):
    """Acha o deslocamento (dx, dy) do fw dentro da ref que maximiza a metrica 'forma' (amostrada)."""
    best = (-1.0, 0, 0)
    for dy in range(-6, 19):
        for dx in range(-8, 25):
            s = score_shape(ref, fw, dx, dy, step=4)[1]
            if s > best[0]:
                best = (s, dx, dy)
    return best[1], best[2]


def metrics(ref, fw):
    dx, dy = align(ref, fw)
    exact, shape, _ = score_shape(ref, fw, dx, dy)
    return exact, shape, (dx, dy)


def write_png(path, w, h, rgb):
    raw = b"".join(b"\0" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))
    def chunk(t, d):
        c = struct.pack(">I", len(d)) + t + d
        return c + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) +
                chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def side_by_side(path, ref, fw, off):
    x0, y0, x1, y1 = overlap(ref, fw, *off)
    w, h = x1 - x0, y1 - y0
    rows = []
    for y in range(y0, y1):
        a = ref[2][(y * ref[0] + x0) * 3:(y * ref[0] + x1) * 3]
        b = fw[2][((y - off[1]) * fw[0] + x0 - off[0]) * 3:((y - off[1]) * fw[0] + x1 - off[0]) * 3]
        d = bytearray(w * 3)
        for x in range(w):
            if a[x * 3:x * 3 + 3] != b[x * 3:x * 3 + 3]:
                d[x * 3] = 255
        rows.append(a + b + bytes(d))
    write_png(path, w * 3, h, b"".join(rows))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fmsxgo", default="E:/fmsxgo")
    ap.add_argument("--fw", default=os.path.join(ROOT, "dist", "fwMSXc.exe"))
    ap.add_argument("--png", help="pasta para gravar referencia | fwMSX | diferenca lado a lado")
    ap.add_argument("--min", type=float, default=0.98, help="piso da metrica 'forma' (padrao 0.98)")
    ap.add_argument("--cores", action="store_true", help="lista as cores da referencia que o fwMSX desenha diferente")
    ap.add_argument("casos", nargs="*")
    a = ap.parse_args()

    tmp = tempfile.mkdtemp(prefix="fmsxref_")
    try:
        ref_exe = os.path.join(tmp, "fmsxref.exe" if os.name == "nt" else "fmsxref")
        # copia o gerador para o tmp para o "replace" apontar para o --fmsxgo escolhido
        src = os.path.join(tmp, "src")
        shutil.copytree(HERE, src, ignore=shutil.ignore_patterns("*.py", "__pycache__"))
        gm = os.path.join(src, "go.mod")
        txt = open(gm).read().replace("E:/fmsxgo", a.fmsxgo.replace("\\", "/"))
        open(gm, "w").write(txt)
        r = subprocess.run(["go", "build", "-o", ref_exe, "."], cwd=src, capture_output=True, text=True)
        if r.returncode:
            print("falha ao compilar o gerador de referencia:\n" + r.stderr)
            return 2

        roms = os.path.join(tmp, "roms")
        os.makedirs(roms)
        for n in ("MSX.ROM", "MSX2.ROM", "MSX2P.ROM"):
            shutil.copy(os.path.join(ROOT, "dist", "roms", "bios", n), roms)
        for n in ("MSX2EXT.ROM", "MSX2PEXT.ROM"):  # sem DISK.ROM: o fwMSX tambem sobe sem interface de disco
            shutil.copy(os.path.join(a.fmsxgo, "third-party", "fMSX", "ROMs", n), roms)

        if a.png:
            os.makedirs(a.png, exist_ok=True)
        bad = 0
        print(f"{'caso@quadros':<22}{'exato':>8}{'forma':>8}")
        for name, model, rom, points in CASES:
            if a.casos and name not in a.casos:
                continue
            cart = os.path.join(a.fmsxgo, "media", rom) if rom else None
            if cart and not os.path.exists(cart):
                print(f"{name:<18} (ROM nao encontrada, ignorado)")
                continue
            best = -1.0
            for frames in points:
                label = f"{name}@{frames}"
                ref_ppm, fw_ppm = os.path.join(tmp, label + "_ref.ppm"), os.path.join(tmp, label + "_fw.ppm")
                cmd = [ref_exe, "-model", str(model), "-roms", roms, "-frames", str(frames), "-out", ref_ppm]
                if cart:
                    cmd += ["-cart", cart]
                if subprocess.run(cmd, capture_output=True).returncode:
                    print(f"{label:<22} falha no fmsxgo")
                    continue
                bios = ["MSX.ROM", "MSX2.ROM", "MSX2P.ROM"][model]
                cmd = [a.fw, "--msx"] + (["--msx2"] if model == 1 else ["--msx2p"] if model == 2 else []) +                       ["--bios", os.path.join(ROOT, "dist", "roms", "bios", bios), "--frames", str(frames),
                       "--shot", fw_ppm, "--mute"]
                if model:
                    cmd += ["--ext", os.path.join(roms, "MSX2PEXT.ROM" if model == 2 else "MSX2EXT.ROM")]
                if cart:
                    cmd += ["--cart", cart]
                if subprocess.run(cmd, capture_output=True).returncode:
                    print(f"{label:<22} falha no fwMSX")
                    continue
                ref, fw = halve(read_ppm(ref_ppm)), halve(read_ppm(fw_ppm))
                exact, shape, off = metrics(ref, fw)
                best = max(best, shape)
                print(f"{label:<22}{exact * 100:7.2f}%{shape * 100:7.2f}%  desloc={off}")
                if a.cores:
                    score_shape(ref, fw, *off)
                    for r, f in sorted(score_shape.last_map.items()):
                        print("    fmsxgo #%02X%02X%02X  ->  fwMSX #%02X%02X%02X" % (*r, *f))
                if a.png:
                    side_by_side(os.path.join(a.png, label + ".png"), ref, fw, off)
            ok = best >= a.min
            bad += not ok
            print(f"  => {name}: {'OK' if ok else 'ABAIXO DO PISO'} (melhor 'forma' {best * 100:.2f}%)")
        return 1 if bad else 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
