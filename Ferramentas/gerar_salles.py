#!/usr/bin/env python3
"""Arte PROVISORIA do Salles (INF 213 - Estruturas de Dados).

Desenhada por programa, como a dos outros chefes refeitos: serve para jogar e
balancear, e sai quando houver arte de verdade. Trocar o .png basta - o json de
quadro continua valendo.

A CAPIVARA QUE ATRAVESSA O CAMPO. O Salles poe capivara em todo exercicio da
disciplina, e de vez em quando ela aparece no meio da fase: atravessa o campo
de lado a lado, andando. Vista de lado, com quatro passos, e cada passo virado
para os dois lados (ela anda para a direita ou para a esquerda).
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

PELO = (156, 108, 66, 255)
PELO_CLARO = (186, 138, 92, 255)
PELO_ESCURO = (112, 76, 46, 255)
CONTORNO = (64, 42, 26, 255)
FOCINHO = (92, 62, 40, 255)


def quadro_json(nome, w, h, largo, alto, quadros):
    """O formato que o leitor de sprites do projeto espera, em 4 colunas."""
    frames = []
    for i in range(quadros):
        x, y = (i % 4) * largo, (i // 4) * alto
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": largo, "h": alto},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": largo, "h": alto},
            "sourceSize": {"w": largo, "h": alto},
            "duration": 120,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_salles.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def capivara(passo):
    """Uma capivara andando para a direita, no passo 0 a 3."""
    s = 4   # desenhada grande e reduzida: o contorno sai liso
    W, H = 96, 64
    im = Image.new("RGBA", (W * s, H * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    def r(x0, y0, x1, y1, raio, cor, borda=True):
        d.rounded_rectangle([x0 * s, y0 * s, x1 * s, y1 * s], radius=raio * s, fill=cor,
                            outline=CONTORNO if borda else None, width=2 * s if borda else 0)

    # AS PERNAS, que andam: as de tras e as da frente trocam de lugar a cada
    # passo, como num trote.
    avanco = [0, 3, 0, -3][passo]
    for x, lado in ((24, 1), (34, -1), (62, 1), (72, -1)):
        dx = avanco * lado
        r(x + dx - 4, 40, x + dx + 4, 58, 3, PELO_ESCURO)

    # O CORPO: um barril, e a barriga mais clara.
    r(10, 16, 82, 50, 16, PELO)
    r(20, 36, 72, 48, 8, PELO_CLARO, borda=False)

    # A CABECA, grande e quadrada, com o focinho rombudo.
    r(62, 8, 92, 38, 10, PELO)
    r(80, 20, 94, 36, 6, FOCINHO)
    d.ellipse([84 * s, 24 * s, 87 * s, 27 * s], fill=CONTORNO)          # a narina
    d.ellipse([74 * s, 15 * s, 79 * s, 20 * s], fill=CONTORNO)          # o olho
    d.ellipse([75 * s, 15 * s, 77 * s, 17 * s], fill=(255, 255, 255, 200))
    r(64, 4, 72, 12, 3, PELO_ESCURO)                                     # a orelha

    return im.resize((W, H), Image.LANCZOS)


def capivarona():
    """A folha: os quatro passos para a direita em cima, e para a esquerda embaixo."""
    folha = Image.new("RGBA", (96 * 4, 64 * 2), (0, 0, 0, 0))
    for p in range(4):
        direita = capivara(p)
        folha.alpha_composite(direita, (p * 96, 0))
        folha.alpha_composite(direita.transpose(Image.FLIP_LEFT_RIGHT), (p * 96, 64))
    return folha


# OS BLOCOS DA PILHA DA FASE 1: um no numerado, de 0 a 9, com o elo em cima -
# o ponteiro para o bloco de baixo, o que entrou antes. Verdes, para nao se
# confundir com a lista ambar e a arvore das outras fases.
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
BLOCO = (110, 186, 150, 255)
BLOCO_CLARO = (160, 220, 190, 255)
TRACO = (30, 52, 44, 255)


def bloco(valor):
    s = 4
    L = 40
    im = Image.new("RGBA", (L * s, L * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([18 * s, 0, 22 * s, 6 * s], fill=TRACO)                    # o elo
    d.rounded_rectangle([3 * s, 5 * s, 37 * s, 37 * s], radius=6 * s, fill=BLOCO, outline=TRACO, width=2 * s)
    d.rounded_rectangle([7 * s, 9 * s, 33 * s, 15 * s], radius=3 * s, fill=BLOCO_CLARO)
    d.text((20 * s, 23 * s), str(valor), fill=TRACO, font=ImageFont.truetype(CARLITO, 22 * s), anchor="mm")
    return im.resize((L, L), Image.LANCZOS)


def lasca():
    """O pedaco de um bloco que estourou: o valor do topo vira lascas."""
    s = 4
    L = 40
    im = Image.new("RGBA", (L * s, L * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([12 * s, 12 * s, 28 * s, 28 * s], radius=3 * s, fill=BLOCO_CLARO, outline=TRACO, width=2 * s)
    return im.resize((L, L), Image.LANCZOS)


def pilha():
    """Os dez blocos numerados, e a lasca no quadro 10."""
    folha = Image.new("RGBA", (40 * 4, 40 * 3), (0, 0, 0, 0))
    for v in range(10):
        folha.alpha_composite(bloco(v), ((v % 4) * 40, (v // 4) * 40))
    folha.alpha_composite(lasca(), ((10 % 4) * 40, (10 // 4) * 40))
    return folha


# A TABELA HASH DA FASE 2: as chaves (0 a 99), os baldes (0 a 16), o tiro do
# percurso e o NULL do fim da lista. As chaves sao ambar, como a lista
# encadeada que elas formam; os baldes sao a fileira escura da tabela.
CHAVE = (236, 190, 100, 255)
CHAVE_CLARA = (250, 220, 150, 255)
TRACO_CHAVE = (70, 46, 20, 255)
BALDE = (52, 70, 110, 255)
BALDE_BORDA = (140, 170, 220, 255)
NULO = (210, 90, 90, 255)


def no_quadro(desenho):
    s = 4
    L = 40
    im = Image.new("RGBA", (L * s, L * s), (0, 0, 0, 0))
    desenho(ImageDraw.Draw(im), s)
    return im.resize((L, L), Image.LANCZOS)


def chave(k):
    def desenho(d, s):
        d.rectangle([18 * s, 32 * s, 22 * s, 40 * s], fill=TRACO_CHAVE)    # o next, para baixo
        d.rounded_rectangle([3 * s, 3 * s, 37 * s, 33 * s], radius=6 * s, fill=CHAVE, outline=TRACO_CHAVE, width=2 * s)
        d.rounded_rectangle([7 * s, 6 * s, 33 * s, 11 * s], radius=2 * s, fill=CHAVE_CLARA)
        d.text((20 * s, 19 * s), str(k), fill=TRACO_CHAVE, font=ImageFont.truetype(CARLITO, 18 * s), anchor="mm")
    return no_quadro(desenho)


def balde(i):
    def desenho(d, s):
        d.rectangle([2 * s, 6 * s, 38 * s, 34 * s], fill=BALDE, outline=BALDE_BORDA, width=2 * s)
        d.text((20 * s, 20 * s), str(i), fill=(230, 240, 255, 255), font=ImageFont.truetype(CARLITO, 18 * s), anchor="mm")
    return no_quadro(desenho)


def percurso():
    def desenho(d, s):
        d.ellipse([13 * s, 13 * s, 27 * s, 27 * s], fill=CHAVE_CLARA, outline=TRACO_CHAVE, width=2 * s)
    return no_quadro(desenho)


def nulo():
    def desenho(d, s):
        d.rounded_rectangle([2 * s, 8 * s, 38 * s, 32 * s], radius=5 * s, fill=NULO, outline=(80, 20, 20, 255), width=2 * s)
        d.text((20 * s, 20 * s), "NULL", fill=(255, 235, 235, 255), font=ImageFont.truetype(CARLITO, 12 * s), anchor="mm")
    return no_quadro(desenho)


def elo():
    """Um ponto da aresta da arvore da fase 3: apagado, porque e o desenho da
    arvore, e nao um tiro."""
    def desenho(d, s):
        d.ellipse([15 * s, 15 * s, 25 * s, 25 * s], fill=(200, 200, 215, 150))
    return no_quadro(desenho)


def hash_():
    """As 100 chaves, os 17 baldes, o percurso (117), o NULL (118) e o elo da
    arvore (119)."""
    quadros = [chave(k) for k in range(100)] + [balde(i) for i in range(17)] + [percurso(), nulo(), elo()]
    linhas = (len(quadros) + 3) // 4
    folha = Image.new("RGBA", (40 * 4, 40 * linhas), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        folha.alpha_composite(q, ((i % 4) * 40, (i // 4) * 40))
    return folha, len(quadros)


def grava(im, nome, pasta, largo, alto, quadros):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, largo, alto, quadros), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    grava(capivarona(), "DPIBHSallesCapivarona", "Assets/Teachers/Projectiles", 96, 64, 8)
    grava(pilha(), "DPIBHSallesPilha", "Assets/Teachers/Projectiles", 40, 40, 11)
    folha, n = hash_()
    grava(folha, "DPIBHSallesHash", "Assets/Teachers/Projectiles", 40, 40, n)
