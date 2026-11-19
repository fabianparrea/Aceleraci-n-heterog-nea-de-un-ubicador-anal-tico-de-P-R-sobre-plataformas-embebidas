#!/usr/bin/env python3
"""Arma un GIF con los cuadros que deja el placer con --frames.

uso: python3 tests/make_gif.py carpeta_de_cuadros salida.gif [--fps 10] [--skip 1]

Cada cuadro lleva abajo el circuito, la iteracion, el HPWL y el overflow, y
una leyenda de colores. Necesita Pillow (pip install pillow).
"""
import argparse
import csv
import os
import sys

from PIL import Image, ImageDraw, ImageFont

# Pillow 9.1 movio estas constantes a Image.Quantize e Image.Dither; asi
# tambien anda con la 8.4 de la Jetson
MEDIANCUT = getattr(getattr(Image, "Quantize", Image), "MEDIANCUT")
NO_DITHER = getattr(getattr(Image, "Dither", Image), "NONE")

# los mismos colores que usa viz.c
BG = (18, 20, 26)
BLUE = (40, 160, 220)
RED = (120, 44, 50)
HOT = (255, 214, 80)
TEXT = (230, 232, 236)
DIM = (150, 155, 165)


# sin FreeType (como el Pillow de la Jetson) queda la letra por defecto
def load_font(size):
    for name in ("DejaVuSans.ttf", "LiberationSans-Regular.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except (OSError, ImportError):
            pass
    try:
        return ImageFont.load_default(size=size)
    except (TypeError, ImportError):
        return ImageFont.load_default()


def text_width(draw, text, font):
    try:
        return draw.textlength(text, font=font)
    except (AttributeError, ValueError):
        return draw.textsize(text, font=font)[0]


def caption(row):
    parts = [row["circuito"]]
    if row["etapa"] == "legalizado":
        parts.append("legalizado")
    else:
        parts.append("iteración %s" % row["iteracion"])
    parts.append("HPWL %.3e" % float(row["hpwl"]))
    if row["overflow"]:
        parts.append("overflow %.3f" % float(row["overflow"]))
    return "    ".join(parts)


def add_footer(img, row, font, small):
    w, h = img.size
    footer = 58
    out = Image.new("RGB", (w, h + footer), BG)
    out.paste(img, (0, 0))
    d = ImageDraw.Draw(out)
    d.text((12, h + 8), caption(row), font=font, fill=TEXT)
    x = 12
    y = h + 34
    for color, label in ((BLUE, "celdas movibles"), (RED, "celdas fijas"),
                         (HOT, "más celdas que espacio")):
        d.rectangle([x, y + 2, x + 11, y + 13], fill=color)
        d.text((x + 17, y), label, font=small, fill=DIM)
        x += 17 + int(text_width(d, label, small)) + 22
    return out


def main():
    ap = argparse.ArgumentParser(description="GIF del colocado a partir de los cuadros")
    ap.add_argument("frames", help="carpeta que se le paso al placer con --frames")
    ap.add_argument("out", help="archivo .gif de salida")
    ap.add_argument("--fps", type=float, default=10.0)
    ap.add_argument("--skip", type=int, default=1,
                    help="usar uno de cada N cuadros del ciclo (el primero y el legalizado siempre van)")
    args = ap.parse_args()

    with open(os.path.join(args.frames, "cuadros.csv"), newline="") as f:
        rows = list(csv.DictReader(f))
    if not rows:
        sys.exit("no hay cuadros en " + args.frames)

    keep = [r for i, r in enumerate(rows)
            if i == 0 or r["etapa"] == "legalizado" or i % args.skip == 0]

    font, small = load_font(16), load_font(13)
    frames = []
    for r in keep:
        path = os.path.join(args.frames, "cuadro_%04d.ppm" % int(r["cuadro"]))
        frames.append(add_footer(Image.open(path).convert("RGB"), r, font, small))

    # una sola paleta para todos los cuadros, sacada de varios repartidos en
    # toda la corrida, para que los colores no salten de un cuadro al otro.
    # Los colores de la leyenda van aparte: en un circuito sin fijas dentro
    # del chip el rojo casi no aparece y se perderia al reducir la paleta.
    sample = frames[:: max(1, len(frames) // 8)] + [frames[-1]]
    w, h = frames[0].size
    strip = Image.new("RGB", (w, h * len(sample)))
    for k, im in enumerate(sample):
        strip.paste(im, (0, h * k))
    fixed = [BG, BLUE, RED, HOT, TEXT, DIM]
    reduced = strip.quantize(colors=64 - len(fixed), method=MEDIANCUT)
    colors = reduced.getpalette()[:3 * (64 - len(fixed))]
    colors += [0] * (3 * (64 - len(fixed)) - len(colors))
    for c in fixed:
        colors += list(c)
    palette = Image.new("P", (1, 1))
    palette.putpalette(colors + [0] * (768 - len(colors)))
    paletted = [im.quantize(palette=palette, dither=NO_DITHER) for im in frames]

    # se queda un rato en el arranque, antes de legalizar y en el resultado
    step = int(round(1000 / args.fps))
    durations = [step] * len(paletted)
    durations[0] = 1500
    if len(paletted) > 2:
        durations[-2] = 1000
    durations[-1] = 3500

    paletted[0].save(args.out, save_all=True, append_images=paletted[1:],
                     duration=durations, loop=0, optimize=True, disposal=1)
    size = os.path.getsize(args.out) / 1e6
    print("%s: %d cuadros, %.1f MB" % (args.out, len(paletted), size))


if __name__ == "__main__":
    main()
