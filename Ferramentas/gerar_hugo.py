#!/usr/bin/env python3
"""Arte PROVISORIA do Hugo (TCC - Trabalho de Conclusao de Curso).

O Hugo nao tem arte no repositorio: este script desenha o cartao dele e a folha
de projeteis, no estilo de placeholder dos outros gerar_*.py. Trocar os PNG
basta - os json de quadro continuam valendo.

    Hugo        o cartao do professor: o capelo da formatura em cima de uma
                monografia encadernada.
    Pagina      uma folha A4 com as linhas do texto - o que sai da monografia.
    Monografia  o volume encadernado, de capa azul e lombada com "TCC".
    Pergunta    o balao da pergunta de banca, com o ponto de interrogacao.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO = (70, 58, 92, 255)
BORDA  = (196, 170, 230, 255)
TINTA  = (240, 234, 250, 255)
PRETO  = (28, 24, 36, 255)
PAPEL  = (244, 242, 232, 255)
LINHA  = (150, 150, 170, 255)
CAPA   = (46, 84, 160, 255)
CAPA_CLARA = (90, 130, 210, 255)
DOURADO = (236, 196, 80, 255)
BALAO  = (250, 226, 120, 255)
BALAO_BORDA = (170, 120, 30, 255)


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
            "duration": 140,
        })
    return {
        "frames": frames,
        "meta": {"app": "gerar_hugo.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def hugo():
    """O cartao do professor: o capelo em cima da monografia."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A MONOGRAFIA: o volume deitado, capa azul e lombada dourada.
    d.rectangle([28, 58, 100, 84], fill=CAPA, outline=PRETO)
    d.rectangle([28, 58, 36, 84], fill=CAPA_CLARA, outline=PRETO)
    d.rectangle([36, 82, 100, 86], fill=PAPEL, outline=PRETO)
    d.text((54, 64), "TCC", fill=DOURADO)

    # O CAPELO: o quadrado do topo, a copa e a borla.
    d.polygon([(64, 22), (100, 34), (64, 46), (28, 34)], fill=PRETO)
    d.rectangle([48, 38, 80, 52], fill=PRETO)
    d.line([96, 35, 98, 52], fill=DOURADO, width=2)
    d.ellipse([95, 50, 101, 56], fill=DOURADO)

    d.text((48, 98), "HUGO", fill=TINTA)
    return im


def pagina():
    """A PAGINA: a folha com as linhas do texto e a dobra no canto."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(8, 3), (20, 3), (25, 8), (25, 29), (8, 29)], fill=PAPEL, outline=PRETO)
    d.polygon([(20, 3), (20, 8), (25, 8)], fill=LINHA, outline=PRETO)
    for y in range(11, 27, 3):
        d.line([11, y, 22, y], fill=LINHA, width=1)
    return im


def monografia():
    """A MONOGRAFIA: o volume em pe, de frente, com o titulo dourado."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([6, 3, 26, 29], fill=CAPA, outline=PRETO)
    d.rectangle([6, 3, 9, 29], fill=CAPA_CLARA, outline=PRETO)
    d.rectangle([12, 8, 23, 12], fill=DOURADO)
    d.line([12, 16, 23, 16], fill=CAPA_CLARA, width=1)
    d.line([12, 19, 21, 19], fill=CAPA_CLARA, width=1)
    return im


def pergunta():
    """A PERGUNTA DE BANCA: o balao amarelo com a interrogacao."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([3, 3, 28, 25], fill=BALAO, outline=BALAO_BORDA, width=2)
    d.polygon([(10, 22), (8, 30), (16, 24)], fill=BALAO, outline=BALAO_BORDA)
    # a interrogacao, em pixels grossos
    d.arc([11, 7, 21, 17], 200, 360, fill=PRETO, width=3)
    d.arc([11, 7, 21, 17], 0, 70, fill=PRETO, width=3)
    d.line([16, 16, 16, 18], fill=PRETO, width=3)
    d.rectangle([15, 20, 17, 22], fill=PRETO)
    return im


def folha():
    """Quatro quadros por linha, de 32 px. As linhas novas vao sempre no FIM.

        0 Pagina   1 Monografia   2 Pergunta
    """
    desenhos = [pagina(), monografia(), pergunta()]
    f = Image.new("RGBA", (128, 32 * ((len(desenhos) + 3) // 4)), (0, 0, 0, 0))
    for i, im in enumerate(desenhos):
        f.alpha_composite(im, ((i % 4) * 32, (i // 4) * 32))
    return f, len(desenhos)


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    grava(hugo(), "DPIBHHugo", "Assets/Teachers")
    f, n = folha()
    grava(f, "DPIBHHugoTcc", "Assets/Teachers/Projectiles", quadros=n, lado=32)
