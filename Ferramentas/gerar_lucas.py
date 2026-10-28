#!/usr/bin/env python3
"""Arte PROVISORIA do Lucas (INF 321 - Programacao Web) e dos projeteis dele.

Mesmo estilo de cartao do Julio, do Thiago, do Vladimir, do Vitor e da Maria
Lucia: e deliberadamente um placeholder, para nao se passar por arte final.
Trocar o PNG basta - o json de quadro continua valendo.

O MOTIVO DO CARTAO E UMA JANELA DE NAVEGADOR: a barra de titulo com os tres
pontos e, dentro dela, um "</>" em cima e um "|>" embaixo. E a materia na ordem
em que ele a da - o front classico primeiro, o Elixir depois - e diz a materia
sem escrever o nome dela.

SEM LOGOTIPO NENHUM. HTML5, JavaScript e Elixir tem marcas proprias; o cartao
usa so os SIMBOLOS da sintaxe, que nao sao de ninguem.

OS PROJETEIS, um por assunto, e cada um com a cor de uma camada da web:

    Tag       laranja, "<>": a estrutura (HTML).
    Regra     azul, "{}": o estilo (CSS).
    Promessa  amarelo, "...": o que ainda nao chegou (JavaScript).
    Processo  roxo, um circulo com "|>": o que roda no servidor (Elixir).
    Mensagem  lilas, um envelope: o send de um processo (Elixir).
    Cursor    a seta do mouse, que clica no <button> da pagina (HTML).
    Pipe      lilas, um ladrilho com "|>": cada etapa do pipe que escorre
              embaixo do supervisor parado (Elixir).
    Ficha     a linha do script no event loop da final: um quadro OCO, com a
              cor do tipo da linha na borda - oco para nao se confundir com os
              projeteis cheios, que machucam; a ficha nao machuca.

O Processo e um TIPO separado em projeteis.json, e nao so uma animacao, porque
ele vai ganhar regras de tipo (ser derrubavel) que os outros nao tem.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO = (58, 66, 92, 255)
BORDA  = (140, 156, 206, 255)
TINTA  = (232, 236, 250, 255)
TRACO  = (30, 34, 48, 255)
BARRA  = (92, 102, 140, 255)
TELA   = (24, 28, 42, 255)

COR_TAG      = (232, 120, 64, 255)    # laranja: estrutura
COR_REGRA    = (86, 150, 232, 255)    # azul: estilo
COR_PROMESSA = (238, 206, 86, 255)    # amarelo: o que ainda nao chegou
COR_PROCESSO = (150, 104, 210, 255)   # roxo: o servidor
COR_MENSAGEM = (206, 176, 246, 255)   # lilas: a mensagem entre processos


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
    return {
        "frames": frames,
        "meta": {"app": "gerar_lucas.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def lucas():
    """O cartao do professor: uma janela de navegador."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A JANELA: barra de titulo com os tres pontos, e a tela embaixo.
    d.rectangle([16, 14, 111, 86], fill=TELA, outline=TINTA, width=2)
    d.rectangle([17, 15, 110, 26], fill=BARRA)
    for i, cor in enumerate((COR_TAG, COR_PROMESSA, COR_REGRA)):
        d.ellipse([22 + i * 9, 18, 28 + i * 9, 24], fill=cor)

    # "</>" em cima, desenhado com linhas: o front.
    d.line([(40, 36), (32, 44), (40, 52)], fill=COR_TAG, width=3)
    d.line([(58, 34), (50, 54)], fill=COR_TAG, width=3)
    d.line([(68, 36), (76, 44), (68, 52)], fill=COR_TAG, width=3)

    # "|>" embaixo: o pipe do Elixir, o back.
    d.line([(36, 62), (36, 78)], fill=COR_PROCESSO, width=3)
    d.line([(44, 62), (54, 70), (44, 78)], fill=COR_PROCESSO, width=3)
    d.line([(62, 75), (92, 75)], fill=COR_PROCESSO, width=2)

    d.text((46, 96), "LUCAS", fill=TINTA)
    return im


def tag(fase):
    """A TAG: um ladrilho laranja com "<>". Pulsa de leve entre os quadros."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = 4 + (fase % 2)
    d.rounded_rectangle([m, m, 31 - m, 31 - m], radius=5, fill=COR_TAG, outline=TRACO, width=2)
    d.line([(13, 10), (8, 16), (13, 22)], fill=TINTA, width=2)
    d.line([(19, 10), (24, 16), (19, 22)], fill=TINTA, width=2)
    return im


def regra(fase):
    """A REGRA: um ladrilho azul com "{}"."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = 4 + (fase % 2)
    d.rounded_rectangle([m, m, 31 - m, 31 - m], radius=5, fill=COR_REGRA, outline=TRACO, width=2)
    for x, s in ((11, 1), (21, -1)):
        d.line([(x, 9), (x - 2 * s, 11), (x - 2 * s, 14), (x - 4 * s, 16),
                (x - 2 * s, 18), (x - 2 * s, 21), (x, 23)], fill=TINTA, width=2)
    return im


def promessa(fase):
    """A PROMESSA: um circulo amarelo com tres pontos que acendem em sequencia."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([4, 4, 27, 27], fill=COR_PROMESSA, outline=TRACO, width=2)
    for i in range(3):
        cor = TRACO if i == fase % 3 else (150, 126, 40, 255)
        d.ellipse([9 + i * 6, 14, 12 + i * 6, 17], fill=cor)
    return im


def processo(fase):
    """O PROCESSO: um circulo roxo com "|>". O anel gira entre os quadros."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([3, 3, 28, 28], fill=COR_PROCESSO, outline=TRACO, width=2)
    d.arc([5, 5, 26, 26], start=fase * 90, end=fase * 90 + 120, fill=TINTA, width=2)
    d.line([(11, 11), (11, 21)], fill=TINTA, width=2)
    d.line([(15, 11), (21, 16), (15, 21)], fill=TINTA, width=2)
    return im


def mensagem(fase):
    """A MENSAGEM: um envelope lilas, a cor do processo mais clara - o send
    de um processo do Elixir. A aba pisca entre os quadros."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([5, 9, 26, 23], radius=3, fill=COR_MENSAGEM, outline=TRACO, width=2)
    aba = 16 + (fase % 2)
    d.line([(6, 10), (16, aba), (25, 10)], fill=TINTA, width=2)
    return im


def cursor(fase):
    """O CURSOR do mouse, que voa ate o <button> da pagina e clica: a seta
    branca de sempre, com o contorno escuro. No clique (o ultimo quadro) ela
    afunda um pixel."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    a = 1 if fase == 3 else 0
    seta = [(9, 5 + a), (9, 25 + a), (14, 20 + a), (18, 28 + a), (21, 26 + a), (17, 19 + a), (24, 19 + a)]
    d.polygon(seta, fill=TINTA, outline=TRACO)
    d.line(seta + [seta[0]], fill=TRACO, width=2)
    return im


def pipe(fase):
    """O PIPE: um ladrilho lilas, cheio como os outros tiros, com o "|>" do
    Elixir. O envelope fica para a mensagem; isto e o dado escorrendo pelo
    pipe."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = 4 + (fase % 2)
    d.rounded_rectangle([m, m, 31 - m, 31 - m], radius=5, fill=COR_PROCESSO, outline=TRACO, width=2)
    d.line([(11, 10), (11, 22)], fill=TINTA, width=2)
    d.line([(15, 10), (22, 16), (15, 22)], fill=TINTA, width=2)
    return im


def ficha(tipo):
    """A FICHA de uma linha do script no event loop: um quadro oco, como um
    frame da pilha, com duas linhas de codigo dentro. Um quadro por tipo de
    linha: console.log (laranja), setTimeout (azul), .then (amarelo) e fetch
    (lilas)."""
    cor = (COR_TAG, COR_REGRA, COR_PROMESSA, COR_MENSAGEM)[tipo]
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([3, 7, 28, 24], radius=3, fill=TELA, outline=cor, width=2)
    d.line([(8, 13), (20, 13)], fill=cor, width=2)
    d.line([(8, 18), (16, 18)], fill=TINTA, width=2)
    return im


def folha():
    """Quatro quadros de cada, um assunto por linha, numa folha 4x8."""
    f = Image.new("RGBA", (128, 256), (0, 0, 0, 0))
    for linha, desenho in enumerate((tag, regra, promessa, processo, mensagem, cursor, ficha, pipe)):
        for i in range(4):
            f.alpha_composite(desenho(i), (i * 32, linha * 32))
    return f


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    grava(lucas(), "DPIBHLucas", "Assets/Teachers")
    grava(folha(), "DPIBHLucasCodigo", "Assets/Teachers/Projectiles", quadros=32, lado=32)
