#!/usr/bin/env python3
"""A DUVIDA, o tiro do aluno: um balao de fala com um "?".

O aluno nao ataca o professor - ele PERGUNTA. O tiro antigo era um losango
cinza com um anel vermelho girando, e lia como arma de arremesso; o balao le
como fala, e nenhum ataque de professor usa esse formato (os post-its da Maria
Lucia sao amarelos e quadrados).

UMA FOLHA POR JOGADOR: o balao e o mesmo, branco; o "?" e o contorno tem a
cor do aluno (azul o 1, laranja o 2 - as cores da escolha do orientador e de
Hud::CorDoAluno). Em dupla da para ver de quem e cada pergunta.

Oito quadros de 24x24 numa folha de 72x72, tres por linha - o formato que o
PlayerProjectile1.json ja descrevia. O balao quica de leve.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PASTA = os.path.join(RAIZ, "Assets", "Player")

PAPEL = (250, 250, 240, 255)

# O "?" em pixels, 9 de largura por 11 de altura.
INTERROGACAO = [
    "..#####..",
    ".##...##.",
    ".##...##.",
    "......##.",
    ".....##..",
    "....##...",
    "...##....",
    "...##....",
    ".........",
    "...##....",
    "...##....",
]

# (cor do "?", cor do contorno) de cada jogador.
JOGADORES = {
    1: ((40, 110, 220, 255), (28, 48, 104, 255)),
    2: ((226, 112, 24, 255), (104, 52, 16, 255)),
}

QUIQUE = [0, 0, -1, -1, -1, 0, 0, 0]


def quadro(i, cor, contorno):
    f = Image.new("RGBA", (24, 24), (0, 0, 0, 0))
    d = ImageDraw.Draw(f)
    dy = QUIQUE[i]
    d.rounded_rectangle((2, 1 + dy, 21, 17 + dy), radius=5, fill=PAPEL, outline=contorno)
    # O rabinho do balao, para baixo e para a esquerda: a fala sai do aluno.
    d.polygon([(6, 16 + dy), (11, 16 + dy), (5, 21 + dy)], fill=PAPEL)
    d.line([(6, 17 + dy), (5, 21 + dy)], fill=contorno)
    d.line([(5, 21 + dy), (11, 17 + dy)], fill=contorno)
    for y, linha in enumerate(INTERROGACAO):
        for x, c in enumerate(linha):
            if c == "#":
                d.point((7 + x, 4 + dy + y), cor)
    return f


def folha(cor, contorno):
    im = Image.new("RGBA", (72, 72), (0, 0, 0, 0))
    for i in range(8):
        q = quadro(i, cor, contorno)
        im.paste(q, ((i % 3) * 24, (i // 3) * 24), q)
    return im


def descricao(imagem):
    frames = {}
    for i in range(8):
        frames[f"Projectile{i}.png"] = {
            "frame": {"x": (i % 3) * 24, "y": (i // 3) * 24, "w": 24, "h": 24},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": 24, "h": 24},
            "sourceSize": {"w": 24, "h": 24},
        }
    return {"frames": frames,
            "meta": {"app": "gerar_duvida.py", "version": "1.0", "image": imagem,
                     "format": "RGBA8888", "size": {"w": 72, "h": 72}}}


if __name__ == "__main__":
    for n, (cor, contorno) in JOGADORES.items():
        nome = f"PlayerProjectile{n}"
        folha(cor, contorno).save(os.path.join(PASTA, nome + ".png"))
        with open(os.path.join(PASTA, nome + ".json"), "w") as saida:
            json.dump(descricao(nome + ".png"), saida)
        print("gravado", nome)
