#!/usr/bin/env python3
"""Gera os assets de splash e de icone do fwMSX a partir de images/ (so' biblioteca padrao, sem PIL).

    python tools/assets/make_assets.py

Saidas:
  dist/fwMSX-splash.png                 splash reduzido a metade (o original tem 7 MB); vai ao lado do executavel
  src/splash/app_icon_data.h            icone da janela em 16, 32, 48 e 64 pixels (RGBA), para o GLFW (Linux)
  packaging/linux/fwmsx.png             icone de 256 pixels para o .desktop / gerenciador de arquivos do Linux

O .ico em si (images/fwMSX-03.ico) entra no executavel do Windows pelo recurso src/splash/fwmsx.rc.
"""
import os
import struct
import sys
import zlib

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def read_png(path):
    data = open(path, "rb").read()
    assert data[:8] == b"\x89PNG\r\n\x1a\n", "nao e' PNG"
    pos, idat, ihdr = 8, b"", None
    while pos < len(data):
        n, typ = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + n]
        if typ == b"IHDR":
            ihdr = struct.unpack(">IIBBBBB", body)
        elif typ == b"IDAT":
            idat += body
        pos += 12 + n
    w, h, depth, ctype, _, _, interlace = ihdr
    assert depth == 8 and interlace == 0 and ctype in (2, 6), "so' PNG 8 bits RGB/RGBA sem entrelacamento"
    bpp = 3 if ctype == 2 else 4
    raw = zlib.decompress(idat)
    stride = w * bpp
    out = bytearray(h * stride)
    prev = bytearray(stride)
    p = 0
    for y in range(h):
        ft = raw[p]
        row = bytearray(raw[p + 1:p + 1 + stride])
        p += 1 + stride
        if ft == 1:
            for i in range(bpp, stride):
                row[i] = (row[i] + row[i - bpp]) & 255
        elif ft == 2:
            for i in range(stride):
                row[i] = (row[i] + prev[i]) & 255
        elif ft == 3:
            for i in range(stride):
                left = row[i - bpp] if i >= bpp else 0
                row[i] = (row[i] + ((left + prev[i]) >> 1)) & 255
        elif ft == 4:
            for i in range(stride):
                a = row[i - bpp] if i >= bpp else 0
                b = prev[i]
                c = prev[i - bpp] if i >= bpp else 0
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 255
        out[y * stride:(y + 1) * stride] = row
        prev = row
    return w, h, bpp, bytes(out)


def write_png(path, w, h, bpp, pixels):
    ctype = 2 if bpp == 3 else 6
    stride = w * bpp
    raw = b"".join(b"\x00" + pixels[y * stride:(y + 1) * stride] for y in range(h))

    def chunk(t, d):
        c = struct.pack(">I", len(d)) + t + d
        return c + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, ctype, 0, 0, 0)) +
                chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def half(w, h, bpp, pixels):
    """Reduz a metade (media de 2x2)."""
    nw, nh = w // 2, h // 2
    stride = w * bpp
    out = bytearray(nw * nh * bpp)
    for y in range(nh):
        r0 = pixels[2 * y * stride:(2 * y + 1) * stride]
        r1 = pixels[(2 * y + 1) * stride:(2 * y + 2) * stride]
        orow = bytearray(nw * bpp)
        for c in range(bpp):
            a, b = r0[c::bpp], r1[c::bpp]
            orow[c::bpp] = bytes((a[2 * x] + a[2 * x + 1] + b[2 * x] + b[2 * x + 1] + 2) >> 2 for x in range(nw))
        out[y * nw * bpp:(y + 1) * nw * bpp] = orow
    return nw, nh, bpp, bytes(out)


def read_ico(path):
    """Devolve {tamanho: (w, h, RGBA)} das entradas BMP de 32 bits do .ico."""
    d = open(path, "rb").read()
    n = struct.unpack("<H", d[4:6])[0]
    icons = {}
    for k in range(n):
        w, h, _, _, _, bp, size, off = struct.unpack("<BBBBHHII", d[6 + 16 * k:22 + 16 * k])
        w, h = w or 256, h or 256
        blob = d[off:off + size]
        if blob[:8] == b"\x89PNG\r\n\x1a\n":
            pw, ph, bpp, px = read_png_bytes(blob)
            rgba = px if bpp == 4 else b"".join(px[i:i + 3] + b"\xff" for i in range(0, len(px), 3))
            icons[w] = (w, h, rgba)
            continue
        hdr = struct.unpack("<IiiHHIIiiII", blob[:40])
        assert hdr[4] == 32, "so' icones BMP de 32 bits"
        pix = blob[40:40 + w * h * 4]
        rgba = bytearray(w * h * 4)
        for y in range(h):  # BMP: de baixo para cima, BGRA
            src = pix[(h - 1 - y) * w * 4:(h - y) * w * 4]
            for x in range(w):
                b, g, r, a = src[4 * x:4 * x + 4]
                rgba[(y * w + x) * 4:(y * w + x) * 4 + 4] = bytes((r, g, b, a))
        icons[w] = (w, h, bytes(rgba))
    return icons


def read_png_bytes(blob):
    import tempfile
    with tempfile.NamedTemporaryFile(delete=False, suffix=".png") as t:
        t.write(blob)
    try:
        w, h, bpp, px = read_png(t.name)
    finally:
        os.remove(t.name)
    return w, h, bpp, px


def main():
    splash_src = os.path.join(ROOT, "images", "fwMSX-02.png")
    w, h, bpp, px = read_png(splash_src)
    print("splash original: %dx%d" % (w, h))
    w2, h2, bpp2, px2 = half(w, h, bpp, px)
    out = os.path.join(ROOT, "dist", "fwMSX-splash.png")
    write_png(out, w2, h2, bpp2, px2)
    print("gravado %s (%dx%d, %d bytes)" % (out, w2, h2, os.path.getsize(out)))

    icons = read_ico(os.path.join(ROOT, "images", "fwMSX-03.ico"))
    print("tamanhos do .ico:", sorted(icons))
    os.makedirs(os.path.join(ROOT, "packaging", "linux"), exist_ok=True)
    big = icons[256]
    write_png(os.path.join(ROOT, "packaging", "linux", "fwmsx.png"), 256, 256, 4, big[2])
    print("gravado packaging/linux/fwmsx.png (256x256)")

    lines = ["// GERADO por tools/assets/make_assets.py a partir de images/fwMSX-03.ico -- nao editar a mao.",
             "// Icone da janela (RGBA, 4 bytes por pixel) em varios tamanhos, para glfwSetWindowIcon (Linux; no Windows o",
             "// icone vem do recurso src/splash/fwmsx.rc).",
             "#pragma once", "", "#include <cstdint>", "", "namespace splash {", ""]
    sizes = [16, 32, 48, 64]
    for s in sizes:
        rgba = icons[s][2]
        lines.append("static const uint8_t kAppIcon%d[%d] = {" % (s, len(rgba)))
        for i in range(0, len(rgba), 24):
            lines.append("    " + ",".join(str(b) for b in rgba[i:i + 24]) + ",")
        lines.append("};")
        lines.append("")
    lines.append("struct AppIconImage {")
    lines.append("    int size;")
    lines.append("    const uint8_t *rgba;")
    lines.append("};")
    lines.append("static const AppIconImage kAppIcons[] = {")
    for s in sizes:
        lines.append("    {%d, kAppIcon%d}," % (s, s))
    lines.append("};")
    lines.append("")
    lines.append("} // namespace splash")
    path = os.path.join(ROOT, "src", "splash", "app_icon_data.h")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, "w", newline="\n").write("\n".join(lines) + "\n")
    print("gravado %s" % path)


if __name__ == "__main__":
    sys.exit(main())
