#!/usr/bin/env python3
"""Compara quadros do fwMSX com os de um emulador de referencia, para uma lista de casos.

Referencias (--ref):
  fmsx    (padrao) o fMSX ORIGINAL de resource/fMSX, compilado sem janela (tools/fmsx_ref/Headless.c no lugar do
          Unix.c; o nucleo nao e' alterado). Precisa de gcc no PATH.
  fmsxgo  o port em Go (E:/fmsxgo, ainda em desenvolvimento -- so' como segunda opiniao). Precisa de Go.

Uso (a partir da raiz do repo):
    python tools/fmsxgo_ref/compare.py [--ref fmsx|fmsxgo] [--fw dist/fwMSXc.exe] [--png saida/] [--cores]
                                       [--min 0.98] [--fmsxgo E:/fmsxgo] [caso ...]

Duas metricas por caso, na janela comum depois de alinhar os quadros (as bordas dos emuladores podem diferir):
  exato  = fracao de pixels identicos;
  forma  = fracao de pixels iguais depois de casar cada cor da referencia com UMA cor do fwMSX (correspondencia
           um-para-um pelas maiores contagens): ignora paletas diferentes, mas duas cores fundidas contam como erro.
  detalhe = a mesma coisa so' nos pixels que nao sao o fundo dominante de nenhuma das imagens (uma tela quase
           uniforme nao engana a metrica: ex. prompt do BASIC contra um jogo parado).
Um caso passa se algum dos seus pontos (quadros) atingir --min em min(forma, detalhe); sai com codigo 1 se algum falhar.
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


FMSX_SRC = os.path.join(ROOT, "resource", "fMSX")
FMSX_HEADLESS = os.path.join(ROOT, "tools", "fmsx_ref", "Headless.c")
# fonte do nucleo original (resource/fMSX, sem alteracao) + a camada de plataforma sem janela (Headless.c)
FMSX_FILES = ["fMSX/fMSX.c", "fMSX/MSX.c", "fMSX/V9938.c", "fMSX/I8251.c", "fMSX/Patch.c", "Z80/Z80.c",
              "EMULib/I8255.c", "EMULib/AY8910.c", "EMULib/YM2413.c", "EMULib/SCC.c", "EMULib/WD1793.c",
              "EMULib/FDIDisk.c", "EMULib/SHA1.c", "EMULib/MCF.c", "EMULib/Hunt.c", "EMULib/Floppy.c"]


def build_fmsx(tmp):
    """Compila o fMSX original sem janela (precisa de gcc); devolve o executavel ou None."""
    exe = os.path.join(tmp, "fmsx_ref.exe" if os.name == "nt" else "fmsx_ref")
    base = ["gcc", "-c", "-O2", "-w", "-DFMSX", "-DLSB_FIRST", "-DDEBUG", "-I", FMSX_SRC + "/fMSX", "-I", FMSX_SRC + "/EMULib",
            "-I", FMSX_SRC + "/Z80"]
    objs = []
    for f in FMSX_FILES + [FMSX_HEADLESS]:
        src = f if os.path.isabs(f) else os.path.join(FMSX_SRC, f)
        o = os.path.join(tmp, os.path.basename(f) + ".o")
        extra = ["-Dmain=fmsx_main"] if f.endswith("fMSX/fMSX.c") else ["-DUNIX"] if f.endswith("Floppy.c") else []
        r = subprocess.run(base + extra + [src, "-o", o], capture_output=True, text=True)
        if r.returncode:
            print("falha ao compilar %s:\n%s" % (f, r.stderr[:2000]))
            return None
        objs.append(o)
    r = subprocess.run(["gcc", "-o", exe] + objs + ["-lz"], capture_output=True, text=True)
    if r.returncode:
        print("falha ao ligar o fMSX:\n" + r.stderr[:2000])
        return None
    return exe


# Jogos provisorios copiados do acervo do file-hunter para tests/media (sairao do repositorio na versao final):
# "MSX1_<nome>.rom" roda como MSX1, "MSX2_<nome>.rom" como MSX2. Cada um vira um caso com 3 pontos de comparacao.
MEDIA_DIR = os.path.join(ROOT, "tests", "media")


def media_cases():
    out = []
    if os.path.isdir(MEDIA_DIR):
        for f in sorted(os.listdir(MEDIA_DIR)):
            if f.lower().endswith(".rom") and f[:5] in ("MSX1_", "MSX2_"):
                name = "".join(ch if ch.isalnum() else "_" for ch in f[5:].split(" - ")[0]).strip("_").lower()
                out.append((name, 0 if f.startswith("MSX1_") else 1, os.path.join(MEDIA_DIR, f), (600, 900, 1200)))
    return out


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


def greedy_match(votes):
    """Casa cada cor da referencia com UMA cor do fwMSX (e vice-versa), pelas maiores contagens: devolve
    (soma dos pixels casados, {cor ref: cor fw})."""
    pairs = sorted(((n, r, f) for r, cnt in votes.items() for f, n in cnt.items()), key=lambda t: -t[0])
    used_r, used_f, total, mapping = set(), set(), 0, {}
    for n, r, f in pairs:
        if r in used_r or f in used_f:
            continue
        used_r.add(r)
        used_f.add(f)
        mapping[r] = f
        total += n
    return total, mapping


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
    matched, mapping = greedy_match(votes)
    score_shape.last_map = {r: f for r, f in mapping.items() if r != f}
    return exact / n, matched / n, n


def score_detail(ref, fw, dx, dy):
    """Como 'forma', mas so' nos pixels que nao sao o fundo dominante de NENHUMA das duas imagens: telas quase
    uniformes (prompt do BASIC contra um jogo parado) nao enganam. Sem pixels de detalhe, vale 1.0."""
    x0, y0, x1, y1 = overlap(ref, fw, dx, dy)
    cnt_r, cnt_f, pairs = collections.Counter(), collections.Counter(), []
    for y in range(y0, y1):
        rr, fr = y * ref[0] * 3, (y - dy) * fw[0] * 3
        for x in range(x0, x1):
            r = ref[2][rr + x * 3:rr + x * 3 + 3]
            f = fw[2][fr + (x - dx) * 3:fr + (x - dx) * 3 + 3]
            cnt_r[r] += 1
            cnt_f[f] += 1
            pairs.append((r, f))
    if not pairs:
        return 0.0
    dom_r, dom_f = cnt_r.most_common(1)[0][0], cnt_f.most_common(1)[0][0]
    votes = collections.defaultdict(collections.Counter)
    n = 0
    for r, f in pairs:
        if r != dom_r or f != dom_f:
            votes[r][f] += 1
            n += 1
    if n < 50:
        return 1.0
    return greedy_match(votes)[0] / n


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
    ap.add_argument("--ref", choices=("fmsx", "fmsxgo"), default="fmsx",
                    help="referencia: fmsx (o fMSX original, compilado sem janela; padrao) ou fmsxgo (port em Go)")
    ap.add_argument("--fw", default=os.path.join(ROOT, "dist", "fwMSXc.exe"))
    ap.add_argument("--png", help="pasta para gravar referencia | fwMSX | diferenca lado a lado")
    ap.add_argument("--min", type=float, default=0.98, help="piso da metrica 'forma' (padrao 0.98)")
    ap.add_argument("--cores", action="store_true", help="lista as cores da referencia que o fwMSX desenha diferente")
    ap.add_argument("casos", nargs="*")
    a = ap.parse_args()

    tmp = tempfile.mkdtemp(prefix="fmsxref_")
    try:
        roms = os.path.join(tmp, "roms")
        os.makedirs(roms)
        for n in ("MSX.ROM", "MSX2.ROM", "MSX2P.ROM"):
            shutil.copy(os.path.join(ROOT, "dist", "roms", "bios", n), roms)
        if a.ref == "fmsx":
            # fMSX original: as sub-ROMs do proprio fMSX; sem DISK.ROM/FMPAC.ROM (o fwMSX sobe sem disco)
            for n in ("MSX2EXT.ROM", "MSX2PEXT.ROM", "CARTS.SHA"):  # CARTS.SHA: mesma fonte de mapper do fwMSX
                shutil.copy(os.path.join(FMSX_SRC, n), roms)
            ref_exe = build_fmsx(tmp)
            if not ref_exe:
                return 2
        else:
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
            for n in ("MSX2EXT.ROM", "MSX2PEXT.ROM"):  # sem DISK.ROM: o fwMSX tambem sobe sem interface de disco
                shutil.copy(os.path.join(a.fmsxgo, "third-party", "fMSX", "ROMs", n), roms)

        if a.png:
            os.makedirs(a.png, exist_ok=True)
        bad = 0
        print(f"{'caso@quadros':<22}{'exato':>8}{'forma':>8}{'detalhe':>9}")
        for name, model, rom, points in CASES + media_cases():
            if a.casos and name not in a.casos:
                continue
            cart = (rom if os.path.isabs(rom) else os.path.join(a.fmsxgo, "media", rom)) if rom else None
            if cart and not os.path.exists(cart):
                print(f"{name:<18} (ROM nao encontrada, ignorado)")
                continue
            best = -1.0
            for frames in points:
                label = f"{name}@{frames}"
                ref_ppm, fw_ppm = os.path.join(tmp, label + "_ref.ppm"), os.path.join(tmp, label + "_fw.ppm")
                if a.ref == "fmsx":
                    # o fMSX grava CMOS.ROM ao sair e le na proxima partida: sem apagar, um caso herda a
                    # configuracao da BIOS do caso anterior (ja' enganou uma medicao de boot do MSX2+)
                    for d in (tmp, roms):
                        if os.path.exists(os.path.join(d, "CMOS.ROM")):
                            os.remove(os.path.join(d, "CMOS.ROM"))
                    cmd = [ref_exe, ["-msx1", "-msx2", "-msx2+"][model], "-home", roms, "-verbose", "0",
                           "-frames", str(frames), "-out", ref_ppm]
                    if cart:
                        cmd += [cart]
                else:
                    cmd = [ref_exe, "-model", str(model), "-roms", roms, "-frames", str(frames), "-out", ref_ppm]
                    if cart:
                        cmd += ["-cart", cart]
                if subprocess.run(cmd, capture_output=True, cwd=tmp).returncode:
                    print(f"{label:<22} falha na referencia ({a.ref})")
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
                detail = score_detail(ref, fw, *off)
                best = max(best, min(shape, detail))
                print(f"{label:<22}{exact * 100:7.2f}%{shape * 100:7.2f}%{detail * 100:8.2f}%  desloc={off}")
                if a.cores:
                    score_shape(ref, fw, *off)
                    for r, f in sorted(score_shape.last_map.items()):
                        print("    fmsxgo #%02X%02X%02X  ->  fwMSX #%02X%02X%02X" % (*r, *f))
                if a.png:
                    side_by_side(os.path.join(a.png, label + ".png"), ref, fw, off)
            ok = best >= a.min
            bad += not ok
            print(f"  => {name}: {'OK' if ok else 'ABAIXO DO PISO'} (melhor min(forma, detalhe) {best * 100:.2f}%)")
        return 1 if bad else 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
