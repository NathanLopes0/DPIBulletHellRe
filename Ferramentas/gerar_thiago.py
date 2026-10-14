#!/usr/bin/env python3
"""Arte PROVISORIA do Thiago (INF 220) e do projetil dele.

Segue o mesmo estilo de cartao que o DPIBHJulio.png ja usa no repositorio: e
deliberadamente um placeholder, para nao se passar por arte final. Quando o
sprite de verdade existir, basta trocar o PNG - o json de quadro continua valendo.
"""
from PIL import Image, ImageDraw
import json, os, sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO   = (74, 94, 128, 255)
BORDA    = (150, 180, 220, 255)
TINTA    = (228, 238, 250, 255)
CELULA   = (122, 210, 180, 255)
CABECALHO= (92, 160, 200, 255)


def quadro_json(nome, w, h):
    return {
        "frames": [{
            "filename": f"{nome}.png 0.aseprite",
            "frame": {"x": 0, "y": 0, "w": w, "h": h},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
            "sourceSize": {"w": w, "h": h},
            "duration": 120,
        }],
        "meta": {"app": "gerar_thiago.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def thiago():
    """O cartao do professor: uma tabela de 3x3 com a primeira linha realcada."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A tabela, com cabecalho - o icone que diz "banco de dados" sem escrever.
    x0, y0, cw, ch, cols, rows = 24, 26, 27, 16, 3, 3
    for r in range(rows):
        for c in range(cols):
            x, y = x0 + c * cw, y0 + r * ch
            d.rectangle([x, y, x + cw - 3, y + ch - 3],
                        fill=CABECALHO if r == 0 else CELULA,
                        outline=(40, 52, 70, 255))

    d.text((40, 92), "THIAGO", fill=TINTA)
    return im


def celula():
    """O projetil: uma celula de tabela, 16x16, com borda marcada."""
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, 15, 15], fill=CELULA, outline=(34, 44, 60, 255))
    d.rectangle([3, 3, 12, 12], outline=(255, 255, 255, 110))
    return im


def grava(im, nome, pasta):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    png = os.path.join(destino, nome + ".png")
    im.save(png)
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}")


if __name__ == "__main__":
    grava(thiago(), "DPIBHThiago", "Assets/Teachers")
    grava(celula(), "DPIBHThiagoCelula", "Assets/Teachers/Projectiles")
