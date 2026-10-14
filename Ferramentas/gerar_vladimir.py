#!/usr/bin/env python3
"""Arte PROVISORIA do Vladimir (INF 331) e dos projeteis dele.

Mesmo estilo de cartao do DPIBHJulio.png e do DPIBHThiago.png: e deliberadamente
um placeholder, para nao se passar por arte final. Trocar o PNG basta - o json
de quadro continua valendo.

O MOTIVO DO CARTAO E UM AUTOMATO: dois estados, uma transicao, e o da direita
com circulo duplo (estado de aceitacao). E o desenho que qualquer pessoa que
cursou Linguagens Formais reconhece de longe, e diz a materia sem escrever.

OS PROJETEIS SAO OS SIMBOLOS DO ALFABETO. Num chefe de linguagens formais, a
rajada E uma palavra, e cada tiro e uma letra dela - entao "a" e "b" nao sao
enfeite de placeholder, sao o desenho certo. A folha tem oito quadros de 32x32
em 4x2, no mesmo arranjo dos projeteis do Salles.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO = (74, 94, 128, 255)
BORDA  = (150, 180, 220, 255)
TINTA  = (228, 238, 250, 255)
ESTADO = (122, 170, 220, 255)
ACEITA = (122, 210, 180, 255)
TRACO  = (40, 52, 70, 255)

# Uma cor por simbolo: e assim que o jogador le "(ab)*" na tela sem precisar
# distinguir as letras no meio do tiroteio. A letra confirma de perto, a cor
# conta de longe.
COR_A = (236, 196, 110, 255)   # ambar
COR_B = (150, 170, 240, 255)   # azul


def quadro_json(nome, w, h, quadros=1, lado=None):
    """O mesmo formato que o leitor de sprites do projeto espera.

    Com "quadros" > 1 a folha e lida como uma grade de 4 colunas, igual a dos
    projeteis do Salles - 4 em cima, o resto embaixo.
    """
    lado = lado or w
    frames = []
    for i in range(quadros):
        x, y = (i % 4) * lado, (i // 4) * lado
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
        "meta": {"app": "gerar_vladimir.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def vladimir():
    """O cartao do professor: um automato de dois estados."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A seta de entrada, que marca o estado inicial.
    d.line([14, 52, 28, 52], fill=TINTA, width=2)
    d.polygon([(28, 48), (34, 52), (28, 56)], fill=TINTA)

    # q0: o estado inicial.
    d.ellipse([34, 34, 70, 70], fill=ESTADO, outline=TRACO, width=2)

    # A transicao q0 -> q1.
    d.line([70, 52, 84, 52], fill=TINTA, width=2)
    d.polygon([(84, 48), (90, 52), (84, 56)], fill=TINTA)

    # q1: o estado de ACEITACAO, e por isso o circulo duplo.
    d.ellipse([90, 34, 122, 66], fill=ACEITA, outline=TRACO, width=2)
    d.ellipse([95, 39, 117, 61], outline=TRACO, width=2)

    d.text((34, 92), "VLADIMIR", fill=TINTA)
    return im


def simbolo(letra, cor):
    """Um simbolo do alfabeto: 32x32, a letra num disco da cor dela."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([2, 2, 29, 29], fill=cor, outline=TRACO, width=2)
    # O brilho de cima da ao disco um volume que o circulo chapado nao tem.
    d.arc([6, 6, 25, 25], start=200, end=330, fill=(255, 255, 255, 120), width=2)
    d.text((13, 11), letra, fill=TRACO)
    return im


def alfabeto():
    """A folha dos simbolos: 4 quadros de "a" e 4 de "b", em 4x2.

    QUATRO DE CADA, e nao um, porque o leitor de animacao do projeto recebe uma
    LISTA de quadros por animacao ("animacoes": {"A": [0,1,2,3]}). Com um quadro
    so a animacao existe e nao anima; com quatro da para pulsar depois sem mexer
    em arquivo nenhum.
    """
    folha = Image.new("RGBA", (128, 64), (0, 0, 0, 0))
    for i in range(4):
        folha.alpha_composite(simbolo("a", COR_A), (i * 32, 0))
    for i in range(4):
        folha.alpha_composite(simbolo("b", COR_B), (i * 32, 32))
    return folha


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    grava(vladimir(), "DPIBHVladimir", "Assets/Teachers")
    grava(alfabeto(), "DPIBHVladimirSimbolo", "Assets/Teachers/Projectiles",
          quadros=8, lado=32)
