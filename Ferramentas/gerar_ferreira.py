#!/usr/bin/env python3
"""Arte PROVISORIA do Lucas Ferreira (INF 216 - Projeto e Implementacao de Jogos Digitais, a
optativa secreta): o cartao do professor e as folhas dos projeteis das quatro provas.

    python3 Ferramentas/gerar_ferreira.py              # grava os PNG e os JSON em Assets/
    python3 Ferramentas/gerar_ferreira.py --animacoes  # so imprime o mapa nome -> quadros

As provas dele mexem nos sistemas do proprio jogo, e a arte e a da interface e a do motor: as
pecas de uma janela, os botoes de um dialogo, a moeda, o disquete do save, as caixas de colisao do
jogo de plataforma. A barra da nota nao tem folha: no campo, ela e a da HUD, de verdade.

  DPIBHFerreira          128x128   o cartao: o controle, o coracao de pixel e a barra de nota
  DPIBHFerreiraTiros      20x20    Tiro Moeda Seu Vazio Fragmento Disquete
  DPIBHFerreiraFisica     28x28    Caixa Espinho Lanca Bloco Fagulha Vazia (a plataforma; a Vazia
                                   e o segmento sem cara embaixo da barra da nota)
  DPIBHFerreiraJanela     20x20    Borda Titulo X
  DPIBHFerreiraBotoes     64x40    Esperar Fechar
  DPIBHFerreiraBugs       32x32    Bug (um besouro visto de cima; dois quadros: as patas mexem)
  DPIBHFerreiraBlocos     28x28    Bloco (um tijolo de uma linha dos creditos)

Os nomes das animacoes sao os que Ferreira.cpp e JogosDigitais.h usam; um teste confere cada um.
"""

import json
import os
import sys

from PIL import Image, ImageDraw, ImageFont

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
SAIRA = os.path.join(RAIZ, "Assets", "Fonts", "SairaSemiCondensed-SemiBold.ttf")

CARTAO = (24, 20, 44, 255)
BORDA = (255, 110, 200, 255)
TINTA = (236, 240, 250, 255)
TRACO = (12, 12, 22, 255)
AMBAR = (255, 205, 90, 255)
ROXO = (170, 110, 255)
VERMELHO = (235, 70, 70)
AMARELO = (240, 200, 60)
VERDE = (110, 220, 130)


def fonte(tamanho, negrito=False):
    caminho = SAIRA if negrito else CARLITO
    try:
        return ImageFont.truetype(caminho if os.path.exists(caminho) else CARLITO, tamanho)
    except OSError:
        return ImageFont.load_default()


def centrado(d, texto, cx, cy, f, cor):
    caixa = d.textbbox((0, 0), texto, font=f)
    w, h = caixa[2] - caixa[0], caixa[3] - caixa[1]
    d.text((cx - w / 2 - caixa[0], cy - h / 2 - caixa[1]), texto, font=f, fill=cor)


def escuro(c, f=0.45):
    return tuple(int(x * f) for x in c[:3]) + (255,)


def claro(c, f=0.45):
    return tuple(int(x + (255 - x) * f) for x in c[:3]) + (255,)


def cheia(c):
    return tuple(c[:3]) + (255,)


def pixels(d, mapa, x0, y0, lado, cores):
    """Desenha um mapa de pixels: cada letra de 'mapa' e uma cor de 'cores' ('.' e vazio)."""
    for j, linha in enumerate(mapa):
        for i, ch in enumerate(linha):
            if ch in cores:
                d.rectangle([x0 + i * lado, y0 + j * lado, x0 + (i + 1) * lado - 1, y0 + (j + 1) * lado - 1], fill=cores[ch])


CORACAO = [".##.##.",
           "#######",
           "#######",
           ".#####.",
           "..###..",
           "...#..."]


def ferreira():
    """O cartao do professor: um controle, um coracao de pixel e uma barra de nota pela metade."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)
    # O controle.
    d.rounded_rectangle([22, 26, 106, 66], radius=14, fill=(52, 48, 84, 255), outline=(150, 140, 210, 255), width=2)
    d.rectangle([34, 42, 50, 48], fill=TINTA)
    d.rectangle([39, 37, 45, 53], fill=TINTA)
    for cx, cy, cor in ((84, 38, VERDE), (94, 46, VERMELHO), (74, 46, (110, 170, 255)), (84, 54, AMARELO)):
        d.ellipse([cx - 4, cy - 4, cx + 4, cy + 4], fill=cheia(cor), outline=TRACO)
    d.rectangle([58, 50, 62, 53], fill=(150, 140, 210, 255))
    d.rectangle([66, 50, 70, 53], fill=(150, 140, 210, 255))
    # O coracao de pixel e a barra de nota.
    pixels(d, CORACAO, 16, 72, 2, {"#": cheia(VERMELHO)})
    d.rectangle([36, 74, 110, 82], fill=(10, 10, 20, 255), outline=(200, 200, 220, 255))
    d.rectangle([38, 76, 80, 80], fill=cheia(AMARELO))
    centrado(d, "INF 216", 64, 95, fonte(12, True), AMBAR)
    centrado(d, "L. FERREIRA", 64, 108, fonte(11), TINTA)
    centrado(d, "Jogos Digitais", 64, 118, fonte(8), claro(BORDA, 0.2))
    return im


# ---- Os tiros (20 x 20) -------------------------------------------------------------

TW = 20
TIROS = ["Tiro", "Moeda", "Seu", "Vazio", "Fragmento", "Disquete"]


def tiro(nome):
    im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if nome == "Tiro":
        # Uma bolinha de pixel, em degraus.
        mapa = ["..####..", ".#oooo#.", "#ooOOoo#", "#oOOOOo#", "#oOOOOo#", "#ooOOoo#", ".#oooo#.", "..####.."]
        pixels(d, mapa, 2, 2, 2, {"#": escuro(ROXO, 0.5), "o": cheia(ROXO), "O": claro(ROXO, 0.6)})
    elif nome == "Moeda":
        d.ellipse([2, 2, 17, 17], fill=cheia(AMARELO), outline=escuro(AMARELO, 0.5))
        d.ellipse([5, 5, 14, 14], outline=claro(AMARELO, 0.6))
        d.rectangle([9, 6, 10, 13], fill=escuro(AMARELO, 0.45))
        d.rectangle([6, 9, 13, 10], fill=escuro(AMARELO, 0.45))
    elif nome == "Seu":
        # A duvida do aluno, de volta e contra ele: o balao vermelho.
        d.rounded_rectangle([2, 2, 17, 14], radius=4, fill=cheia(VERMELHO), outline=TRACO)
        d.polygon([(6, 14), (10, 14), (5, 18)], fill=cheia(VERMELHO))
        centrado(d, "?", 10, 8, fonte(11, True), TINTA)
    elif nome == "Fragmento":
        d.polygon([(10, 3), (16, 10), (10, 17), (4, 10)], fill=claro(ROXO, 0.3), outline=escuro(ROXO, 0.4))
    elif nome == "Disquete":
        d.rectangle([2, 2, 17, 17], fill=(60, 110, 220, 255), outline=TRACO)
        d.rectangle([5, 2, 14, 8], fill=(210, 215, 230, 255))
        d.rectangle([11, 3, 13, 7], fill=(60, 110, 220, 255))
        d.rectangle([5, 11, 14, 17], fill=TINTA)
    return im


# ---- As pecas da plataforma (28 x 28) -------------------------------------------------

FW = 28
FISICA = ["Caixa", "Espinho", "Lanca", "Bloco", "Fagulha", "Vazia"]


def fisica(nome):
    """As pecas do jogo de plataforma, de pe: o chao e o lado de baixo do quadro (o C++ gira cada
    uma para o chao da vez), e a lanca aponta para a direita."""
    im = Image.new("RGBA", (FW, FW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if nome == "Caixa":
        # A caixa de colisao do modo debug: o fundo translucido, a borda e o X de canto a canto.
        d.rectangle([1, 1, FW - 2, FW - 2], fill=(235, 70, 90, 95), outline=(255, 90, 110, 255), width=2)
        d.line([(4, 4), (FW - 5, FW - 5)], fill=(255, 150, 160, 220), width=1)
        d.line([(4, FW - 5), (FW - 5, 4)], fill=(255, 150, 160, 220), width=1)
        for (x, y) in [(1, 1), (FW - 2, 1), (1, FW - 2), (FW - 2, FW - 2)]:
            d.rectangle([x - 1, y - 1, x + 1, y + 1], fill=TINTA)
    elif nome == "Espinho":
        # O espinho do buraco: a base rente ao chao, a ponta vermelha.
        d.polygon([(14, 2), (25, FW - 2), (3, FW - 2)], fill=(196, 200, 214, 255), outline=TRACO)
        d.polygon([(14, 2), (18, 11), (10, 11)], fill=cheia(VERMELHO))
        d.line([(14, 6), (8, FW - 3)], fill=(240, 242, 250, 255))
    elif nome == "Lanca":
        # A lanca: a haste de codigo e a ponta, deitadas, apontando para a direita.
        d.rectangle([1, 12, 18, 15], fill=cheia(AMBAR), outline=escuro(AMBAR, 0.5))
        d.polygon([(17, 6), (27, 14), (17, 21)], fill=claro(AMBAR, 0.4), outline=escuro(AMBAR, 0.5))
        d.rectangle([1, 10, 3, 17], fill=escuro(AMBAR, 0.35))
    elif nome == "Bloco":
        # Um ladrilho do cenario que caiu: a borda em degrau e os rebites.
        d.rectangle([1, 1, FW - 2, FW - 2], fill=(92, 78, 150, 255), outline=TRACO)
        d.rectangle([3, 3, FW - 4, 7], fill=(150, 132, 220, 255))
        d.rectangle([3, FW - 7, FW - 4, FW - 4], fill=(58, 48, 100, 255))
        for (x, y) in [(6, 12), (FW - 7, 12), (6, FW - 11), (FW - 7, FW - 11)]:
            d.rectangle([x - 1, y - 1, x + 1, y + 1], fill=(200, 190, 245, 255))
    elif nome == "Fagulha":
        # O estilhaco do bloco no chao: uma faisca de oito pontas.
        c = FW // 2
        pontas = []
        import math
        for k in range(16):
            r = 12 if k % 2 == 0 else 5
            a = math.pi * k / 8
            pontas.append((c + r * math.cos(a), c + r * math.sin(a)))
        d.polygon(pontas, fill=(255, 196, 70, 255), outline=(150, 70, 20, 255))
        d.ellipse([c - 4, c - 4, c + 4, c + 4], fill=(255, 246, 200, 255))
    return im


# ---- A janela (20 x 20) ---------------------------------------------------------------

JW = 20
JANELA = ["Borda", "Titulo", "X"]


def janela(nome):
    im = Image.new("RGBA", (JW, JW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if nome == "Borda":
        d.rectangle([1, 1, JW - 2, JW - 2], fill=(196, 200, 214, 255), outline=(90, 94, 110, 255))
        d.line([(2, 2), (JW - 3, 2)], fill=(240, 242, 250, 255))
        d.line([(2, 2), (2, JW - 3)], fill=(240, 242, 250, 255))
    elif nome == "Titulo":
        d.rectangle([0, 1, JW - 1, JW - 2], fill=(40, 70, 190, 255), outline=(20, 30, 100, 255))
        d.line([(0, 3), (JW - 1, 3)], fill=(110, 150, 255, 255))
    elif nome == "X":
        d.rectangle([0, 0, JW - 1, JW - 1], fill=cheia(VERMELHO), outline=TINTA)
        d.line([(5, 5), (14, 14)], fill=TINTA, width=3)
        d.line([(14, 5), (5, 14)], fill=TINTA, width=3)
    return im


# ---- Os botoes do dialogo (64 x 40) ---------------------------------------------------

QW, QH = 64, 40
BOTOES = {"Esperar": ("ESPERAR", VERDE), "Fechar": ("FECHAR", VERMELHO)}


def botao(nome):
    texto, cor = BOTOES[nome]
    im = Image.new("RGBA", (QW, QH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1, 5, QW - 2, QH - 6], radius=5, fill=escuro(cor, 0.5), outline=TINTA, width=2)
    d.rounded_rectangle([4, 8, QW - 5, QH - 9], radius=3, fill=cheia(cor))
    centrado(d, texto, QW / 2, QH / 2 - 1, fonte(12, True), TRACO)
    return im


# ---- Os bugs do lancamento (32 x 32) --------------------------------------------------

GW = 32


def bug(quadro):
    """Um besouro visto de cima: o bug de software. As patas trocam de um quadro para o outro."""
    im = Image.new("RGBA", (GW, GW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    casca = (120, 235, 120)
    # As seis patas, tres de cada lado, abertas num quadro e recolhidas no outro.
    abre = 3 if quadro == 0 else -2
    for k, y in enumerate((11, 16, 21)):
        dy = abre * (k - 1)
        d.line([(9, y), (3, y + dy - 2)], fill=escuro(casca, 0.55), width=2)
        d.line([(22, y), (28, y + dy - 2)], fill=escuro(casca, 0.55), width=2)
    # A cabeca, a casca partida ao meio e duas pintas.
    d.ellipse([11, 3, 20, 11], fill=escuro(casca, 0.4), outline=TRACO)
    d.ellipse([7, 7, 24, 27], fill=cheia(casca), outline=TRACO)
    d.line([(15, 8), (15, 26)], fill=escuro(casca, 0.5), width=2)
    d.ellipse([10, 13, 13, 16], fill=escuro(casca, 0.45))
    d.ellipse([18, 18, 21, 21], fill=escuro(casca, 0.45))
    d.arc([9, 9, 22, 22], 200, 260, fill=claro(casca, 0.6))
    return im


# ---- Os blocos dos creditos (28 x 28) -------------------------------------------------

CW = 28


def bloco():
    im = Image.new("RGBA", (CW, CW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 3, CW - 1, CW - 4], fill=(34, 34, 48, 255))
    d.rectangle([1, 5, CW - 2, CW - 6], fill=(226, 228, 238, 255))
    d.line([(0, 3), (CW - 1, 3)], fill=(120, 124, 150, 255))
    d.line([(0, CW - 4), (CW - 1, CW - 4)], fill=(120, 124, 150, 255))
    d.rectangle([11, 0, 16, 2], fill=(34, 34, 48, 255))
    d.rectangle([11, CW - 3, 16, CW - 1], fill=(34, 34, 48, 255))
    return im


# ---- Gravar ----------------------------------------------------------------------------

def quadro_json(nome, w, h):
    return {"frames": [{
                "filename": f"{nome} 0.placeholder",
                "frame": {"x": 0, "y": 0, "w": w, "h": h},
                "rotated": False, "trimmed": False,
                "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
                "sourceSize": {"w": w, "h": h},
                "duration": 120}],
            "meta": {"app": "gerar_ferreira.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def tira(quadros, w, h, colunas):
    linhas = (len(quadros) + colunas - 1) // colunas
    f = Image.new("RGBA", (w * colunas, h * linhas), (0, 0, 0, 0))
    for i, im in enumerate(quadros):
        f.alpha_composite(im, ((i % colunas) * w, (i // colunas) * h))
    return f


def tira_json(nome, f, w, h, n):
    colunas = f.width // w
    frames = []
    for i in range(n):
        x, y = (i % colunas) * w, (i // colunas) * h
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": w, "h": h},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
            "sourceSize": {"w": w, "h": h},
            "duration": 120,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_ferreira.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": f.width, "h": f.height}, "scale": "1"}}


def grava_tira(quadros, nome, w, h, colunas):
    f = tira(quadros, w, h, colunas)
    destino = os.path.join(RAIZ, "Assets/Teachers/Projectiles")
    f.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as arq:
        json.dump(tira_json(nome, f, w, h, len(quadros)), arq, indent=1)
    print(f"  Assets/Teachers/Projectiles/{nome}.png  {f.width}x{f.height}  ({len(quadros)} quadro(s))")


def grava(im, nome, pasta):
    destino = os.path.join(RAIZ, pasta)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}")


def animacoes():
    return {
        "Tiros": {nome: [i] for i, nome in enumerate(TIROS)},
        "Fisica": {nome: [i] for i, nome in enumerate(FISICA)},
        "Janela": {nome: [i] for i, nome in enumerate(JANELA)},
        "Botoes": {nome: [i] for i, nome in enumerate(BOTOES)},
        "Bugs": {"Bug": [0, 1]},
        "Blocos": {"Bloco": [0]},
    }


if __name__ == "__main__":
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(ferreira(), "DPIBHFerreira", "Assets/Teachers")
    grava_tira([tiro(n) for n in TIROS], "DPIBHFerreiraTiros", TW, TW, len(TIROS))
    grava_tira([fisica(n) for n in FISICA], "DPIBHFerreiraFisica", FW, FW, len(FISICA))
    grava_tira([janela(n) for n in JANELA], "DPIBHFerreiraJanela", JW, JW, len(JANELA))
    grava_tira([botao(n) for n in BOTOES], "DPIBHFerreiraBotoes", QW, QH, len(BOTOES))
    grava_tira([bug(0), bug(1)], "DPIBHFerreiraBugs", GW, GW, 2)
    grava_tira([bloco()], "DPIBHFerreiraBlocos", CW, CW, 1)
