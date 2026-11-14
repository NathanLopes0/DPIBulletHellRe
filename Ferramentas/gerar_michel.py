#!/usr/bin/env python3
"""Arte PROVISORIA do Michel (INF 429 - Aprendizado de Maquina com Dados Visuais,
optativa) e dos projeteis dele.

Mesmo estilo de cartao dos outros professores: e deliberadamente um
placeholder, para nao se passar por arte final. Trocar o PNG basta - o json de
quadro continua valendo.

O MOTIVO DO CARTAO E A CAMERA QUE VE EM CAMADAS: a lente, o filtro 3x3 por
cima dela e os mapas de features, cada vez menores, saindo de lado.

OS PROJETEIS (Assets/Teachers/Projectiles/DPIBHMichel*), uma folha por assunto:

    Cena       Casa Arvore Pessoa Carro        os objetos que ele arremessa na
               (e as quatro Invertidas)        cena, e a imagem de cada um do
               Borrao                          outro lado do furo, de cabeca
                                               para baixo; e a ponta do anel
                                               do desfoque
    Enfeites   Camera Reflexo ReflexoFora      a camera escura e o reflexo do
                                               professor, invertido (enfeite)
    Tiros      Seta Ativacao Vetor Leque       os tiros das quatro provas
               Sinal Atalho Saida Previsao
               Cerco Ruido Pixel
    Neuronios  Neuronio Aceso                  o neuronio da rede, e ele
                                               disparando
    Colegas    Colega0 .. Colega4              os colegas do MaVIlab, de costas
                                               como o aluno (enfeite)

    python3 Ferramentas/gerar_michel.py               # grava as folhas
    python3 Ferramentas/gerar_michel.py --animacoes   # o mapa de animacoes
"""

import json
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
SAIRA = os.path.join(RAIZ, "Assets", "Fonts", "SairaSemiCondensed-SemiBold.ttf")

CARTAO = (22, 28, 44, 255)
BORDA = (120, 190, 255, 255)
TINTA = (232, 240, 250, 255)
TRACO = (12, 14, 22, 255)
AMBAR = (255, 205, 90, 255)
CIANO = (110, 220, 255)
VIOLETA = (200, 140, 255)
PERIGO = (255, 80, 90)


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
            "meta": {"app": "gerar_michel.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def michel():
    """O cartao do professor: a lente, o filtro 3x3 e os mapas de features."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)
    # Os mapas de features, cada vez menores, saindo para a direita.
    for k, (x, y, lado) in enumerate([(70, 20, 40), (84, 30, 28), (96, 38, 18), (106, 43, 10)]):
        cor = claro(CIANO, 0.15 * k)
        d.rectangle([x, y, x + lado, y + lado], fill=escuro(CIANO, 0.25 + 0.1 * k), outline=cor)
    # A lente.
    d.ellipse([14, 16, 66, 68], fill=(30, 40, 64, 255), outline=TINTA, width=2)
    d.ellipse([24, 26, 56, 58], fill=(16, 22, 36, 255), outline=claro(CIANO, 0.3), width=2)
    d.ellipse([34, 36, 46, 48], fill=(8, 10, 18, 255))
    d.ellipse([28, 29, 35, 36], fill=(220, 240, 255, 200))
    # O filtro 3x3 por cima, em ambar.
    for i in range(4):
        d.line([(22 + 12 * i, 24), (22 + 12 * i, 60)], fill=AMBAR, width=1)
        d.line([(22, 24 + 12 * i), (58, 24 + 12 * i)], fill=AMBAR, width=1)
    centrado(d, "INF 429", 64, 92, fonte(12, True), AMBAR)
    centrado(d, "MICHEL", 64, 106, fonte(11), TINTA)
    centrado(d, "MaVIlab", 64, 117, fonte(8), claro(CIANO, 0.2))
    return im


# ---- A cena: os objetos e as imagens --------------------------------------------

SW = 20


def objeto(qual, cor, contorno):
    """Um pictograma de 20x20: a casa, a arvore, a pessoa ou o carro."""
    im = Image.new("RGBA", (SW, SW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if qual == "Casa":
        d.polygon([(3, 9), (10, 2), (17, 9)], fill=cor, outline=contorno)
        d.rectangle([5, 9, 15, 17], fill=cor, outline=contorno)
        d.rectangle([9, 12, 11, 17], fill=contorno)
    elif qual == "Arvore":
        d.ellipse([3, 1, 17, 13], fill=cor, outline=contorno)
        d.rectangle([9, 12, 11, 18], fill=contorno)
    elif qual == "Pessoa":
        d.ellipse([7, 1, 13, 7], fill=cor, outline=contorno)
        d.rectangle([7, 8, 13, 13], fill=cor, outline=contorno)
        d.line([(8, 13), (6, 18)], fill=contorno, width=2)
        d.line([(12, 13), (14, 18)], fill=contorno, width=2)
    else:  # Carro
        d.rectangle([2, 8, 18, 14], fill=cor, outline=contorno)
        d.polygon([(5, 8), (8, 4), (13, 4), (15, 8)], fill=cor, outline=contorno)
        d.ellipse([4, 12, 8, 16], fill=contorno)
        d.ellipse([12, 12, 16, 16], fill=contorno)
    return im


OBJETOS = ["Casa", "Arvore", "Pessoa", "Carro"]
INVERTIDOS = ["CasaInvertida", "ArvoreInvertida", "PessoaInvertida", "CarroInvertido"]


def objeto_na_cena(qual):
    # Na cena, o objeto e a luz: claro e frio.
    return objeto(qual, (200, 230, 255, 235), (90, 130, 180, 255))


def imagem(qual):
    # A imagem, do outro lado do furo: a mesma figura de cabeca para baixo, quente - e a que machuca.
    im = objeto(qual, PERIGO + (255,), (110, 20, 30, 255))
    return im.rotate(180)


def borrao():
    im = Image.new("RGBA", (SW, SW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for r, a in [(8, 70), (6, 140), (4, 230)]:
        d.ellipse([10 - r, 10 - r, 10 + r, 10 + r], fill=(255, 130, 150, a))
    return im


CENA = OBJETOS + INVERTIDOS + ["Borrao"]


def da_cena(nome):
    if nome == "Borrao":
        return borrao()
    if nome in INVERTIDOS:
        return imagem(OBJETOS[INVERTIDOS.index(nome)])
    return objeto_na_cena(nome)


# ---- Os enfeites: a camera e o reflexo ------------------------------------------

EW = 48


def camera():
    im = Image.new("RGBA", (EW, EW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 12, 43, 35], radius=4, fill=(40, 44, 58, 255), outline=(150, 160, 190, 255), width=2)
    d.rectangle([10, 8, 20, 12], fill=(60, 64, 80, 255), outline=(150, 160, 190, 255))
    d.ellipse([16, 16, 32, 32], fill=(12, 14, 22, 255), outline=claro(CIANO, 0.2), width=2)
    d.ellipse([22, 22, 26, 26], fill=(255, 250, 220, 255))
    return im


def reflexo(fora=False):
    # O professor do outro lado do furo: o cartao em miniatura, de cabeca para baixo e apagado.
    mini = michel().resize((40, 40), Image.LANCZOS).rotate(180)
    alfa = mini.getchannel("A").point(lambda a: int(a * (0.35 if fora else 0.6)))
    mini.putalpha(alfa)
    im = Image.new("RGBA", (EW, EW), (0, 0, 0, 0))
    im.alpha_composite(mini, (4, 4))
    if fora:
        # Fora do campo: a seta para baixo diz que ele esta abaixo do chao.
        d = ImageDraw.Draw(im)
        d.polygon([(18, 34), (30, 34), (24, 44)], fill=PERIGO + (220,))
    return im


ENFEITES = ["Camera", "Reflexo", "ReflexoFora"]


def enfeite(nome):
    if nome == "Camera":
        return camera()
    return reflexo(nome == "ReflexoFora")


# ---- Os tiros ---------------------------------------------------------------------

TW = 16
TIROS = ["Seta", "Ativacao", "Vetor", "Leque", "Sinal", "Atalho", "Saida", "Previsao", "Cerco", "Ruido", "Pixel"]


def bolinha(cor, raio=5, brilho=True):
    im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([8 - raio - 2, 8 - raio - 2, 8 + raio + 2, 8 + raio + 2], fill=cor + (70,))
    d.ellipse([8 - raio, 8 - raio, 8 + raio, 8 + raio], fill=cor + (255,), outline=escuro(cor))
    if brilho:
        d.point((6, 6), fill=(255, 255, 255, 255))
        d.point((7, 6), fill=(255, 255, 255, 200))
    return im


def tiro(tipo):
    if tipo == "Seta":
        im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.polygon([(2, 5), (9, 5), (9, 2), (15, 8), (9, 14), (9, 11), (2, 11)], fill=CIANO + (255,),
                  outline=escuro(CIANO))
        return im
    if tipo == "Ativacao":
        im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rectangle([2, 2, 13, 13], fill=(255, 160, 60, 255), outline=(120, 60, 10, 255))
        d.rectangle([5, 5, 10, 10], fill=(255, 220, 150, 255))
        return im
    if tipo == "Vetor":
        im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.ellipse([1, 1, 14, 14], outline=AMBAR, width=3)
        d.ellipse([6, 6, 9, 9], fill=(255, 255, 255, 255))
        return im
    if tipo == "Pixel":
        im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rectangle([2, 2, 13, 13], fill=PERIGO + (230,), outline=(110, 20, 30, 255))
        d.line([(4, 4), (8, 4)], fill=(255, 200, 200, 255))
        return im
    if tipo == "Saida":
        im = Image.new("RGBA", (TW, TW), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.polygon([(8, 1), (14, 8), (8, 15), (2, 8)], fill=(255, 150, 60, 255), outline=(110, 50, 10, 255))
        d.point((7, 5), fill=(255, 255, 255, 255))
        return im
    cores = {"Leque": (110, 160, 255), "Sinal": (150, 220, 255), "Atalho": (255, 205, 90),
             "Previsao": VIOLETA, "Cerco": PERIGO, "Ruido": (170, 175, 190)}
    raio = {"Ruido": 4, "Sinal": 4, "Atalho": 5}.get(tipo, 5)
    return bolinha(cores[tipo], raio, tipo != "Ruido")


# ---- Os neuronios ----------------------------------------------------------------

NW = 24


def neuronio(aceso):
    im = Image.new("RGBA", (NW, NW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = (255, 240, 160) if aceso else (120, 150, 255)
    if aceso:
        d.ellipse([0, 0, 23, 23], fill=(255, 230, 120, 90))
    d.ellipse([3, 3, 20, 20], fill=escuro(cor, 0.55), outline=cor + (255,), width=2)
    d.ellipse([7, 7, 16, 16], fill=cor + (255,))
    d.point((9, 9), fill=(255, 255, 255, 255))
    return im


# ---- Os colegas do MaVIlab --------------------------------------------------------

CW = 64
CABELOS = [(60, 40, 30), (230, 200, 120), (30, 30, 36), (170, 80, 50), (120, 120, 130)]
CAMISAS = [(90, 170, 255), (110, 210, 110), (250, 140, 80), (200, 140, 255), (255, 210, 90)]


def colega(k):
    """Um colega de costas, como o aluno: o cabelo, o jaleco branco e a cor da camisa por baixo."""
    im = Image.new("RGBA", (CW, CW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cabelo, camisa = CABELOS[k], CAMISAS[k]
    jaleco, sombra = (236, 240, 246, 255), (170, 176, 190, 255)
    # As pernas.
    d.rectangle([24, 48, 29, 58], fill=(50, 56, 80, 255))
    d.rectangle([34, 48, 39, 58], fill=(50, 56, 80, 255))
    # O jaleco, com a gola da camisa aparecendo, e os bracos.
    d.rectangle([20, 30, 43, 50], fill=jaleco, outline=sombra)
    d.line([(31, 31), (31, 50)], fill=sombra)
    d.rectangle([28, 29, 35, 32], fill=camisa + (255,))
    d.rectangle([16, 31, 20, 46], fill=jaleco, outline=sombra)
    d.rectangle([43, 31, 47, 46], fill=jaleco, outline=sombra)
    # A cabeca, de costas: quase tudo cabelo.
    d.ellipse([22, 10, 41, 31], fill=(240, 200, 170, 255))
    d.ellipse([21, 8, 42, 28], fill=cabelo + (255,))
    if k == 1:   # o rabo de cavalo
        d.ellipse([28, 24, 35, 33], fill=cabelo + (255,))
    if k == 3:   # o coque
        d.ellipse([27, 4, 36, 12], fill=cabelo + (255,))
    # O cracha do laboratorio, nas costas.
    d.rectangle([33, 36, 40, 41], fill=(60, 120, 200, 255))
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
            "meta": {"app": "gerar_michel.py", "version": "1.0", "image": f"{nome}.png",
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
        "Cena": {nome: [i] for i, nome in enumerate(CENA)},
        "Enfeites": {nome: [i] for i, nome in enumerate(ENFEITES)},
        "Tiros": {nome: [i] for i, nome in enumerate(TIROS)},
        "Neuronios": {"Neuronio": [0], "Aceso": [1]},
        "Colegas": {f"Colega{k}": [k] for k in range(len(CABELOS))},
    }


if __name__ == "__main__":
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(michel(), "DPIBHMichel", "Assets/Teachers")
    grava_tira([da_cena(n) for n in CENA], "DPIBHMichelCena", SW, SW, len(CENA))
    grava_tira([enfeite(n) for n in ENFEITES], "DPIBHMichelEnfeites", EW, EW, len(ENFEITES))
    grava_tira([tiro(t) for t in TIROS], "DPIBHMichelTiros", TW, TW, len(TIROS))
    grava_tira([neuronio(False), neuronio(True)], "DPIBHMichelNeuronios", NW, NW, 2)
    grava_tira([colega(k) for k in range(len(CABELOS))], "DPIBHMichelColegas", CW, CW, len(CABELOS))
