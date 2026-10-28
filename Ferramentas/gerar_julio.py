#!/usr/bin/env python3
"""Arte PROVISORIA dos projeteis novos do Julio (INF 420 - Inteligencia
Artificial).

O cartao do professor (DPIBHJulio.png) e os dados (DPIBHJulioDado.png) sao a
arte de antes e NAO sao gerados aqui: este script so desenha a folha nova,
DPIBHJulioToken.png. Mesmo estilo de placeholder dos outros gerar_*.py - trocar
o PNG basta, o json de quadro continua valendo.

A MATERIA E A DE UM LLM, e os projeteis sao as pecas dele:

    Token       um pedaco de palavra num chip, como um modelo de linguagem
                quebra o texto. Os sete primeiros, em ordem, formam a frase que
                o professor gera na fase 1: "o mo de lo te pre diz" - o modelo
                te prediz. O oitavo e o "e" da repeticao da fase final.
    Agente      o robo que o professor convoca na fase 2: uma cabeca com antena
                e um visor que varre de um lado para o outro.
    Ferramenta  a engrenagem que o agente manda quando age - a "chamada de
                ferramenta" de um agente de IA -, girando de um quadro ao outro.
    Guardrail   o robo-escudo da fase 3, que gira em volta do professor e
                barra o tiro do aluno: um escudo com o visor aceso, pulsando.
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CHIP   = (24, 34, 52, 255)      # o fundo escuro do chip
BORDA  = (90, 220, 220, 255)    # o ciano do cartao do professor
TEXTO  = (220, 250, 250, 255)
PINO   = (120, 140, 160, 255)   # as perninhas do chip

TOKENS = ["o", "mo", "de", "lo", "te", "pre", "diz", "e"]

METAL  = (150, 170, 190, 255)   # a cabeca do robo
SOMBRA = (70, 84, 104, 255)
VISOR  = (16, 24, 36, 255)
OLHO   = (90, 240, 200, 255)
LUZ    = (250, 210, 80, 255)    # a ponta da antena
ENGRENAGEM = (240, 180, 70, 255)


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


def agente(fase):
    """O AGENTE: cabeca de robo, antena que pisca e visor que varre."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.line([16, 4, 16, 9], fill=SOMBRA, width=2)                         # a antena
    d.ellipse([13, 1, 19, 7], fill=LUZ if fase % 2 == 0 else SOMBRA, outline=VISOR)
    d.rounded_rectangle([5, 9, 26, 28], radius=5, fill=METAL, outline=VISOR, width=2)
    d.rectangle([2, 15, 5, 21], fill=SOMBRA)                             # as orelhas
    d.rectangle([26, 15, 29, 21], fill=SOMBRA)
    d.rounded_rectangle([8, 14, 23, 21], radius=3, fill=VISOR)           # o visor
    x = (10, 14, 18, 14)[fase]
    d.rectangle([x, 16, x + 3, 19], fill=OLHO)                           # o olho, varrendo
    d.line([11, 24, 20, 24], fill=SOMBRA, width=2)                       # a boca
    return im


def ferramenta(fase):
    """A FERRAMENTA: uma engrenagem de oito dentes, girando."""
    import math
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    giro = fase * math.pi / 16
    pontos = []
    for k in range(16):
        r = 11 if k % 2 == 0 else 8
        a = giro + k * math.pi / 8
        pontos.append((16 + r * math.cos(a), 16 + r * math.sin(a)))
    d.polygon(pontos, fill=ENGRENAGEM, outline=VISOR)
    d.ellipse([12, 12, 20, 20], fill=VISOR)
    return im


def guardrail(fase):
    """O GUARDRAIL: um escudo de metal com o visor do robo, pulsando."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(5, 5), (26, 5), (26, 16), (16, 28), (5, 16)], fill=METAL, outline=VISOR)
    d.polygon([(8, 8), (23, 8), (23, 15), (16, 24), (8, 15)], fill=SOMBRA)
    brilho = (OLHO, (150, 250, 225, 255), OLHO, (60, 170, 140, 255))[fase]
    d.rectangle([10, 11, 21, 14], fill=VISOR)
    d.rectangle([11, 12, 20, 13], fill=brilho)
    d.line([12, 18, 16, 21], fill=LUZ, width=2)                          # a divisa
    d.line([16, 21, 20, 18], fill=LUZ, width=2)
    return im


def folha():
    """Quatro quadros por linha, de 32 px: os oito tokens em duas linhas, e
    uma linha para cada coisa que vem depois. As linhas novas vao sempre no
    FIM: os quadros de antes ficam onde estavam."""
    desenhos = [token(t) for t in TOKENS]
    desenhos += [agente(i) for i in range(4)]
    desenhos += [ferramenta(i) for i in range(4)]
    desenhos += [guardrail(i) for i in range(4)]
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
