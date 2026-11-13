#!/usr/bin/env python3
"""Arte PROVISORIA do Arroyo (INF 332 - Projeto e Analise de Algoritmos) e dos
projeteis dele.

Mesmo estilo de cartao do Julio, do Thiago, do Vladimir, do Vitor, da Maria
Lucia, do Lucas e do Luizinho: e deliberadamente um placeholder, para nao se
passar por arte final. Trocar o PNG basta - o json de quadro continua valendo.

O MOTIVO DO CARTAO E A ARVORE DE RECURSAO: o professor que divide o problema
ate ele ficar pequeno, com o Teta do Teorema Mestre embaixo.

OS PROJETEIS (Assets/Teachers/Projectiles/DPIBHArroyo*), uma folha por assunto:

    No         N N2 N4 N3 Um   o no da arvore de recursao, com o tamanho do
                               subproblema escrito (DPIBHArroyoNos)
    Trabalho   C               o tiro de trabalho f(n), e o de cada folha
    Moeda      M100 .. M1      as moedas do troco guloso (DPIBHArroyoMoedas)
    Celula     V0..V9 C1..C9   a casa da tabela da programacao dinamica:
                               calculada, e a que casou letra com letra
    Gota       Gota            a chuva da tabela
    Rainha     Rainha          a rainha do backtracking (enfeite)
    Ataque     Ataque          o tiro da rainha
    Valor      V1..V6          o valor que desce a rede de ordenacao
    Cidade     Cidade Acesa    a cidade do caixeiro viajante
    Caixeiro   Caixeiro        o caixeiro, que corre a rota
    Solucao    Q4 Q3 Q2 Q1     a solucao da tempera, da quente a fria
    Vale       Vale            um otimo local (enfeite)

    python3 Ferramentas/gerar_arroyo.py               # grava as folhas
    python3 Ferramentas/gerar_arroyo.py --animacoes   # as animacoes de projeteis.json
"""
from PIL import Image, ImageDraw, ImageFont
import json, math, os, sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")

CARTAO = (36, 62, 70, 255)
BORDA = (126, 214, 190, 255)
TINTA = (232, 246, 240, 255)
TRACO = (14, 26, 30, 255)
FOLHA = (250, 214, 110, 255)


def fonte(tamanho, negrito=False):
    caminho = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Bold.ttf") if negrito else CARLITO
    try:
        return ImageFont.truetype(caminho if os.path.exists(caminho) else CARLITO, tamanho)
    except OSError:
        return ImageFont.load_default()


def centrado(d, texto, cx, cy, f, cor):
    caixa = d.textbbox((0, 0), texto, font=f)
    w, h = caixa[2] - caixa[0], caixa[3] - caixa[1]
    d.text((cx - w / 2 - caixa[0], cy - h / 2 - caixa[1]), texto, font=f, fill=cor)


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
    return {"frames": frames,
            "meta": {"app": "gerar_arroyo.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def arroyo():
    """O cartao do professor: a arvore de recursao e o Teta."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)
    # A arvore: a raiz, dois filhos, quatro netos - as arestas primeiro, os nos por cima.
    nos = [(64, 22)] + [(40, 46), (88, 46)] + [(26, 70), (54, 70), (74, 70), (102, 70)]
    for pai, filhos in ((0, (1, 2)), (1, (3, 4)), (2, (5, 6))):
        for f in filhos:
            d.line([nos[pai], nos[f]], fill=BORDA, width=2)
    for i, (x, y) in enumerate(nos):
        r = 9 if i == 0 else (7 if i < 3 else 6)
        d.ellipse([x - r, y - r, x + r, y + r], fill=TINTA if i < 3 else FOLHA, outline=TRACO, width=2)
    f = fonte(10, True)
    centrado(d, "n", 64, 22, f, TRACO)
    centrado(d, "n/2", 40, 46, fonte(8, True), TRACO)
    centrado(d, "n/2", 88, 46, fonte(8, True), TRACO)
    centrado(d, "Θ(n log n)", 64, 90, fonte(13, True), FOLHA)
    centrado(d, "ARROYO", 64, 108, fonte(11), TINTA)
    return im


# ---- A fase 1: a arvore de recursao ------------------------------------------

NW = 34   # o no: o colisor e metade disto

NOS = [("N", "n", (232, 246, 240)), ("N2", "n/2", (206, 232, 250)), ("N4", "n/4", (178, 214, 250)),
       ("N3", "n/3", (206, 232, 250)), ("Um", "1", (250, 214, 110))]


def no(texto, cor):
    im = Image.new("RGBA", (NW, NW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    escuro = tuple(int(c * 0.35) for c in cor)
    d.ellipse([1, 1, NW - 2, NW - 2], fill=escuro + (255,))
    d.ellipse([3, 3, NW - 4, NW - 4], fill=cor + (255,), outline=TRACO, width=2)
    d.arc([6, 6, NW - 7, NW - 7], 200, 290, fill=(255, 255, 255, 255), width=2)   # o brilho
    centrado(d, texto, NW / 2, NW / 2 + 1, fonte(13 if len(texto) < 3 else 11, True), TRACO)
    return im


def trabalho():
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([1, 1, 14, 14], fill=(250, 214, 110, 255), outline=(120, 80, 20, 255))
    d.ellipse([4, 4, 7, 7], fill=(255, 250, 220, 255))
    return im


# ---- A fase 2: os paradigmas -------------------------------------------------

MW = 30
MOEDAS = [100, 50, 25, 10, 5, 4, 3, 1]


def moeda(valor):
    im = Image.new("RGBA", (MW, MW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    ouro = valor in (100, 25, 4)
    face = (236, 196, 84) if ouro else (206, 212, 222)
    borda = (140, 96, 30) if ouro else (110, 116, 130)
    d.ellipse([1, 1, MW - 2, MW - 2], fill=borda + (255,))
    d.ellipse([3, 3, MW - 4, MW - 4], fill=face + (255,))
    d.ellipse([5, 5, MW - 6, MW - 6], outline=borda + (255,), width=1)
    d.arc([6, 6, MW - 7, MW - 7], 200, 280, fill=(255, 255, 240, 255), width=2)
    centrado(d, str(valor), MW / 2, MW / 2 + 1, fonte(12 if valor < 100 else 10, True), (60, 40, 16, 255))
    return im


CW, CH = 32, 24


def celula(valor, casou):
    im = Image.new("RGBA", (CW, CH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fundo = (60, 120, 70) if casou else (34, 52, 78)
    borda = (150, 240, 160) if casou else (110, 150, 200)
    d.rectangle([0, 0, CW - 1, CH - 1], fill=fundo + (235,), outline=borda + (255,))
    centrado(d, str(valor), CW / 2, CH / 2, fonte(15, True), (240, 250, 255, 255))
    return im


def gota():
    im = Image.new("RGBA", (14, 18), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(7, 1), (12, 10), (7, 16), (2, 10)], fill=(120, 190, 255, 255), outline=(30, 60, 120, 255))
    d.line([(5, 8), (6, 11)], fill=(230, 245, 255, 255))
    return im


RW = 34


def rainha():
    im = Image.new("RGBA", (RW, RW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor, escuro = (240, 120, 150), (90, 30, 50)
    # A coroa: cinco pontas sobre a base.
    pontas = [(5, 12), (10, 18), (17, 6), (24, 18), (29, 12)]
    d.polygon([(5, 26)] + pontas + [(29, 26)], fill=cor + (255,), outline=escuro + (255,))
    for (x, y) in pontas:
        d.ellipse([x - 2, y - 2, x + 2, y + 2], fill=(255, 230, 120, 255), outline=escuro + (255,))
    d.rectangle([4, 26, 30, 30], fill=cor + (255,), outline=escuro + (255,))
    d.line([(8, 22), (26, 22)], fill=(255, 200, 215, 255))
    return im


def ataque():
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(8, 1), (15, 8), (8, 15), (1, 8)], fill=(255, 140, 170, 255), outline=(110, 30, 60, 255))
    d.point((6, 6), fill=(255, 240, 245, 255))
    return im


# ---- A fase 3: a rede de ordenacao e o caixeiro --------------------------------

VW = 28


def valor(v):
    im = Image.new("RGBA", (VW, VW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # Cada valor numa cor, do frio (1) ao quente (6): ordenado, a fila vira um degrade.
    cores = [(90, 140, 255), (90, 200, 240), (110, 220, 150), (240, 220, 90), (250, 150, 70), (240, 80, 90)]
    c = cores[v - 1]
    d.rounded_rectangle([1, 1, VW - 2, VW - 2], radius=6, fill=c + (255,), outline=TRACO, width=2)
    d.line([(5, 5), (12, 5)], fill=(255, 255, 255, 255), width=2)
    centrado(d, str(v), VW / 2, VW / 2 + 1, fonte(16, True), TRACO)
    return im


CI = 30


def cidade(acesa):
    im = Image.new("RGBA", (CI, CI), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = (250, 214, 110) if acesa else (150, 200, 230)
    d.ellipse([2, 2, CI - 3, CI - 3], fill=(20, 30, 50, 230), outline=cor + (255,), width=3)
    d.ellipse([10, 10, CI - 11, CI - 11], fill=cor + (255,))
    return im


def caixeiro():
    im = Image.new("RGBA", (24, 24), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # A maleta.
    d.rectangle([3, 8, 20, 20], fill=(170, 100, 50, 255), outline=(60, 30, 10, 255))
    d.rectangle([8, 4, 15, 8], outline=(60, 30, 10, 255), width=2)
    d.line([(3, 13), (20, 13)], fill=(220, 170, 90, 255))
    return im


# ---- A repescagem: a tempera -------------------------------------------------

SW = 22


def solucao(calor):
    im = Image.new("RGBA", (SW, SW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cores = {4: (255, 90, 50), 3: (255, 170, 60), 2: (150, 210, 255), 1: (90, 130, 255)}
    c = cores[calor]
    d.ellipse([1, 1, SW - 2, SW - 2], fill=tuple(int(x * 0.4) for x in c) + (255,))
    d.ellipse([3, 3, SW - 4, SW - 4], fill=c + (255,))
    d.ellipse([6, 6, 10, 10], fill=(255, 255, 255, 230))
    return im


VALE_W, VALE_H = 96, 80


def vale():
    # O otimo local: os aneis no centro do quadro (o centro do projetil) e o nome embaixo.
    im = Image.new("RGBA", (VALE_W, VALE_H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cx, cy = VALE_W // 2, VALE_H // 2
    for r, a in ((22, 120), (15, 160), (8, 200)):
        d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=(160, 120, 255, a), width=2)
    centrado(d, "ótimo local", cx, cy + 32, fonte(13, True), (200, 175, 255, 235))
    return im


# ---- Gravar ------------------------------------------------------------------

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
            "meta": {"app": "gerar_arroyo.py", "version": "1.0", "image": f"{nome}.png",
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


CELULAS = [(f"V{v}", v, False) for v in range(10)] + [(f"C{v}", v, True) for v in range(1, 10)]


def animacoes():
    return {
        "No": {nome: [i] for i, (nome, _, _) in enumerate(NOS)},
        "Moeda": {f"M{v}": [i] for i, v in enumerate(MOEDAS)},
        "Celula": {nome: [i] for i, (nome, _, _) in enumerate(CELULAS)},
        "Valor": {f"V{v}": [v - 1] for v in range(1, 7)},
        "Cidade": {"Cidade": [0], "Acesa": [1]},
        "Solucao": {f"Q{c}": [i] for i, c in enumerate((4, 3, 2, 1))},
    }


if __name__ == "__main__":
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(arroyo(), "DPIBHArroyo", "Assets/Teachers")
    grava_tira([no(t, c) for _, t, c in NOS], "DPIBHArroyoNos", NW, NW, len(NOS))
    grava_tira([trabalho()], "DPIBHArroyoTrabalho", 16, 16, 1)
    grava_tira([moeda(v) for v in MOEDAS], "DPIBHArroyoMoedas", MW, MW, len(MOEDAS))
    grava_tira([celula(v, c) for _, v, c in CELULAS], "DPIBHArroyoCelulas", CW, CH, 10)
    grava_tira([gota()], "DPIBHArroyoGota", 14, 18, 1)
    grava_tira([rainha()], "DPIBHArroyoRainha", RW, RW, 1)
    grava_tira([ataque()], "DPIBHArroyoAtaque", 16, 16, 1)
    grava_tira([valor(v) for v in range(1, 7)], "DPIBHArroyoValores", VW, VW, 6)
    grava_tira([cidade(False), cidade(True)], "DPIBHArroyoCidades", CI, CI, 2)
    grava_tira([caixeiro()], "DPIBHArroyoCaixeiro", 24, 24, 1)
    grava_tira([solucao(c) for c in (4, 3, 2, 1)], "DPIBHArroyoSolucoes", SW, SW, 4)
    grava_tira([vale()], "DPIBHArroyoVale", VALE_W, VALE_H, 1)
