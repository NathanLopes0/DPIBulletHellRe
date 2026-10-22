#!/usr/bin/env python3
"""Arte PROVISORIA dos projeteis novos do Julio (INF 420 - Inteligencia
Artificial).

O cartao do professor (DPIBHJulio.png) e os dados (DPIBHJulioDado.png) sao a
arte de antes e NAO sao gerados aqui: este script so desenha a folha nova,
DPIBHJulioToken.png. Mesmo estilo de placeholder dos outros gerar_*.py - trocar
o PNG basta, o json de quadro continua valendo.

A MATERIA E A DE UM LLM, e os projeteis sao as pecas dele:

    Token   um pedaco de palavra num chip, como um modelo de linguagem quebra
            o texto. Os sete primeiros, em ordem, formam a frase que o
            professor gera na fase 1: "o mo de lo te pre diz" - o modelo te
            prediz. O oitavo e o "e" da repeticao da fase final.
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CHIP   = (24, 34, 52, 255)      # o fundo escuro do chip
BORDA  = (90, 220, 220, 255)    # o ciano do cartao do professor
TEXTO  = (220, 250, 250, 255)
PINO   = (120, 140, 160, 255)   # as perninhas do chip

TOKENS = ["o", "mo", "de", "lo", "te", "pre", "diz", "e"]


def quadro_json(nome, w, h, quadros=1, lado=None):
    lado = lado or w
    colunas = max(1, w // lado)
    frames = []
    for i in range(quadros):
        x, y = (i % colunas) * lado, (i // colunas) * lado
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": lado, "h": lado},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": lado, "h": lado},
            "sourceSize": {"w": lado, "h": lado},
            "duration": 120,
        })
    return {
        "frames": frames,
        "meta": {"app": "gerar_julio.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def token(texto):
    """UM TOKEN: o chip com o pedaco de palavra, e as perninhas dos lados."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for x in (9, 15, 21):
        d.line([x, 7, x, 9], fill=PINO, width=1)
        d.line([x, 22, x, 24], fill=PINO, width=1)
    d.rounded_rectangle([3, 9, 28, 22], radius=3, fill=CHIP, outline=BORDA, width=1)
    fonte = ImageFont.load_default(size=10)
    d.text((16, 15), texto, fill=TEXTO, font=fonte, anchor="mm")
    return im


def folha():
    """Quatro quadros por linha, de 32 px: os oito tokens em duas linhas."""
    desenhos = [token(t) for t in TOKENS]
    linhas = (len(desenhos) + 3) // 4
    f = Image.new("RGBA", (128, 32 * linhas), (0, 0, 0, 0))
    for i, im in enumerate(desenhos):
        f.alpha_composite(im, ((i % 4) * 32, (i // 4) * 32))
    return f, len(desenhos)


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    im, n = folha()
    grava(im, "DPIBHJulioToken", "Assets/Teachers/Projectiles", quadros=n, lado=32)
