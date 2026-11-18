#!/usr/bin/env python3
"""Arte PROVISORIA do Lucas Ferreira (INF 216 - Projeto e Implementacao de Jogos Digitais, a
optativa secreta): o cartao do professor e as folhas dos projeteis das quatro provas.

    python3 Ferramentas/gerar_ferreira.py              # grava os PNG e os JSON em Assets/
    python3 Ferramentas/gerar_ferreira.py --animacoes  # so imprime o mapa nome -> quadros

As provas dele mexem nos sistemas do proprio jogo, e a arte e a da interface e a do motor: o
inimigo sem textura (o xadrez magenta e preto da arte que nao carregou), os botoes de um dialogo,
a moeda, o disquete do save, as caixas de colisao do jogo de plataforma. A barra da nota nao tem
folha: no campo, ela e a da HUD, de verdade.

  DPIBHFerreira          128x128   o cartao: o controle, o coracao de pixel e a barra de nota
  DPIBHFerreiraTiros      20x20    Tiro Moeda Seu Vazio Fragmento SemDt ComDt (o vermelho feito sem
                                   deltaTime e o azul com ele, os dois do FPS)
  DPIBHFerreiraFisica     28x28    Caixa Espinho Lanca Bloco Fagulha Vazia (a plataforma; a Vazia
                                   e o segmento sem cara embaixo da barra da nota)
  DPIBHFerreiraInimigo    24x24    Textura Moeda Olho OlhoEsquerda OlhoDireita Assustado Poder (dois
                                   quadros): cada bloco do inimigo gigante, os olhos e o power-up
  DPIBHFerreiraSave       34x34    Save Foge Persegue: o disquete do save, vivo
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
TIROS = ["Tiro", "Moeda", "Seu", "Vazio", "Fragmento", "SemDt", "ComDt"]
AZUL = (90, 160, 255)


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
    elif nome == "SemDt":
        # O vermelho, feito sem deltaTime: a bolinha quente, com o miolo branco - corre com o FPS.
        mapa = ["..####..", ".#rrrr#.", "#rrRRrr#", "#rRWWRr#", "#rRWWRr#", "#rrRRrr#", ".#rrrr#.", "..####.."]
        pixels(d, mapa, 2, 2, 2, {"#": escuro(VERMELHO, 0.5), "r": cheia(VERMELHO), "R": claro(VERMELHO, 0.35), "W": TINTA})
    elif nome == "ComDt":
        # O azul, com deltaTime: um relogio - anda tantos px por segundo, em qualquer FPS.
        d.ellipse([2, 2, 17, 17], fill=cheia(AZUL), outline=escuro(AZUL, 0.45))
        d.ellipse([5, 5, 14, 14], fill=claro(AZUL, 0.75), outline=escuro(AZUL, 0.5))
        d.line([(10, 10), (10, 6)], fill=escuro(AZUL, 0.3), width=1)
        d.line([(10, 10), (13, 10)], fill=escuro(AZUL, 0.3), width=1)
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


# ---- O inimigo gigante (24 x 24) ------------------------------------------------------

IW = 24
INIMIGO = ["Textura", "Moeda", "Olho", "OlhoEsquerda", "OlhoDireita", "Assustado", "Poder0", "Poder1"]
MAGENTA = (255, 0, 220, 255)
PRETO_DA_TEXTURA = (58, 12, 72, 255)


def estrela(d, cx, cy, r_fora, r_dentro, cor, contorno):
    import math
    pontas = []
    for k in range(10):
        r = r_fora if k % 2 == 0 else r_dentro
        a = -math.pi / 2 + math.pi * k / 5
        pontas.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    d.polygon(pontas, fill=cor, outline=contorno)


def inimigo(nome):
    """Um bloco do inimigo gigante: a textura que faltou (o xadrez magenta e preto que se repete de
    bloco em bloco), a moeda em que ele vira quando foge, os olhos e o power-up."""
    im = Image.new("RGBA", (IW, IW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = IW // 2
    if nome == "Textura":
        d.rectangle([0, 0, m - 1, m - 1], fill=MAGENTA)
        d.rectangle([m, 0, IW - 1, m - 1], fill=PRETO_DA_TEXTURA)
        d.rectangle([0, m, m - 1, IW - 1], fill=PRETO_DA_TEXTURA)
        d.rectangle([m, m, IW - 1, IW - 1], fill=MAGENTA)
        # Um brilho de um pixel no canto de cada quadrado magenta: a textura tem relevo de pixel.
        d.line([(1, 1), (m - 3, 1)], fill=(255, 150, 245, 255))
        d.line([(m + 1, m + 1), (IW - 3, m + 1)], fill=(255, 150, 245, 255))
    elif nome == "Moeda":
        d.ellipse([1, 1, IW - 2, IW - 2], fill=cheia(AMARELO), outline=escuro(AMARELO, 0.5), width=2)
        d.ellipse([5, 5, IW - 6, IW - 6], outline=claro(AMARELO, 0.6))
        d.rectangle([m - 1, 7, m, IW - 8], fill=escuro(AMARELO, 0.45))
        d.rectangle([7, m - 1, IW - 8, m], fill=escuro(AMARELO, 0.45))
    elif nome.startswith("Olho") or nome == "Assustado":
        # O olho e um bloco tambem: o branco ocupa o bloco inteiro, e a pupila olha para o aluno.
        d.rectangle([0, 0, IW - 1, IW - 1], fill=(250, 250, 255, 255), outline=PRETO_DA_TEXTURA, width=2)
        if nome == "Assustado":
            d.rectangle([m - 2, m - 1, m + 1, m + 2], fill=TRACO)
            d.line([(4, 6), (m - 2, 3)], fill=TRACO, width=2)
            d.line([(m + 1, 3), (IW - 5, 6)], fill=TRACO, width=2)
        else:
            dx = {"Olho": 0, "OlhoEsquerda": -5, "OlhoDireita": 5}[nome]
            d.rectangle([m - 4 + dx, m - 3, m + 3 + dx, m + 5], fill=TRACO)
            d.rectangle([m - 3 + dx, m - 2, m - 2 + dx, m - 1], fill=(250, 250, 255, 255))
    elif nome in ("Poder0", "Poder1"):
        # O power-up: uma estrela de pixel, que pulsa entre os dois quadros.
        grande = nome == "Poder1"
        if grande:
            d.ellipse([0, 0, IW - 1, IW - 1], fill=(255, 230, 120, 90))
        estrela(d, m, m + 1, 11 if grande else 10, 4.5, cheia(AMARELO), escuro(AMARELO, 0.4))
        estrela(d, m, m + 1, 5, 2.2, claro(AMARELO, 0.7), None)
    return im


# ---- O disquete do save (34 x 34) ---------------------------------------------------------

SW = 34
SAVE = ["Save", "Foge", "Persegue"]


def save(nome):
    """O disquete do save, vivo: o de sempre; o que foge (vale mais que a nota de agora: verde, com
    olhos assustados); e o que persegue (vale menos: vermelho, com olhos bravos)."""
    im = Image.new("RGBA", (SW, SW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    corpo = {"Save": (60, 110, 220), "Foge": (60, 170, 110), "Persegue": (200, 60, 70)}[nome]
    d.rectangle([2, 2, SW - 3, SW - 3], fill=cheia(corpo), outline=TRACO, width=2)
    d.polygon([(SW - 3, 2), (SW - 3, 8), (SW - 9, 2)], fill=(0, 0, 0, 0))
    # A tampa de metal e a etiqueta.
    d.rectangle([9, 2, 24, 12], fill=(205, 210, 226, 255), outline=TRACO)
    d.rectangle([18, 4, 21, 10], fill=cheia(corpo))
    d.rectangle([7, 17, SW - 8, SW - 4], fill=TINTA, outline=TRACO)
    if nome == "Foge":
        for x in (12, 21):
            d.ellipse([x - 3, 20, x + 3, 26], fill=TINTA, outline=TRACO)
            d.rectangle([x - 1, 22, x, 23], fill=TRACO)
        d.arc([13, 26, 21, 31], 200, 340, fill=TRACO, width=1)
    elif nome == "Persegue":
        for x, inclina in ((12, 1), (21, -1)):
            d.rectangle([x - 2, 22, x + 1, 25], fill=TRACO)
            d.line([(x - 4, 20 - inclina), (x + 3, 20 + inclina)], fill=TRACO, width=2)
        d.line([(13, 29), (21, 29)], fill=TRACO, width=2)
    else:
        for y in (21, 25, 29):
            d.line([(10, y), (SW - 11, y)], fill=(150, 156, 180, 255))
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
        "Inimigo": {"Textura": [0], "Moeda": [1], "Olho": [2], "OlhoEsquerda": [3], "OlhoDireita": [4],
                    "Assustado": [5], "Poder": [6, 7]},
        "Save": {nome: [i] for i, nome in enumerate(SAVE)},
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
    grava_tira([inimigo(n) for n in INIMIGO], "DPIBHFerreiraInimigo", IW, IW, len(INIMIGO))
    grava_tira([save(n) for n in SAVE], "DPIBHFerreiraSave", SW, SW, len(SAVE))
    grava_tira([botao(n) for n in BOTOES], "DPIBHFerreiraBotoes", QW, QH, len(BOTOES))
    grava_tira([bug(0), bug(1)], "DPIBHFerreiraBugs", GW, GW, 2)
    grava_tira([bloco()], "DPIBHFerreiraBlocos", CW, CW, 1)
