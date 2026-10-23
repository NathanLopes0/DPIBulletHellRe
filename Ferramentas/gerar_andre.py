#!/usr/bin/env python3
"""Arte do Andre (INF 110 - Programacao 1) que nao veio do Aseprite.

O balao de DPIBHAndreBaloon.png e arte de verdade, e fica. O que sai daqui e o
que as fases novas precisam e ele nao tem, no MESMO estilo de pixel dele (as
mesmas cores, o mesmo brilho, o mesmo barbante):

  DPIBHAndreBaloes   o balao do BOCA com um numero pintado - o dado da fase 1
                     (Condicional.h), em quatro cores
  DPIBHAndreConfete  o confete do balao que estoura, girando
  DPIBHAndreRotulos  o cabecalho de cada ramo do fluxograma da fase 1, como
                     uma linha de codigo num editor - apagado e aceso
  DPIBHAndreIndices  o indice de cada casa do vetor no chao da fase 2
                     (Laco.h): apagado, aceso (o i esta nela), pulado (o
                     continue) e vermelho (o segfault)
  DPIBHAndreBytes    a memoria despejada do segfault: um byte em hexa

Os nomes das animacoes estao em Assets/Attacks/andre/projeteis.json, e um teste
confere que toda cara que o Andre pede existe la.
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")

# As cores do balao original (DPIBHAndreBaloon.png): claro, meio, escuro. O
# verde nao existia la, e segue a mesma receita.
CORES = {
    "Vermelho": ((246, 86, 71), (214, 47, 47), (200, 38, 52)),
    "Azul":     ((66, 122, 228), (50, 72, 205), (47, 34, 182)),
    "Amarelo":  ((255, 247, 85), (223, 215, 36), (193, 200, 23)),
    "Verde":    ((96, 222, 112), (44, 176, 72), (30, 140, 60)),
}
ORDEM_DAS_CORES = ["Vermelho", "Azul", "Amarelo", "Verde"]   # a de Condicional.h
BRANCO = (255, 255, 255, 255)
CINZA = (205, 205, 205, 255)

# Os algarismos em 5 x 7, de pixel.
DIGITOS = {
    "0": ["01110", "10001", "10011", "10101", "11001", "10001", "01110"],
    "1": ["00100", "01100", "00100", "00100", "00100", "00100", "01110"],
    "2": ["01110", "10001", "00001", "00010", "00100", "01000", "11111"],
    "3": ["11110", "00001", "00001", "01110", "00001", "00001", "11110"],
    "4": ["00010", "00110", "01010", "10010", "11111", "00010", "00010"],
    "5": ["11111", "10000", "11110", "00001", "00001", "10001", "01110"],
    "6": ["00110", "01000", "10000", "11110", "10001", "10001", "01110"],
    "7": ["11111", "00001", "00010", "00100", "01000", "01000", "01000"],
    "8": ["01110", "10001", "10001", "01110", "10001", "10001", "01110"],
    "9": ["01110", "10001", "10001", "01111", "00001", "00010", "01100"],
}

BW, BH = 24, 34   # o quadro do balao numerado


def balao(cor, texto):
    """Um balao de 20 x 24 com o brilho do original, o no, o barbante e o
    numero no meio, branco com contorno escuro."""
    claro, meio, escuro = (c + (255,) for c in CORES[cor])
    im = Image.new("RGBA", (BW, BH), (0, 0, 0, 0))
    px = im.load()
    cx, cy, rx, ry = 11.5, 12.0, 10.0, 12.0
    for y in range(BH):
        for x in range(BW):
            d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
            if d <= 1.0:
                px[x, y] = meio
                if d > 0.78:
                    px[x, y] = escuro if (x > cx or y > cy + 4) else claro
    # o contorno claro, como o do original
    for y in range(BH):
        for x in range(BW):
            if px[x, y][3] == 0:
                viz = [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
                if any(0 <= a < BW and 0 <= b < BH and px[a, b][3] and px[a, b] not in (BRANCO, CINZA) for a, b in viz):
                    px[x, y] = BRANCO if (x + y) % 3 else CINZA
    # o brilho: a diagonal clara do alto a esquerda
    for k in range(6):
        x, y = 5 + k // 2, 9 - k
        if px[x, y][3]:
            px[x, y] = claro
    # o no e o barbante
    px[11, 25] = escuro
    px[12, 25] = escuro
    for y, x in zip(range(26, BH), [12, 12, 13, 13, 12, 11, 11, 12]):
        px[x, y] = BRANCO if y % 3 else CINZA
    # o numero, com contorno escuro para ler sobre qualquer cor
    largura = 6 * len(texto) - 1
    x0, y0 = int(round(cx - largura / 2 + 0.5)), 8
    contorno = (30, 24, 40, 255)
    pontos = []
    for i, ch in enumerate(texto):
        for j, linha in enumerate(DIGITOS[ch]):
            for k, b in enumerate(linha):
                if b == "1":
                    pontos.append((x0 + 6 * i + k, y0 + j))
    for (x, y) in pontos:
        for a in (-1, 0, 1):
            for b in (-1, 0, 1):
                if 0 <= x + a < BW and 0 <= y + b < BH and (x + a, y + b) not in pontos:
                    px[x + a, y + b] = contorno
    for (x, y) in pontos:
        px[x, y] = BRANCO
    return im


def baloes():
    """Os quadros na ordem de Condicional::CaraDoBalao: cor por cor, e em cada
    cor os numeros de 0 a 9."""
    quadros = [balao(cor, str(n)) for cor in ORDEM_DAS_CORES for n in range(10)]
    return folha(quadros, BW, BH, 10), len(quadros)


def confetes():
    """O confete: um papelzinho de 8 x 8 que gira - deitado e em pe -, em cada
    cor do balao."""
    quadros = []
    for cor in ORDEM_DAS_CORES:
        claro, meio, _ = (c + (255,) for c in CORES[cor])
        for deitado in (True, False):
            im = Image.new("RGBA", (8, 8), (0, 0, 0, 0))
            d = ImageDraw.Draw(im)
            if deitado:
                d.rectangle([0, 2, 7, 5], fill=meio)
                d.line([0, 2, 7, 2], fill=claro)
            else:
                d.rectangle([2, 0, 5, 7], fill=meio)
                d.line([2, 0, 2, 7], fill=claro)
            quadros.append(im)
    return folha(quadros, 8, 8, 8), len(quadros)


# Os cabecalhos dos ramos, na ordem dos quadros: (nome da animacao, texto).
# Os nomes e os textos sao os de Condicional::CaraDoRotulo e Rotulo.
ROTULOS = (
    [(f"Maior{a}", f"if (n > {a})") for a in (3, 4, 5, 6)]
    + [(f"Entre{a}e{b}", f"if (n > {a} && n < {b})") for a, b in ((2, 7), (3, 8), (1, 6))]
    + [("Par", "if (n % 2 == 0)"), ("Impar", "if (n % 2 == 1)")]
    + [(f"Menor{a}", f"if (n < {a})") for a in (3, 4)]
    + [(f"SenaoMenor{b}", f"else if (n < {b})") for b in (6, 7)]
    + [("Senao", "else")]
    + [(f"Caso{k}", f"case {k}:") for k in range(4)]
    + [(f"Atribui{a}", f"if (n = {a})") for a in (0, 7)]
)
RW, RH = 210, 30


def rotulo(texto, aceso):
    """Uma linha de codigo num editor escuro; acesa, com a moldura amarela de
    quando o confete daquele ramo cai."""
    s = 4
    im = Image.new("RGBA", (RW * s, RH * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fonte = ImageFont.truetype(CARLITO, 19 * s)
    largura = d.textlength(texto, font=fonte) / s + 24
    x0 = (RW - largura) / 2
    fundo = (70, 62, 24, 235) if aceso else (28, 32, 46, 225)
    borda = (255, 214, 72, 255) if aceso else (120, 140, 180, 230)
    d.rounded_rectangle([x0 * s, 1 * s, (RW - x0) * s, (RH - 2) * s], radius=5 * s,
                        fill=fundo, outline=borda, width=2 * s)
    tinta = (255, 240, 190, 255) if aceso else (214, 226, 250, 255)
    d.text((RW * s // 2, RH * s // 2), texto, fill=tinta, font=fonte, anchor="mm")
    return im.resize((RW, RH), Image.LANCZOS)


def rotulos():
    quadros = [rotulo(t, False) for _, t in ROTULOS] + [rotulo(t, True) for _, t in ROTULOS]
    return folha(quadros, RW, RH, 4), len(quadros)


IW, IH = 96, 28


def indice(i, estado):
    """O indice da casa i do vetor do chao: "v[5]" numa plaquinha."""
    s = 4
    im = Image.new("RGBA", (IW * s, IH * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fundo, borda, tinta = {
        "apagado": ((28, 32, 46, 200), (90, 104, 140, 220), (170, 184, 214, 255)),
        "aceso":   ((92, 78, 20, 240), (255, 214, 72, 255), (255, 244, 200, 255)),
        "pulado":  ((24, 40, 58, 220), (90, 170, 220, 255), (150, 210, 250, 255)),
        "erro":    ((110, 20, 24, 240), (255, 90, 80, 255), (255, 220, 214, 255)),
    }[estado]
    d.rounded_rectangle([2 * s, 2 * s, (IW - 3) * s, (IH - 3) * s], radius=5 * s,
                        fill=fundo, outline=borda, width=2 * s)
    d.text((IW * s // 2, IH * s // 2), f"v[{i}]", fill=tinta,
           font=ImageFont.truetype(CARLITO, 17 * s), anchor="mm")
    return im.resize((IW, IH), Image.LANCZOS)


ESTADOS_DO_INDICE = ["apagado", "aceso", "pulado", "erro"]   # a ordem dos quadros


def indices():
    """Os quadros: os doze indices apagados, depois os doze acesos, os doze
    pulados e os doze do segfault."""
    quadros = [indice(i, e) for e in ESTADOS_DO_INDICE for i in range(12)]
    return folha(quadros, IW, IH, 12), len(quadros)


BYTES = ["DE", "AD", "BE", "EF", "00", "FF"]   # a ordem de Laco::CaraDoByte


def bytes_():
    """Um byte da memoria despejada: dois digitos hexa num chip vermelho."""
    quadros = []
    for b in BYTES:
        s = 4
        im = Image.new("RGBA", (22 * s, 16 * s), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([1 * s, 1 * s, 20 * s, 14 * s], radius=3 * s,
                            fill=(150, 28, 34, 240), outline=(255, 120, 100, 255), width=1 * s)
        d.text((11 * s, 8 * s), b, fill=(255, 236, 220, 255),
               font=ImageFont.truetype(CARLITO, 11 * s), anchor="mm")
        quadros.append(im.resize((22, 16), Image.LANCZOS))
    return folha(quadros, 22, 16, 6), len(quadros)


def folha(quadros, largo, alto, colunas):
    linhas = (len(quadros) + colunas - 1) // colunas
    im = Image.new("RGBA", (largo * colunas, alto * linhas), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        im.alpha_composite(q, ((i % colunas) * largo, (i // colunas) * alto))
    return im


def folha_json(nome, w, h, largo, alto, quadros):
    frames = []
    colunas = w // largo
    for i in range(quadros):
        x, y = (i % colunas) * largo, (i // colunas) * alto
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": largo, "h": alto},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": largo, "h": alto},
            "sourceSize": {"w": largo, "h": alto},
            "duration": 120,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_andre.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def grava_folha(im, nome, largo, alto, quadros):
    destino = os.path.join(RAIZ, "Assets", "Teachers", "Projectiles")
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(folha_json(nome, im.width, im.height, largo, alto, quadros), f, indent=1)
    print(f"  {nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


def animacoes():
    """As animacoes de projeteis.json, para copiar de la para ca quando a
    folha mudar."""
    bal = {f"{cor}{n}": [i * 10 + n] for i, cor in enumerate(ORDEM_DAS_CORES) for n in range(10)}
    conf = {cor: [2 * i, 2 * i + 1] for i, cor in enumerate(ORDEM_DAS_CORES)}
    rot = {nome: [k] for k, (nome, _) in enumerate(ROTULOS)}
    rot.update({nome + "Aceso": [k + len(ROTULOS)] for k, (nome, _) in enumerate(ROTULOS)})
    sufixos = ["", "Aceso", "Pulado", "Erro"]
    ind = {f"V{i}{suf}": [k * 12 + i] for k, suf in enumerate(sufixos) for i in range(12)}
    byt = {b: [k] for k, b in enumerate(BYTES)}
    return {"Balao": bal, "Confete": conf, "Rotulo": rot, "Indice": ind, "Byte": byt}


if __name__ == "__main__":
    import sys
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    im, n = baloes()
    grava_folha(im, "DPIBHAndreBaloes", BW, BH, n)
    im, n = confetes()
    grava_folha(im, "DPIBHAndreConfete", 8, 8, n)
    im, n = rotulos()
    grava_folha(im, "DPIBHAndreRotulos", RW, RH, n)
    im, n = indices()
    grava_folha(im, "DPIBHAndreIndices", IW, IH, n)
    im, n = bytes_()
    grava_folha(im, "DPIBHAndreBytes", 22, 16, n)
