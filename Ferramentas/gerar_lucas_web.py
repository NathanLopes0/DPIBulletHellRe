#!/usr/bin/env python3
"""Arte PROVISORIA das provas novas do Lucas (INF 321 - Programacao Web).

O cartao do professor (DPIBHLucas.png) e arte de verdade e NAO e tocado aqui; a
folha antiga dos projeteis (DPIBHLucasCodigo.png, de gerar_lucas.py) continua
valendo para os leques. Este script so desenha o que as provas novas pedem:

    DPIBHLucasWeb.png (32x32, uma linha de 16 quadros)
        0  Vazia      o quadro transparente das pecas que machucam da <div>
        1  Css        a regra de CSS: um cartao azul com "{ }"
        2  Js         a regra de JavaScript: um cartao amarelo com "( )"
        3-4 Gosma     a gosma roxa da sala do Elixir, em dois quadros
        5  Chave      a chave pequena, dourada
        6  Bau        o bau fechado, com o cadeado
        7  Aberto     o bau aberto, vazio
        8  Elixir     o frasco do Elixir: uma gota roxa
        9-10 Pena     a pena de fogo da fenix
        11 Cinzas     o monte de cinzas, com brasas
        12 Texto      a gota do overflow: um pedaco de texto que transbordou da <div>
        13 Apagada    a tocha da sala do Elixir, apagada
        14-15 Acesa   a tocha acesa, com a chama em dois quadros
    DPIBHLucasFenix.png (64x64, quatro quadros): a fenix batendo as asas.

Tudo desenhado numa grade pequena e ampliado sem suavizar, como o resto.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

AZUL, AZUL_E = (70, 140, 240, 255), (30, 70, 150, 255)
AMARELO, AMARELO_E = (245, 205, 60, 255), (150, 110, 20, 255)
BRANCO = (245, 245, 255, 255)
ROXO, ROXO_E, ROXO_C = (150, 80, 210, 255), (90, 40, 140, 255), (210, 170, 250, 255)
OURO, OURO_E = (250, 205, 70, 255), (170, 120, 30, 255)
MADEIRA, MADEIRA_E = (150, 95, 50, 255), (95, 58, 28, 255)
VIDRO = (220, 230, 250, 200)
FOGO, FOGO_C, FOGO_E = (250, 120, 40, 255), (255, 220, 90, 255), (200, 50, 30, 255)
CINZA, CINZA_E = (120, 116, 112, 255), (80, 76, 72, 255)


def quadro_json(nome, lado, quadros):
    frames = [{"filename": f"{nome} {i}.placeholder",
               "frame": {"x": i * lado, "y": 0, "w": lado, "h": lado},
               "rotated": False, "trimmed": False,
               "spriteSourceSize": {"x": 0, "y": 0, "w": lado, "h": lado},
               "sourceSize": {"w": lado, "h": lado}, "duration": 120} for i in range(quadros)]
    return {"frames": frames,
            "meta": {"app": "gerar_lucas_web.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": lado * quadros, "h": lado}, "scale": "1"}}


def grade(lado=16):
    g = Image.new("RGBA", (lado, lado), (0, 0, 0, 0))
    return g, ImageDraw.Draw(g)


def cartao(fundo, borda, simbolo):
    g, d = grade()
    d.rectangle([1, 4, 14, 11], fill=fundo, outline=borda)
    if simbolo == "chaves":   # { }
        for x, s in ((4, 1), (11, -1)):
            d.line([(x + s, 6), (x, 6), (x, 9), (x + s, 9)], fill=BRANCO)
            d.point((x - s, 7), fill=BRANCO); d.point((x - s, 8), fill=BRANCO)
    else:                     # ( )
        d.line([(5, 6), (4, 7), (4, 8), (5, 9)], fill=BRANCO)
        d.line([(10, 6), (11, 7), (11, 8), (10, 9)], fill=BRANCO)
    return g


def gosma(quadro):
    g, d = grade()
    achata = 1 if quadro else 0
    d.ellipse([2, 5 + achata, 13, 14], fill=ROXO, outline=ROXO_E)
    d.ellipse([4, 7 + achata, 7, 9 + achata], fill=ROXO_C)
    d.point((6, 10), fill=(20, 10, 30, 255)); d.point((10, 10), fill=(20, 10, 30, 255))
    return g


def chave():
    g, d = grade()
    d.ellipse([2, 2, 8, 8], outline=OURO_E, fill=OURO)
    d.ellipse([4, 4, 6, 6], fill=(0, 0, 0, 0))
    d.rectangle([7, 4, 14, 6], fill=OURO)
    d.rectangle([11, 6, 12, 9], fill=OURO); d.rectangle([13, 6, 14, 8], fill=OURO)
    return g


def bau(aberto):
    g, d = grade()
    d.rectangle([1, 7, 14, 14], fill=MADEIRA, outline=MADEIRA_E)
    if aberto:
        d.rectangle([1, 3, 14, 6], fill=MADEIRA_E)
        d.rectangle([3, 8, 12, 9], fill=(30, 20, 15, 255))
    else:
        d.rectangle([1, 4, 14, 8], fill=MADEIRA, outline=MADEIRA_E)
        d.rectangle([6, 7, 9, 10], fill=OURO, outline=OURO_E)
    d.line([(1, 11), (14, 11)], fill=OURO_E)
    return g


def elixir():
    g, d = grade()
    # O frasco em forma de gota (o roxo do Elixir), com a rolha.
    d.rectangle([7, 1, 8, 3], fill=MADEIRA)
    d.polygon([(7, 4), (8, 4), (12, 10), (12, 12), (10, 14), (5, 14), (3, 12), (3, 10)], fill=ROXO, outline=VIDRO)
    d.point((5, 10), fill=ROXO_C); d.point((5, 11), fill=ROXO_C)
    return g


def pena(quadro):
    g, d = grade()
    dx = 1 if quadro else 0
    d.polygon([(8 + dx, 1), (12, 8), (8, 15), (4, 8)], fill=FOGO, outline=FOGO_E)
    d.line([(8, 3), (8, 13)], fill=FOGO_C)
    return g


def cinzas():
    g, d = grade()
    d.ellipse([1, 8, 14, 15], fill=CINZA, outline=CINZA_E)
    d.ellipse([4, 6, 11, 12], fill=CINZA)
    for p in ((5, 10), (9, 9), (11, 12), (7, 13)):
        d.point(p, fill=FOGO)
    return g


def texto():
    g, d = grade()
    # Tres linhas de "texto" claro num retalho escuro: o conteudo que nao coube na <div>.
    d.rectangle([2, 3, 13, 12], fill=(20, 26, 44, 230), outline=(160, 190, 240, 255))
    for y, fim in ((5, 11), (7, 9), (9, 12)):
        d.line([(4, y), (fim, y)], fill=BRANCO)
    return g


def tocha(acesa, quadro=0):
    g, d = grade()
    # O pe de pedra e a cuba; acesa, a chama em cima.
    d.rectangle([6, 9, 9, 15], fill=CINZA, outline=CINZA_E)
    d.rectangle([4, 7, 11, 9], fill=CINZA_E)
    if acesa:
        dx = 1 if quadro else 0
        d.polygon([(5, 7), (10, 7), (9 + dx, 3), (8, 0), (6 - dx, 3)], fill=FOGO, outline=FOGO_E)
        d.polygon([(6, 7), (9, 7), (8, 4), (7, 3)], fill=FOGO_C)
    return g


def fenix(quadro):
    g, d = grade(32)
    # O corpo e a cabeca, no meio; as asas sobem e descem com o quadro.
    asa = (-6, -2, 3, -2)[quadro]
    for lado in (-1, 1):
        ponta = (16 + lado * 15, 12 + asa)
        d.polygon([(16 + lado * 3, 13), ponta, (16 + lado * 12, 18 + asa // 2), (16 + lado * 4, 18)], fill=FOGO, outline=FOGO_E)
        d.line([(16 + lado * 4, 14), (16 + lado * 12, 13 + asa)], fill=FOGO_C)
    d.ellipse([12, 11, 20, 21], fill=FOGO, outline=FOGO_E)
    d.ellipse([13, 6, 19, 12], fill=FOGO_C, outline=FOGO_E)
    d.point((15, 8), fill=(30, 10, 10, 255)); d.point((17, 8), fill=(30, 10, 10, 255))
    d.polygon([(15, 10), (17, 10), (16, 12)], fill=OURO)
    # A crista e a cauda de fogo.
    d.line([(16, 5), (14, 2)], fill=FOGO_C); d.line([(16, 5), (18, 2)], fill=FOGO_C)
    d.polygon([(13, 21), (19, 21), (22, 30), (16, 26), (10, 30)], fill=FOGO_E, outline=FOGO)
    d.line([(16, 22), (16, 28)], fill=FOGO_C)
    return g


def grava(quadros, nome, lado, escala):
    folha = Image.new("RGBA", (lado * len(quadros), lado), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        folha.alpha_composite(q.resize((lado, lado), Image.NEAREST), (i * lado, 0))
    destino = os.path.join(RAIZ, "Assets", "Teachers", "Projectiles")
    folha.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w") as f:
        json.dump(quadro_json(nome, lado, len(quadros)), f, indent=1)
    print(f"  {nome}.png  {folha.width}x{folha.height}  ({len(quadros)} quadros)")


if __name__ == "__main__":
    vazia, _ = grade()
    grava([vazia, cartao(AZUL, AZUL_E, "chaves"), cartao(AMARELO, AMARELO_E, "parenteses"),
           gosma(0), gosma(1), chave(), bau(False), bau(True), elixir(), pena(0), pena(1), cinzas(),
           texto(), tocha(False), tocha(True, 0), tocha(True, 1)],
          "DPIBHLucasWeb", 32, 2)
    grava([fenix(i) for i in range(4)], "DPIBHLucasFenix", 64, 2)
