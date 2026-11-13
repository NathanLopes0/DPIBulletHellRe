#!/usr/bin/env python3
"""Arte PROVISORIA do Daniel (INF 425 - Ciencia de Dados, optativa) e dos
projeteis dele.

Mesmo estilo de cartao dos outros professores: e deliberadamente um
placeholder, para nao se passar por arte final. Trocar o PNG basta - o json de
quadro continua valendo.

O MOTIVO DO CARTAO E O GRAFICO DE DISPERSAO: tres grupos de pontos coloridos e
a fronteira de decisao tracejada entre eles.

OS PROJETEIS (Assets/Teachers/Projectiles/DPIBHDaniel*), uma folha por assunto:

    Dados      Fogo Agua Planta       os dados do catalogo dos monstrinhos: um
               Eletrico Pedra         bichinho redondo por especie (e por grupo,
               Outlier Nulo Cinza     na fase 2), o outlier espinhento, o nulo
                                      oco e o cinza, ja imputado
    Fontes     Sensor Planilha Api    de onde os dados vem (enfeite)
    Centroides C0 .. C4               o centroide de cada grupo das k-medias
    Arvores    A0 A1 A2               as tres arvores do comite (enfeite)
    Tiros      Estouro Engano Palpite os tiros: o caco do outlier e o da
               BarraMais BarraMenos   resposta errada, o palpite da caixa-preta,
               Previsao               as barras do SHAP e o anel da previsao
    Respostas  R0Q0O0 .. R2Q2O2       as cartas do sorteio: uma por opcao de
                                      cada pergunta (Chamada.cpp tem as mesmas)
    Caixa      Caixa                  a caixa-preta sobre o professor (enfeite)

Os monstrinhos sao bichinhos genericos de RPG - uma bolinha com olhos e um
detalhe do elemento -, de proposito: e so uma piscadela para o professor que
gosta do genero, sem personagem de ninguem.

    python3 Ferramentas/gerar_daniel.py               # grava as folhas
    python3 Ferramentas/gerar_daniel.py --animacoes   # o mapa de animacoes
"""

import json
import os
import sys

from PIL import Image, ImageDraw, ImageFont

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
SAIRA = os.path.join(RAIZ, "Assets", "Fonts", "SairaSemiCondensed-SemiBold.ttf")

CARTAO = (26, 34, 44, 255)
BORDA = (120, 220, 190, 255)
TINTA = (232, 244, 240, 255)
TRACO = (14, 18, 24, 255)
AMBAR = (255, 205, 90, 255)

# As especies (e os grupos): fogo, agua, planta, eletrico, pedra. As mesmas de Daniel::kCores.
CORES = [(255, 130, 70), (90, 170, 255), (110, 210, 110), (250, 220, 70), (190, 150, 110)]
ESPECIES = ["Fogo", "Agua", "Planta", "Eletrico", "Pedra"]


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


def quadro_json(nome, w, h):
    return {"frames": [{
                "filename": f"{nome} 0.placeholder",
                "frame": {"x": 0, "y": 0, "w": w, "h": h},
                "rotated": False, "trimmed": False,
                "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
                "sourceSize": {"w": w, "h": h},
                "duration": 120}],
            "meta": {"app": "gerar_daniel.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def daniel():
    """O cartao do professor: o grafico de dispersao com tres grupos e a fronteira."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)
    # Os eixos.
    d.line([(18, 14), (18, 84), (112, 84)], fill=TINTA, width=1)
    for k in range(4):
        d.line([(18 + 24 * (k + 1), 84), (18 + 24 * (k + 1), 86)], fill=TINTA)
        d.line([(16, 84 - 17 * (k + 1)), (18, 84 - 17 * (k + 1))], fill=TINTA)
    # Tres grupos de pontos, cada um na cor da especie.
    grupos = [((40, 64), CORES[0]), ((86, 66), CORES[1]), ((64, 32), CORES[2])]
    desvios = [(-7, -3), (5, 4), (-2, 7), (8, -6), (0, 0), (-6, 6), (6, 1), (2, -8)]
    for (cx, cy), cor in grupos:
        for dx, dy in desvios:
            x, y = cx + dx, cy + dy
            d.ellipse([x - 2, y - 2, x + 2, y + 2], fill=cor + (255,), outline=escuro(cor))
    # A fronteira de decisao, tracejada.
    for k in range(9):
        x0 = 22 + 10 * k
        d.line([(x0, 18 + 7 * k), (x0 + 5, 18 + 7 * k + 3)], fill=AMBAR, width=2)
    centrado(d, "INF 425", 64, 98, fonte(12, True), AMBAR)
    centrado(d, "DANIEL", 64, 111, fonte(11), TINTA)
    return im


# ---- Os dados: os monstrinhos ---------------------------------------------------

DW = 16


def bicho(especie):
    """Um monstrinho redondo: corpo, barriga clara, dois olhos e o detalhe do elemento."""
    cor = CORES[ESPECIES.index(especie)]
    im = Image.new("RGBA", (DW, DW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([2, 4, 13, 15], fill=cor + (255,), outline=escuro(cor), width=1)
    d.ellipse([5, 10, 10, 14], fill=claro(cor, 0.5))
    d.point((6, 8), fill=TRACO)
    d.point((9, 8), fill=TRACO)
    d.point((6, 7), fill=(255, 255, 255, 255))
    d.point((9, 7), fill=(255, 255, 255, 255))
    if especie == "Fogo":       # o topete de chama
        d.polygon([(6, 5), (8, 0), (9, 3), (11, 1), (10, 5)], fill=(255, 220, 90, 255), outline=escuro(cor))
    elif especie == "Agua":     # a ponta de gota
        d.polygon([(6, 5), (8, 0), (10, 5)], fill=claro(cor, 0.35), outline=escuro(cor))
    elif especie == "Planta":   # o broto
        d.line([(8, 5), (8, 2)], fill=(60, 120, 50, 255))
        d.ellipse([8, 0, 12, 3], fill=(150, 235, 120, 255), outline=(50, 110, 40, 255))
    elif especie == "Eletrico": # o raio
        d.line([(9, 0), (7, 3), (10, 3), (8, 6)], fill=(255, 250, 170, 255), width=1)
    else:                       # Pedra: as lascas
        d.polygon([(4, 6), (6, 2), (8, 5)], fill=(150, 120, 90, 255), outline=escuro(cor))
        d.polygon([(8, 5), (10, 1), (12, 6)], fill=(170, 140, 105, 255), outline=escuro(cor))
    return im


def outlier():
    im = Image.new("RGBA", (DW, DW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    pontas = []
    for k in range(16):
        import math
        a = math.pi * 2 * k / 16
        r = 7.5 if k % 2 == 0 else 4.0
        pontas.append((8 + r * math.cos(a), 8 + r * math.sin(a)))
    d.polygon(pontas, fill=(220, 70, 120, 255), outline=(90, 20, 50, 255))
    d.point((6, 7), fill=TRACO)
    d.point((10, 7), fill=TRACO)
    d.line([(6, 10), (10, 10)], fill=TRACO)
    return im


def nulo():
    im = Image.new("RGBA", (DW, DW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([1, 1, 14, 14], outline=(200, 210, 230, 255), width=2)
    centrado(d, "?", 8, 8, fonte(10, True), (200, 210, 230, 255))
    return im


def cinza():
    im = bicho("Agua")
    # O imputado: o mesmo bicho, sem cor - a media de todos.
    px = im.load()
    for y in range(DW):
        for x in range(DW):
            r, g, b, a = px[x, y]
            m = int(0.3 * r + 0.59 * g + 0.11 * b)
            px[x, y] = (m, m, m + 10 if m < 245 else m, a)
    return im


DADOS = ESPECIES + ["Outlier", "Nulo", "Cinza"]


def dado(nome):
    if nome == "Outlier":
        return outlier()
    if nome == "Nulo":
        return nulo()
    if nome == "Cinza":
        return cinza()
    return bicho(nome)


# ---- As fontes, os centroides e as arvores --------------------------------------

FW, FH = 40, 32


def fonte_de_dados(qual):
    im = Image.new("RGBA", (FW, FH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1, 1, FW - 2, FH - 2], radius=6, fill=(34, 46, 60, 235), outline=BORDA, width=1)
    if qual == "Sensor":
        d.ellipse([14, 4, 26, 16], outline=AMBAR, width=2)
        d.arc([9, 0, 31, 20], 200, 340, fill=AMBAR)
        d.line([(20, 16), (20, 20)], fill=AMBAR, width=2)
    elif qual == "Planilha":
        d.rectangle([9, 5, 31, 23], fill=(230, 240, 235, 255), outline=(60, 120, 90, 255))
        for y in (11, 17):
            d.line([(9, y), (31, y)], fill=(60, 120, 90, 255))
        for x in (16, 23):
            d.line([(x, 5), (x, 23)], fill=(60, 120, 90, 255))
    else:  # Api
        centrado(d, "{ }", FW / 2, 13, fonte(14, True), (150, 210, 255, 255))
    centrado(d, {"Sensor": "sensor", "Planilha": "csv", "Api": "API"}[qual], FW / 2, FH - 6, fonte(8), TINTA)
    return im


CW = 28


def centroide(k):
    cor = CORES[k]
    im = Image.new("RGBA", (CW, CW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([2, 2, CW - 3, CW - 3], fill=cor + (90,), outline=cor + (255,), width=2)
    d.ellipse([9, 9, CW - 10, CW - 10], fill=cor + (255,), outline=escuro(cor), width=1)
    d.line([(CW / 2, 0), (CW / 2, 6)], fill=cor + (255,), width=2)
    d.line([(CW / 2, CW - 7), (CW / 2, CW - 1)], fill=cor + (255,), width=2)
    d.line([(0, CW / 2), (6, CW / 2)], fill=cor + (255,), width=2)
    d.line([(CW - 7, CW / 2), (CW - 1, CW / 2)], fill=cor + (255,), width=2)
    return im


AW, AH = 24, 28
CORES_DAS_ARVORES = [(150, 230, 150), (240, 200, 120), (170, 190, 255)]


def arvore(k):
    cor = CORES_DAS_ARVORES[k]
    im = Image.new("RGBA", (AW, AH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # Uma arvore de decisao: o no de cima, dois filhos e quatro folhas.
    d.line([(12, 6), (6, 14), (3, 22)], fill=cor + (255,), width=1)
    d.line([(6, 14), (9, 22)], fill=cor + (255,), width=1)
    d.line([(12, 6), (18, 14), (15, 22)], fill=cor + (255,), width=1)
    d.line([(18, 14), (21, 22)], fill=cor + (255,), width=1)
    for (x, y, r) in ((12, 5, 3), (6, 14, 2), (18, 14, 2)):
        d.ellipse([x - r, y - r, x + r, y + r], fill=cor + (255,), outline=escuro(cor))
    for i, x in enumerate((3, 9, 15, 21)):
        folha = CORES[0] if i in (0, 3) else CORES[1]
        d.rectangle([x - 2, 22, x + 2, 26], fill=folha + (255,), outline=escuro(folha))
    return im


# ---- Os tiros ----------------------------------------------------------------------

TW = 12
TIROS = ["Estouro", "Engano", "Palpite", "BarraMais", "BarraMenos", "Previsao"]


def tiro(tipo):
    im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if tipo == "Estouro":
        d.polygon([(6, 0), (8, 4), (12, 6), (8, 8), (6, 12), (4, 8), (0, 6), (4, 4)],
                  fill=(240, 90, 140, 255), outline=(100, 20, 60, 255))
    elif tipo == "Engano":
        d.polygon([(1, 2), (10, 1), (11, 6), (6, 11), (2, 8)], fill=(200, 200, 215, 255), outline=(120, 40, 60, 255))
        d.line([(3, 4), (8, 8)], fill=(220, 60, 80, 255), width=2)
    elif tipo == "Palpite":
        d.polygon([(6, 0), (12, 6), (6, 12), (0, 6)], fill=(70, 40, 110, 255), outline=(200, 170, 255, 255))
        d.rectangle([5, 5, 7, 7], fill=(240, 230, 255, 255))
    elif tipo == "BarraMais":
        d.rectangle([0, 2, 11, 9], fill=(255, 90, 90, 255), outline=(110, 20, 20, 255))
        d.line([(3, 6), (8, 6)], fill=(255, 230, 230, 255))
        d.line([(6, 4), (6, 8)], fill=(255, 230, 230, 255))
    elif tipo == "BarraMenos":
        d.rectangle([0, 2, 11, 9], fill=(80, 150, 255, 255), outline=(20, 40, 110, 255))
        d.line([(3, 6), (8, 6)], fill=(230, 240, 255, 255))
    else:  # Previsao
        d.ellipse([0, 0, 11, 11], fill=AMBAR, outline=(120, 80, 10, 255))
        d.ellipse([4, 4, 7, 7], fill=(255, 255, 230, 255))
    return im


# ---- As cartas do sorteio ------------------------------------------------------------
# As opcoes sao as de Source/Attacks/Chamada.cpp, na mesma ordem: a 0 e a certa.

AULAS = [
    [["média 0 e desvio 1", "tudo entre 0 e 1", "apaga os nulos"],
     ["imputa: vira a média", "vira um outlier", "fica como está"],
     ["o z-score passa de 2", "é o dado mais novo", "é o que veio da API"]],
    [["para a média do grupo", "para o ponto mais longe", "para o canto do campo"],
     ["de maior variância", "de menor variância", "do professor"],
     ["pelo cotovelo da inércia", "é sempre 3", "pelo número de alunos"]],
    [["a maioria dos votos", "a árvore mais funda", "a última plantada"],
     ["supervisionado", "não supervisionado", "por sorteio"],
     ["os erros", "os acertos", "a média dos rótulos"]],
]

RW, RH = 176, 38


def resposta(texto):
    im = Image.new("RGBA", (RW, RH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1, 1, RW - 2, RH - 2], radius=8, fill=(244, 238, 218, 255), outline=(150, 110, 50, 255), width=2)
    d.rounded_rectangle([4, 4, RW - 5, RH - 5], radius=6, outline=(220, 200, 160, 255), width=1)
    f = fonte(15, True)
    centrado(d, texto, RW / 2, RH / 2, f, (40, 30, 20, 255))
    return im


def respostas():
    quadros, nomes = [], []
    for a, aula in enumerate(AULAS):
        for q, opcoes in enumerate(aula):
            for o, texto in enumerate(opcoes):
                quadros.append(resposta(texto))
                nomes.append(f"R{a}Q{q}O{o}")
    return quadros, nomes


XW, XH = 56, 44


def caixa():
    im = Image.new("RGBA", (XW, XH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([1, 1, XW - 2, XH - 2], fill=(12, 12, 16, 245), outline=(90, 80, 120, 255), width=2)
    centrado(d, "?", XW / 2, XH / 2 - 2, fonte(26, True), (190, 170, 255, 255))
    return im


# ---- Gravar ----------------------------------------------------------------------------

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
            "meta": {"app": "gerar_daniel.py", "version": "1.0", "image": f"{nome}.png",
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


FONTES = ["Sensor", "Planilha", "Api"]


def animacoes():
    _, nomes = respostas()
    return {
        "Dados": {nome: [i] for i, nome in enumerate(DADOS)},
        "Fontes": {nome: [i] for i, nome in enumerate(FONTES)},
        "Centroides": {f"C{k}": [k] for k in range(len(CORES))},
        "Arvores": {f"A{k}": [k] for k in range(3)},
        "Tiros": {nome: [i] for i, nome in enumerate(TIROS)},
        "Respostas": {nome: [i] for i, nome in enumerate(nomes)},
        "Caixa": {"Caixa": [0]},
    }


if __name__ == "__main__":
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(daniel(), "DPIBHDaniel", "Assets/Teachers")
    grava_tira([dado(n) for n in DADOS], "DPIBHDanielDados", DW, DW, len(DADOS))
    grava_tira([fonte_de_dados(q) for q in FONTES], "DPIBHDanielFontes", FW, FH, len(FONTES))
    grava_tira([centroide(k) for k in range(len(CORES))], "DPIBHDanielCentroides", CW, CW, len(CORES))
    grava_tira([arvore(k) for k in range(3)], "DPIBHDanielArvores", AW, AH, 3)
    grava_tira([tiro(t) for t in TIROS], "DPIBHDanielTiros", TW, TW, len(TIROS))
    quadros, _ = respostas()
    grava_tira(quadros, "DPIBHDanielRespostas", RW, RH, 3)
    grava_tira([caixa()], "DPIBHDanielCaixa", XW, XH, 1)
