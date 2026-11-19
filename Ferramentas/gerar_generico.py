#!/usr/bin/env python3
"""A arte do professor generico da fase tutorial (INF 191 - Introducao a Ciencia
da Computacao).

Nao e ninguem do DPI: e "o professor", de oculos, camisa clara e gravata, com a
pasta debaixo do braco - para o tutorial nao parecer a prova de um professor de
verdade. Desenhado numa grade de 32x32 e ampliado 4x, como o cartao do Andre.

    DPIBHGenerico.png            o professor (128x128, um quadro)
    Projectiles/DPIBHGenericoBola.png
                                 a bolinha que ele atira no tutorial (32x32, dois
                                 quadros: acesa e piscando)
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

PELE    = (240, 196, 160, 255)
PELE_S  = (214, 164, 128, 255)
CABELO  = (120, 120, 128, 255)   # grisalho
CABELO_S = (92, 92, 100, 255)
OCULOS  = (150, 150, 160, 255)   # a armacao prateada, para os olhos aparecerem
OLHO    = (30, 30, 36, 255)
BOCA    = (170, 90, 80, 255)
CAMISA  = (220, 228, 240, 255)
CAMISA_S = (180, 190, 208, 255)
GRAVATA = (60, 110, 200, 255)
CALCA   = (64, 70, 92, 255)
SAPATO  = (40, 34, 30, 255)
PASTA   = (120, 80, 50, 255)
PASTA_S = (90, 58, 36, 255)

BOLA    = (250, 130, 70, 255)
BOLA_S  = (210, 80, 50, 255)
BRILHO  = (255, 230, 200, 255)


def quadro_json(nome, lado, quadros):
    frames = []
    for i in range(quadros):
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": i * lado, "y": 0, "w": lado, "h": lado},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": lado, "h": lado},
            "sourceSize": {"w": lado, "h": lado},
            "duration": 120,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_generico.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": lado * quadros, "h": lado}, "scale": "1"}}


def ret(d, x0, y0, x1, y1, cor):
    d.rectangle([x0, y0, x1, y1], fill=cor)


def professor():
    g = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(g)
    # As pernas e os sapatos.
    ret(d, 12, 25, 14, 29, CALCA); ret(d, 17, 25, 19, 29, CALCA)
    ret(d, 11, 30, 14, 31, SAPATO); ret(d, 17, 30, 20, 31, SAPATO)
    # O tronco, a camisa, a gravata.
    ret(d, 10, 16, 21, 25, CAMISA)
    ret(d, 10, 16, 10, 25, CAMISA_S); ret(d, 21, 16, 21, 25, CAMISA_S)
    ret(d, 15, 16, 16, 23, GRAVATA); ret(d, 14, 16, 17, 17, GRAVATA)
    # Os bracos; a mao direita segura a pasta.
    ret(d, 7, 17, 9, 23, CAMISA); ret(d, 22, 17, 24, 23, CAMISA)
    ret(d, 7, 24, 9, 25, PELE); ret(d, 22, 24, 24, 25, PELE)
    ret(d, 23, 22, 28, 27, PASTA); ret(d, 23, 27, 28, 27, PASTA_S); ret(d, 25, 21, 26, 21, PASTA_S)
    # O pescoco e a cabeca.
    ret(d, 14, 14, 17, 15, PELE_S)
    ret(d, 11, 4, 20, 13, PELE)
    ret(d, 10, 7, 10, 10, PELE); ret(d, 21, 7, 21, 10, PELE)   # as orelhas
    # O cabelo grisalho, curto, com entradas.
    ret(d, 11, 2, 20, 4, CABELO); ret(d, 10, 3, 11, 7, CABELO); ret(d, 20, 3, 21, 7, CABELO)
    ret(d, 14, 4, 17, 4, PELE)
    ret(d, 11, 2, 20, 2, CABELO_S)
    # Os oculos e os olhos.
    # A armacao vazada: o aro em volta, a pele e o olho dentro.
    for x0 in (11, 16):
        d.rectangle([x0, 7, x0 + 3, 9], outline=OCULOS)
        ret(d, x0 + 1, 8, x0 + 2, 8, PELE)
        d.point((x0 + 2, 8), fill=OLHO)
    ret(d, 15, 7, 15, 7, OCULOS)
    # O sorriso.
    ret(d, 14, 11, 17, 11, BOCA)
    return g.resize((128, 128), Image.NEAREST)


def bola(piscando):
    g = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(g)
    d.ellipse([2, 2, 13, 13], fill=BOLA_S)
    d.ellipse([2, 2, 12, 12], fill=BRILHO if piscando else BOLA)
    d.ellipse([4, 4, 7, 7], fill=BRILHO)
    return g.resize((32, 32), Image.NEAREST)


def main():
    prof = professor()
    prof.save(os.path.join(RAIZ, "Assets", "Teachers", "DPIBHGenerico.png"))
    with open(os.path.join(RAIZ, "Assets", "Teachers", "DPIBHGenerico.json"), "w") as f:
        json.dump(quadro_json("DPIBHGenerico", 128, 1), f, indent=1)

    folha = Image.new("RGBA", (64, 32), (0, 0, 0, 0))
    folha.alpha_composite(bola(False), (0, 0))
    folha.alpha_composite(bola(True), (32, 0))
    destino = os.path.join(RAIZ, "Assets", "Teachers", "Projectiles")
    folha.save(os.path.join(destino, "DPIBHGenericoBola.png"))
    with open(os.path.join(destino, "DPIBHGenericoBola.json"), "w") as f:
        json.dump(quadro_json("DPIBHGenericoBola", 32, 2), f, indent=1)


if __name__ == "__main__":
    main()
