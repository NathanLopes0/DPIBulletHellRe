#!/usr/bin/env python3
"""Arte PROVISORIA da Sabrina (INF 491 - Topicos Especiais: Bioinformatica,
optativa) e dos projeteis dela.

Mesmo estilo de cartao dos outros professores: e deliberadamente um
placeholder, para nao se passar por arte final. Trocar o PNG basta - o json de
quadro continua valendo.

O MOTIVO DO CARTAO E A DUPLA HELICE EM 3D: as duas fitas, a da frente clara e a
de tras apagada, os degraus na cor de cada base, e uma molecula de bolas e
varetas no canto.

AS CORES DAS BASES sao as dos cromatogramas: A verde, C azul, G ambar, T e U
vermelho (as mesmas de Sabrina::CorDa).

OS PROJETEIS (Assets/Teachers/Projectiles/DPIBHSabrina*), uma folha por assunto:

    Bases       A C G T U                   as bases que ela atira
                LA LC LG LT LU              as letras das fitas no alto (enfeite)
    Capsulas    CA CC CG CT CU              o nucleotideo livre que troca a base
                                            do aluno (enfeite)
    Selos       SA SC SG ST SU              o selo da base que o aluno carrega
    Moleculas   Fita0 Fita1                 as contas das duas fitas da helice
                Hidrofobico Polar           os residuos e os atomos da proteina
                Positivo Negativo
                Solto Agua Inibidor Sitio   o residuo solto, a agua, o inibidor
                                            e a parede do sitio ativo (abrigo)
    Genomica    Dupla Simples Primer Read   a PCR e o shotgun
                Contig Ponta
                GA GC GG GT Elo             a fita do genoma do CRISPR e o
                                            esqueleto entre as letras
                Estilhaco
    Maquinas    Polimerase Ribossomo        as maquinas da celula (enfeite)
                Tesoura

    python3 Ferramentas/gerar_sabrina.py               # grava as folhas
    python3 Ferramentas/gerar_sabrina.py --animacoes   # o mapa de animacoes
"""

import json
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
SAIRA = os.path.join(RAIZ, "Assets", "Fonts", "SairaSemiCondensed-SemiBold.ttf")

CARTAO = (20, 30, 40, 255)
BORDA = (110, 230, 190, 255)
TINTA = (232, 240, 250, 255)
TRACO = (12, 14, 22, 255)
AMBAR = (255, 205, 90, 255)

# As bases, como nos cromatogramas.
CORES = {"A": (110, 220, 120), "C": (100, 170, 255), "G": (255, 200, 70), "T": (255, 100, 100),
         "U": (255, 100, 100)}
BASES = ["A", "C", "G", "T", "U"]
FAMILIAS = {"Hidrofobico": (255, 210, 90), "Polar": (120, 230, 160), "Positivo": (110, 160, 255),
            "Negativo": (255, 110, 110)}


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
            "meta": {"app": "gerar_sabrina.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def esfera(d, cx, cy, r, cor, contorno=None):
    """Uma bolinha com brilho: o desenho dos modelos de bolas e varetas."""
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=escuro(cor, 0.55), outline=contorno or escuro(cor, 0.3))
    d.ellipse([cx - r + 1, cy - r + 1, cx + r - 2, cy + r - 2], fill=cor + (255,) if len(cor) == 3 else cor)
    d.ellipse([cx - r * 0.55, cy - r * 0.6, cx - r * 0.05, cy - r * 0.1], fill=claro(cor, 0.65))


def sabrina():
    """O cartao da professora: a dupla helice em 3D e uma molecula de bolas e varetas."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)
    # A dupla helice, de pe, na esquerda: primeiro o que esta atras, depois a frente.
    pontos = []
    for i in range(15):
        y = 14 + i * 4.8
        for fita, fase in ((0, 0.0), (1, 2.5)):
            a = 0.62 * i + fase
            x = 34 + 18 * math.cos(a)
            z = math.sin(a)
            pontos.append((z, x, y, fita, i))
    for i in range(15):
        y = 14 + i * 4.8
        a0, a1 = 0.62 * i, 0.62 * i + 2.5
        x0, x1 = 34 + 18 * math.cos(a0), 34 + 18 * math.cos(a1)
        base = "ACGT"[(i * 7) % 4]
        par = {"A": "T", "T": "A", "C": "G", "G": "C"}[base]
        d.line([(x0, y), ((x0 + x1) / 2, y)], fill=CORES[base] + (220,), width=2)
        d.line([((x0 + x1) / 2, y), (x1, y)], fill=CORES[par] + (220,), width=2)
    for z, x, y, fita, i in sorted(pontos):
        cor = (170, 200, 255) if fita == 0 else (200, 160, 255)
        r = 2.4 + 1.4 * (z + 1) / 2
        if z < 0:
            d.ellipse([x - r, y - r, x + r, y + r], outline=cor + (110,))
        else:
            esfera(d, x, y, r, cor)
    # A molecula no canto: um anel de seis e duas varetas.
    cx, cy = 88, 40
    anel = [(cx + 15 * math.cos(math.pi / 3 * k + 0.5), cy + 15 * math.sin(math.pi / 3 * k + 0.5)) for k in range(6)]
    for k in range(6):
        d.line([anel[k], anel[(k + 1) % 6]], fill=(200, 205, 220, 255), width=2)
    d.line([anel[0], (cx + 28, cy + 16)], fill=(200, 205, 220, 255), width=2)
    d.line([anel[3], (cx - 26, cy - 14)], fill=(200, 205, 220, 255), width=2)
    for k, (x, y) in enumerate(anel):
        esfera(d, x, y, 4, (120, 230, 160) if k % 3 else (110, 160, 255))
    esfera(d, cx + 28, cy + 16, 4, (255, 110, 110))
    esfera(d, cx - 26, cy - 14, 4, (255, 210, 90))
    centrado(d, "INF 491", 64, 92, fonte(12, True), AMBAR)
    centrado(d, "SABRINA", 64, 106, fonte(11), TINTA)
    centrado(d, "Bioinfo", 64, 117, fonte(8), claro(BORDA, 0.2))
    return im


# ---- As bases ---------------------------------------------------------------------

BW = 20


def base(letra):
    """A base que ela atira: o nucleotideo (um pentagono, o acucar) na cor da base, com a letra."""
    im = Image.new("RGBA", (BW, BW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = CORES[letra]
    c, r = 9.5, 9.0
    pent = [(c + r * math.cos(-math.pi / 2 + 2 * math.pi * k / 5), c + 1 + r * math.sin(-math.pi / 2 + 2 * math.pi * k / 5))
            for k in range(5)]
    d.polygon(pent, fill=escuro(cor, 0.5), outline=TRACO)
    pent2 = [(c + (x - c) * 0.8, c + 1 + (y - c - 1) * 0.8) for x, y in pent]
    d.polygon(pent2, fill=cor + (255,))
    centrado(d, letra, c, c + 1.5, fonte(11, True), TRACO)
    return im


def letra_da_fita(letra):
    """A letra de uma fita no alto (enfeite): o quadradinho escuro com a letra na cor da base."""
    im = Image.new("RGBA", (BW, BW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = CORES[letra]
    d.rounded_rectangle([1, 1, 18, 18], radius=4, fill=(24, 30, 44, 235), outline=cor + (200,))
    centrado(d, letra, 9.5, 10, fonte(12, True), cor + (255,))
    return im


# ---- As capsulas e os selos -------------------------------------------------------

KW = 30


def capsula(letra):
    """O nucleotideo livre: um anel brilhante e oco (nao e tiro), com a letra no meio."""
    im = Image.new("RGBA", (KW, KW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = CORES[letra]
    for k, a in enumerate((60, 110, 170)):
        r = 14 - 2 * k
        d.ellipse([15 - r, 15 - r, 15 + r, 15 + r], outline=cor + (a,), width=2)
    d.ellipse([8, 8, 22, 22], fill=(20, 26, 40, 220))
    centrado(d, letra, 15, 15.5, fonte(13, True), claro(cor, 0.2))
    # Os brilhinhos de quem se pega.
    for x, y in ((4, 6), (25, 5), (26, 24)):
        d.line([(x - 2, y), (x + 2, y)], fill=(255, 255, 255, 200))
        d.line([(x, y - 2), (x, y + 2)], fill=(255, 255, 255, 200))
    return im


SW = 18


def selo(letra):
    """O selo da base que o aluno carrega: um escudinho com a letra."""
    im = Image.new("RGBA", (SW, SW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = CORES[letra]
    d.polygon([(2, 2), (15, 2), (15, 10), (8.5, 16), (2, 10)], fill=(20, 24, 36, 240), outline=cor + (255,))
    centrado(d, letra, 8.5, 8, fonte(11, True), claro(cor, 0.15))
    return im


# ---- As moleculas -----------------------------------------------------------------

MW = 20
MOLECULAS = ["Fita0", "Fita1", "Hidrofobico", "Polar", "Positivo", "Negativo", "Solto", "Agua", "Inibidor", "Sitio"]


def molecula(nome):
    im = Image.new("RGBA", (MW, MW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if nome == "Fita0":
        esfera(d, 10, 10, 7.5, (170, 205, 255))
    elif nome == "Fita1":
        esfera(d, 10, 10, 7.5, (205, 165, 255))
    elif nome in FAMILIAS:
        esfera(d, 10, 10, 7.5, FAMILIAS[nome])
        if nome == "Positivo":
            d.line([(7, 10.5), (13, 10.5)], fill=TRACO, width=2)
            d.line([(10, 7.5), (10, 13.5)], fill=TRACO, width=2)
        elif nome == "Negativo":
            d.line([(7, 10.5), (13, 10.5)], fill=TRACO, width=2)
    elif nome == "Solto":
        d.ellipse([4, 4, 16, 16], outline=(200, 190, 170, 200), width=2)
        d.ellipse([8, 8, 12, 12], fill=(200, 190, 170, 160))
    elif nome == "Agua":
        # A gota: o oxigenio e os dois hidrogenios.
        d.polygon([(10, 2), (15, 11), (10, 17), (5, 11)], fill=(90, 200, 255, 255), outline=(30, 90, 140, 255))
        d.ellipse([7.5, 8, 12.5, 13], fill=(200, 240, 255, 255))
    elif nome == "Inibidor":
        hexa = [(10 + 8 * math.cos(math.pi / 3 * k), 10 + 8 * math.sin(math.pi / 3 * k)) for k in range(6)]
        d.polygon(hexa, fill=(200, 50, 70, 255), outline=(255, 160, 170, 255))
        d.line([(6, 6), (14, 14)], fill=(255, 220, 220, 255), width=2)
    elif nome == "Sitio":
        d.ellipse([2, 2, 18, 18], fill=(150, 100, 220, 120), outline=(220, 180, 255, 255), width=2)
        d.ellipse([6, 6, 12, 12], fill=(230, 210, 255, 200))
    return im


# ---- A genomica -------------------------------------------------------------------

GW = 24
GENOMICA = ["Dupla", "Simples", "Primer", "Read", "Contig", "Ponta", "GA", "GC", "GG", "GT", "Estilhaco", "Elo"]


def genomica(nome):
    """Os da repescagem. Os que viram pela velocidade (copia, read) apontam para a direita."""
    im = Image.new("RGBA", (GW, GW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if nome in ("Dupla", "Simples"):
        fitas = (0, math.pi) if nome == "Dupla" else (0,)
        for k, fase in enumerate(fitas):
            pts = [(x, 12 + 5 * math.sin(x * 0.55 + fase)) for x in range(2, 23)]
            d.line(pts, fill=(170, 205, 255, 255) if k == 0 else (205, 165, 255, 255), width=3)
        if nome == "Dupla":
            for x in range(4, 22, 4):
                base = "ACGT"[(x // 4) % 4]
                d.line([(x, 12 + 5 * math.sin(x * 0.55)), (x, 12 + 5 * math.sin(x * 0.55 + math.pi))],
                       fill=CORES[base] + (230,), width=1)
    elif nome == "Primer":
        d.polygon([(3, 8), (15, 8), (21, 12), (15, 16), (3, 16)], fill=(255, 220, 90, 255), outline=(120, 80, 10, 255))
        centrado(d, "P", 10, 12, fonte(9, True), TRACO)
    elif nome == "Read":
        d.rounded_rectangle([1, 8, 22, 16], radius=3, fill=(40, 48, 66, 255), outline=(220, 230, 255, 255))
        for k, b in enumerate("GATC"):
            d.rectangle([3 + k * 5, 10, 6 + k * 5, 14], fill=CORES[b] + (255,))
    elif nome == "Contig":
        esfera(d, 12, 12, 8, (120, 170, 255))
    elif nome == "Ponta":
        esfera(d, 12, 12, 8, (255, 160, 70))
        d.ellipse([2, 2, 22, 22], outline=(255, 220, 140, 200), width=1)
    elif nome.startswith("G") and len(nome) == 2:
        letra = nome[1]
        cor = CORES[letra]
        d.rounded_rectangle([2, 2, 21, 21], radius=5, fill=escuro(cor, 0.55), outline=claro(cor, 0.2), width=2)
        centrado(d, letra, 11.5, 12, fonte(14, True), claro(cor, 0.5))
    elif nome == "Estilhaco":
        d.polygon([(4, 12), (12, 4), (20, 9), (13, 20)], fill=(255, 140, 90, 255), outline=(120, 40, 20, 255))
    elif nome == "Elo":
        # O esqueleto de acucar e fosfato entre duas letras do genoma.
        d.rounded_rectangle([0, 8, 23, 15], radius=3, fill=(70, 84, 120, 255), outline=(150, 170, 220, 255))
        d.ellipse([8, 7, 15, 16], fill=(200, 160, 255, 255), outline=(90, 60, 140, 255))
    return im


# ---- As maquinas ------------------------------------------------------------------

QW = 48
MAQUINAS = ["Polimerase", "Ribossomo", "Tesoura"]


def maquina(nome):
    im = Image.new("RGBA", (QW, QW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if nome == "Polimerase":
        # A enzima: uma mao fechada sobre a fita.
        d.ellipse([4, 8, 44, 40], fill=(60, 170, 160, 230), outline=(180, 255, 240, 255), width=2)
        d.ellipse([12, 14, 30, 30], fill=(90, 210, 190, 255))
        d.rectangle([2, 22, 46, 26], fill=(20, 30, 40, 200))
        centrado(d, "pol", 24, 34, fonte(10, True), TRACO)
    elif nome == "Ribossomo":
        # As duas subunidades: a grande em cima, a pequena embaixo.
        d.ellipse([3, 2, 45, 30], fill=(190, 150, 110, 235), outline=(255, 225, 190, 255), width=2)
        d.ellipse([9, 26, 39, 46], fill=(160, 120, 90, 235), outline=(255, 225, 190, 255), width=2)
        for x in (15, 24, 33):
            d.ellipse([x - 3, 13, x + 3, 19], fill=(110, 80, 60, 255))
    else:
        # A Cas9 com a tesoura.
        d.ellipse([4, 4, 44, 44], fill=(140, 90, 200, 200), outline=(230, 200, 255, 255), width=2)
        d.ellipse([12, 26, 22, 36], outline=TINTA, width=3)
        d.ellipse([26, 26, 36, 36], outline=TINTA, width=3)
        d.line([(19, 26), (32, 8)], fill=TINTA, width=3)
        d.line([(29, 26), (16, 8)], fill=TINTA, width=3)
    return im


# ---- Gravar ----------------------------------------------------------------------

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
            "meta": {"app": "gerar_sabrina.py", "version": "1.0", "image": f"{nome}.png",
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
        "Bases": {**{b: [i] for i, b in enumerate(BASES)}, **{"L" + b: [5 + i] for i, b in enumerate(BASES)}},
        "Capsulas": {"C" + b: [i] for i, b in enumerate(BASES)},
        "Selos": {"S" + b: [i] for i, b in enumerate(BASES)},
        "Moleculas": {nome: [i] for i, nome in enumerate(MOLECULAS)},
        "Genomica": {nome: [i] for i, nome in enumerate(GENOMICA)},
        "Maquinas": {nome: [i] for i, nome in enumerate(MAQUINAS)},
    }


if __name__ == "__main__":
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(sabrina(), "DPIBHSabrina", "Assets/Teachers")
    grava_tira([base(b) for b in BASES] + [letra_da_fita(b) for b in BASES], "DPIBHSabrinaBases", BW, BW, 10)
    grava_tira([capsula(b) for b in BASES], "DPIBHSabrinaCapsulas", KW, KW, len(BASES))
    grava_tira([selo(b) for b in BASES], "DPIBHSabrinaSelos", SW, SW, len(BASES))
    grava_tira([molecula(n) for n in MOLECULAS], "DPIBHSabrinaMoleculas", MW, MW, len(MOLECULAS))
    grava_tira([genomica(n) for n in GENOMICA], "DPIBHSabrinaGenomica", GW, GW, len(GENOMICA))
    grava_tira([maquina(n) for n in MAQUINAS], "DPIBHSabrinaMaquinas", QW, QW, len(MAQUINAS))
