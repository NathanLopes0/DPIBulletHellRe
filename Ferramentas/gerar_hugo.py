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
    Materias    um ladrilho de cada professor, COPIADO da folha de projeteis
                dele (MATERIAS, abaixo): no TCC o Hugo atira a materia do
                orientador e as da banca, e cada uma tem de ser reconhecida
                pelo projetil que o aluno ja enfrentou.
"""
from PIL import Image, ImageDraw, ImageFont
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


# De onde vem o ladrilho de cada professor: a folha e o quadro dela. Na ordem
# de Golpe::Materia (sem o Generico), que e a ordem dos quadros 3 a 12.
MATERIAS = [
    ("Andre",      "DPIBHAndreBaloes",         0),   # o balao do BOCA
    ("Salles",     "DPIBHSallesPilha",         0),   # um bloco da pilha
    ("Ricardo",    "DPIBHRicardoLogica",       1),   # o bit 1
    ("Thiago",     "DPIBHThiagoDados",         0),   # uma casa da tabela
    ("Luizinho",   "DPIBHLuizinhoMemoria",     1),   # a celula viva do heap
    ("Vladimir",   "DPIBHVladimirSimbolo",     0),   # o simbolo a
    ("Julio",      "DPIBHJulioToken",          8),   # o agente
    ("MariaLucia", "DPIBHMariaLuciaRequisito", 12),  # o bug
    ("Lucas",      "DPIBHLucasCodigo",         8),   # a promessa
    ("Vitor",      "DPIBHVitorTarefa",         20),  # o pacote
]


def ladrilho(folha, quadro):
    """O quadro 'quadro' da folha de outro professor, cabendo em 32 px."""
    pasta = os.path.join(RAIZ, "Assets", "Teachers", "Projectiles")
    im = Image.open(os.path.join(pasta, folha + ".png")).convert("RGBA")
    with open(os.path.join(pasta, folha + ".json"), encoding="utf-8") as f:
        r = json.load(f)["frames"][quadro]["frame"]
    recorte = im.crop((r["x"], r["y"], r["x"] + r["w"], r["y"] + r["h"]))
    recorte = recorte.crop(recorte.getbbox() or (0, 0, r["w"], r["h"]))
    escala = 30.0 / max(recorte.width, recorte.height)
    recorte = recorte.resize((max(1, round(recorte.width * escala)), max(1, round(recorte.height * escala))),
                             Image.NEAREST)
    saida = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    saida.alpha_composite(recorte, ((32 - recorte.width) // 2, (32 - recorte.height) // 2))
    return saida


# ---- OS ALVOS DOS OBJETIVOS (Objetivo.h): o que o aluno derruba para abrir o
# escudo do Hugo. Cores proprias, para nao se confundir com nenhuma materia.
VERDE = (90, 200, 120, 255)
VERMELHO = (220, 70, 70, 255)
AZUL_BALAO = (110, 170, 240, 255)


def fonte(tamanho):
    caminho = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
    try:
        return ImageFont.truetype(caminho, tamanho)
    except OSError:
        return ImageFont.load_default()


def assinatura():
    """A ASSINATURA: a folha da proposta com o rabisco e a linha de assinar."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([5, 4, 26, 28], fill=PAPEL, outline=PRETO)
    d.line([8, 22, 23, 22], fill=PRETO, width=1)
    d.line([(9, 19), (12, 13), (14, 19), (17, 12), (19, 18), (22, 14)], fill=CAPA, width=2)
    d.ellipse([20, 23, 26, 29], fill=VERDE, outline=PRETO)
    return im


def resposta(letra):
    """A RESPOSTA: o balao azul com a letra - A, B ou C - do placar."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([2, 2, 29, 26], fill=AZUL_BALAO, outline=PRETO, width=2)
    d.polygon([(9, 23), (7, 30), (15, 25)], fill=AZUL_BALAO, outline=PRETO)
    f = fonte(20)
    caixa = d.textbbox((0, 0), letra, font=f)
    w, h = caixa[2] - caixa[0], caixa[3] - caixa[1]
    d.text((16 - w / 2 - caixa[0], 14 - h / 2 - caixa[1]), letra, font=f, fill=TINTA)
    return im


def correcao():
    """A CORRECAO: o X vermelho da banca numa folha."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([5, 4, 26, 28], fill=PAPEL, outline=PRETO)
    d.line([9, 8, 22, 24], fill=VERMELHO, width=3)
    d.line([22, 8, 9, 24], fill=VERMELHO, width=3)
    return im


def bug():
    """O BUG dos experimentos: um besouro vermelho, de patas abertas."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for y in (11, 16, 21):
        d.line([4, y - 2, 27, y + 2], fill=PRETO, width=2)
    d.ellipse([9, 7, 22, 27], fill=VERMELHO, outline=PRETO, width=2)
    d.ellipse([12, 3, 19, 10], fill=PRETO)
    d.line([15, 9, 15, 26], fill=PRETO, width=1)
    return im


def folha():
    """Quatro quadros por linha, de 32 px. As linhas novas vao sempre no FIM.

        0 Pagina   1 Monografia   2 Pergunta   3 a 12 as materias (MATERIAS)
        13 Assinatura   14 A   15 B   16 C   17 Correcao   18 Bug
    """
    desenhos = ([pagina(), monografia(), pergunta()] + [ladrilho(f, q) for _, f, q in MATERIAS] +
                [assinatura(), resposta("A"), resposta("B"), resposta("C"), correcao(), bug()])
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
