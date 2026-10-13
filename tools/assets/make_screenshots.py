#!/usr/bin/env python3
"""Gera as capturas de tela do README (images/fwMSX-04.png em diante) a partir do proprio emulador.

    python tools/assets/make_screenshots.py          (a partir da raiz do repositorio, depois do build)

Nao fotografa a area de trabalho: usa a captura do proprio emulador (`--shot`, sem janela) e, para o depurador, o comando
`dbgui` da ponte de controle, desenhado com a FONTE DO MSX tirada da BIOS (a mesma tabela que a janela usa).

  04  MSX BASIC 3.0 (MSX2+) com um programinha rodando
  05  jogo de MSX1 (Gradius / Nemesis)
  06  jogo de MSX2 (Vampire Killer)
  07  Z80 Debugger: desmontagem, parado num breakpoint do jogo
  08  Z80 Debugger: dump de memoria com os caracteres do MSX

Precisa de dist/fwMSXc.exe, das BIOS em dist/roms/bios, de resource/fMSX/MSX2PEXT.ROM, MSX2EXT.ROM e dos jogos em tests/media
(provisorios). Os jogos NAO sao distribuidos; so' as imagens geradas ficam em images/.
"""
import os
import socket
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import make_assets as ma  # leitura/escrita de PNG sem dependencias

EXE = os.path.join(ROOT, "dist", "fwMSXc.exe" if os.name == "nt" else "fwMSX")
BIOS = os.path.join(ROOT, "dist", "roms", "bios")
EXT = os.path.join(ROOT, "resource", "fMSX")
MEDIA = os.path.join(ROOT, "tests", "media")
OUT = os.path.join(ROOT, "images")
TMP = os.environ.get("TEMP", "/tmp")


def media(prefix):
    for f in sorted(os.listdir(MEDIA)):
        if f.startswith(prefix) and f.lower().endswith(".rom"):
            return os.path.join(MEDIA, f)
    raise SystemExit("jogo nao encontrado em tests/media: " + prefix)


def read_ppm(path):
    data = open(path, "rb").read()
    parts, pos = [], 0
    while len(parts) < 4:
        while data[pos:pos + 1].isspace():
            pos += 1
        s = pos
        while not data[pos:pos + 1].isspace():
            pos += 1
        parts.append(data[s:pos])
    pos += 1
    w, h = int(parts[1]), int(parts[2])
    return w, h, data[pos:pos + w * h * 3]


def scale(w, h, rgb, factor):
    """Vizinho mais proximo (factor pode ser 1.5: as telas MSX2 ja' saem com os pixels dobrados)."""
    nw, nh = int(w * factor), int(h * factor)
    out = bytearray(nw * nh * 3)
    for y in range(nh):
        sy = int(y / factor)
        row = bytearray()
        for x in range(nw):
            sx = int(x / factor)
            row += rgb[(sy * w + sx) * 3:(sy * w + sx) * 3 + 3]
        out[y * nw * 3:(y + 1) * nw * 3] = row
    return nw, nh, bytes(out)


def shot(name, args, factor):
    ppm = os.path.join(TMP, "fwmsx_%s.ppm" % name)
    cmd = [EXE, "--msx", "--mute", "--no-splash", "--shot", ppm] + args
    subprocess.run(cmd, check=True, capture_output=True, timeout=300)
    w, h, rgb = read_ppm(ppm)
    nw, nh, px = scale(w, h, rgb, factor)
    path = os.path.join(OUT, name)
    ma.write_png(path, nw, nh, 3, px)
    os.remove(ppm)
    print("gravado %s (%dx%d)" % (path, nw, nh))


# ---- Z80 Debugger: o painel como texto (comando dbgui) desenhado com a fonte do MSX ----------------------------------
PALETTE = {  # estilo: (frente, fundo) -- as mesmas cores de src/debug/cpp/debug_view.cpp
    "N": (0xE0E0E0, 0x2020E0), "T": (0x2020E0, 0xE0E0E0), "R": (0xE0E060, 0x2020E0), "C": (0x000000, 0x40C0E0),
    "P": (0x000000, 0xE0E060), "D": (0x9090FF, 0x2020E0),
}


def load_font():
    rom = open(os.path.join(BIOS, "MSX.ROM"), "rb").read()
    p = rom[4] | (rom[5] << 8)  # CGTABL
    return [rom[p + c * 8:p + c * 8 + 8] for c in range(256)]


def render_panel(lines, font, memory_view):
    """lines: 32 linhas "<estilo><44 caracteres>". No dump, os 8 caracteres vem dos bytes (para mostrar os do MSX)."""
    cols, rows = 44, 32
    img = bytearray(cols * 6 * rows * 8 * 3)
    for cy, line in enumerate(lines[:rows]):
        style, text = line[0], line[1:].ljust(cols)[:cols]
        codes = [ord(c) for c in text]
        if memory_view and cy >= 6 and len(text) > 7 and text[5] == ":" and text[7:9].strip():
            try:
                for k in range(8):
                    codes[32 + k] = int(text[7 + k * 3:9 + k * 3], 16)
            except ValueError:
                pass
        for cx, code in enumerate(codes):
            fg, bg = PALETTE.get(style, PALETTE["N"])
            for r in range(8):
                bits = font[code & 255][r]
                for b in range(6):
                    c = fg if bits & (0x80 >> b) else bg
                    o = (((cy * 8 + r) * cols * 6) + cx * 6 + b) * 3
                    img[o:o + 3] = bytes(((c >> 16) & 255, (c >> 8) & 255, c & 255))
    return cols * 6, rows * 8, bytes(img)


class Bridge:
    def __init__(self, port):
        self.s = socket.create_connection(("127.0.0.1", port), timeout=10)
        self.f = self.s.makefile("rb")

    def cmd(self, line):
        self.s.sendall((line + "\n").encode())
        while True:
            head = self.f.readline().decode("latin-1").rstrip("\r\n")
            if head.startswith("! "):
                continue  # evento assincrono
            if head.startswith("ok +"):
                return [self.f.readline().decode("latin-1").rstrip("\r\n") for _ in range(int(head[4:]))]
            return [head]


def debugger_shots(font):
    cart = media("MSX1_Gradius")
    data = open(cart, "rb").read()
    init = data[2] | (data[3] << 8)  # endereco de INIT do cabecalho "AB": a BIOS chama aqui ao ligar
    port = 7811
    emu = subprocess.Popen([EXE, "--msx", "--bios", os.path.join(BIOS, "MSX.ROM"), "--cart", cart, "--ctl-port", str(port),
                            "--break", "0x%04X" % init, "--no-splash", "--mute"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        br = None
        for _ in range(60):
            try:
                br = Bridge(port)
                break
            except OSError:
                time.sleep(0.25)
        if br is None:
            raise SystemExit("nao consegui conectar na ponte de controle")
        for _ in range(80):  # espera o breakpoint no INIT do jogo
            if "stopped=1" in br.cmd("status")[0]:
                break
            time.sleep(0.25)
        br.cmd("dbgui open")
        for _ in range(4):
            br.cmd("dbgui key enter")   # anda algumas instrucoes dentro do codigo do jogo
        br.cmd("dbgui key down")
        br.cmd("dbgui key down")
        lines = br.cmd("dbgui key b")    # breakpoint (*) no cursor; a linha do cursor fica em destaque
        w, h, px = render_panel(lines[1:], font, False)
        w, h, px = scale(w, h, px, 3)
        ma.write_png(os.path.join(OUT, "fwMSX-07.png"), w, h, 3, px)
        print("gravado images/fwMSX-07.png (%dx%d)" % (w, h))

        # dump de memoria: a tabela de caracteres da propria BIOS (CGTABL) -- os bytes viram os graficos do MSX
        br.cmd("dbgui key m")
        br.cmd("dbgui key g")
        cg = open(os.path.join(BIOS, "MSX.ROM"), "rb").read()
        addr = cg[4] | (cg[5] << 8)
        for ch in "%04X" % (addr + 0x41 * 8):
            br.cmd("dbgui char " + ch)
        lines = br.cmd("dbgui key enter")
        w, h, px = render_panel(lines[1:], font, True)
        w, h, px = scale(w, h, px, 3)
        ma.write_png(os.path.join(OUT, "fwMSX-08.png"), w, h, 3, px)
        print("gravado images/fwMSX-08.png (%dx%d)" % (w, h))
        br.cmd("quit")
    finally:
        time.sleep(0.5)
        emu.kill()


def main():
    os.makedirs(OUT, exist_ok=True)
    # 04: MSX BASIC 3.0 -- digita alguns comandos
    # Nada de pontuacao: o que o --keys digita segue o teclado INTERNACIONAL e a BIOS do fMSX e' japonesa (shift+9 e' '(' no
    # internacional e ')' no japones; '=' sai '^'...). So' letras e digitos aqui.
    prog = 'KEY LIST|PRINT TIME|PRINT 1234|'
    shot("fwMSX-04.png", ["--msx2p", "--bios", os.path.join(BIOS, "MSX2P.ROM"), "--ext", os.path.join(EXT, "MSX2PEXT.ROM"), "--frames", "400",
                          "--keys", prog, "--wait", "60"], 1.5)
    # 05: jogo de MSX1
    shot("fwMSX-05.png", ["--bios", os.path.join(BIOS, "MSX.ROM"), "--cart", media("MSX1_Gradius"), "--frames", "700"], 3)
    # 06: jogo de MSX2
    shot("fwMSX-06.png", ["--msx2", "--bios", os.path.join(BIOS, "MSX2.ROM"), "--ext", os.path.join(EXT, "MSX2EXT.ROM"), "--cart",
                          media("MSX2_Vampire"), "--frames", "2000"], 1.5)
    debugger_shots(load_font())


if __name__ == "__main__":
    main()
