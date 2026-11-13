#!/usr/bin/env python3
"""Arte da interface que sai de codigo, e nao do Aseprite.

  DPIBHSelos      os selos das mencoes (SelosDeMencao), em pixel art de 13 x 10
                  desenhada em 2x: uma moeda para as notas (B, P, O, 100), uma
                  placa para o Dificil, a dupla e o TCC, e um escudo para os
                  desafios. Cada selo tem o quadro aceso, na cor da mencao, e o
                  apagado. Os quadros vao na ordem de Mencoes::kTodas: o aceso
                  da mencao i e o quadro 2i, o apagado o 2i + 1.
  DPIBHLosango    o losango de cada materia da selecao de fases, em 256 x 128,
                  o dobro do tamanho em que e desenhado: a tela cheia o mostra
                  sem serrilhado. Cinco quadros: aberta, em foco, aprovada,
                  fechada e fechada em foco.

O sombreamento e o de pixel art: a luz vem de cima e da esquerda, cada cor tem
tres ou quatro tons (sem degrade), o contorno e o tom mais escuro dela, e um
pontilhado de xadrez faz a passagem de um tom para o outro.

  python3 Ferramentas/gerar_interface.py
"""
from PIL import Image
import json, math, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ICONES = os.path.join(RAIZ, "Assets", "Icons")


def misturar(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


def clarear(c, t):
    return misturar(c, (255, 255, 255), t)


def escurecer(c, t):
    # Escurecer puxando para o azul-noite, e nao para o preto: a sombra fica viva.
    return misturar(c, (16, 14, 40), t)


def folha_json(nome, largura, altura, largo, alto, quadros):
    frames = []
    for i in range(quadros):
        frames.append({
            "filename": f"{nome} {i}.png",
            "frame": {"x": i * largo, "y": 0, "w": largo, "h": alto},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": largo, "h": alto},
            "sourceSize": {"w": largo, "h": alto}, "duration": 100,
        })
    return {"frames": frames,
            "meta": {"app": "Ferramentas/gerar_interface.py", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": largura, "h": altura}, "scale": "1"}}


def gravar(nome, im, largo, alto, quadros):
    im.save(os.path.join(ICONES, nome + ".png"))
    with open(os.path.join(ICONES, nome + ".json"), "w") as f:
        json.dump(folha_json(nome, im.width, im.height, largo, alto, quadros), f, indent=1)
        f.write("\n")


# ---------------------------------------------------------------- os selos ----

SL, SA = 13, 10   # o selo, em pixels de arte (o jogo desenha em 2x)

# As cores e os simbolos de Mencoes.cpp (CorDa e Selo), na ordem de kTodas.
MENCOES = [
    ("moeda",  (214, 138, 72),  "B"),
    ("moeda",  (204, 212, 224), "P"),
    ("moeda",  (255, 214, 72),  "O"),
    ("moeda",  (255, 244, 170), "100"),
    ("placa",  (240, 104, 104), "D"),
    ("placa",  (110, 200, 255), "2"),
    ("placa",  (186, 158, 250), "T"),
    ("escudo", (140, 225, 160), "Ø"),
    ("escudo", (255, 150, 205), "I"),
    ("escudo", (100, 225, 215), "M"),
    ("escudo", (255, 160, 60),  "!!"),
]

# Letras de 5 de altura.
GLIFOS = {
    "B": ["110", "101", "110", "101", "110"],
    "P": ["110", "101", "110", "100", "100"],
    "O": ["010", "101", "101", "101", "010"],
    "D": ["110", "101", "101", "101", "110"],
    "T": ["111", "010", "010", "010", "010"],
    "I": ["111", "010", "010", "010", "111"],
    "2": ["110", "001", "010", "100", "111"],
    "M": ["10001", "11011", "10101", "10001", "10001"],
    "Ø": ["01111", "10011", "10101", "11001", "11110"],
    "1": ["01", "11", "01", "01", "01"],
    "0": ["010", "101", "101", "101", "010"],
    "!": ["1", "1", "1", "0", "1"],
}


def forma(tipo, x, y):
    """O pixel (x, y) esta dentro do selo?"""
    if tipo == "moeda":
        # Redonda: os cantos cortados em degraus, mais nas pontas.
        corte = [3, 1, 0, 0, 0, 0, 0, 0, 1, 3][y]
        return corte <= x <= SL - 1 - corte
    if tipo == "placa":
        canto = (x in (0, SL - 1)) and (y in (0, SA - 1))
        return not canto
    # escudo: reto em cima, a ponta embaixo
    if y < SA - 3:
        return not ((x in (0, SL - 1)) and y == 0)
    falta = y - (SA - 4)
    return falta <= x <= SL - 1 - falta


def texto_em_pixels(simbolo):
    colunas = []
    for i, letra in enumerate(simbolo):
        g = GLIFOS[letra]
        if i > 0:
            colunas.append("0" * 5)
        for c in range(len(g[0])):
            colunas.append("".join(g[l][c] for l in range(5)))
    return colunas   # cada coluna: 5 caracteres, de cima para baixo


def selo(tipo, cor, simbolo, aceso):
    im = Image.new("RGBA", (SL, SA), (0, 0, 0, 0))
    px = im.load()
    dentro = [[forma(tipo, x, y) for x in range(SL)] for y in range(SA)]

    def borda(x, y):
        if not dentro[y][x]:
            return False
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < SL and 0 <= ny < SA) or not dentro[ny][nx]:
                return True
        return False

    if aceso:
        contorno = escurecer(cor, 0.72)
        luz, meio, sombra, fundo = clarear(cor, 0.45), cor, escurecer(cor, 0.28), escurecer(cor, 0.45)
        tinta = escurecer(cor, 0.86)
    else:
        contorno = (58, 66, 92)
        luz, meio, sombra, fundo = (36, 42, 62), (28, 33, 50), (22, 26, 40), (19, 22, 34)
        tinta = (74, 84, 110)

    for y in range(SA):
        for x in range(SL):
            if not dentro[y][x]:
                continue
            if borda(x, y):
                px[x, y] = contorno + (255,)
                continue
            # A luz vem de cima e da esquerda: a faixa pela soma das coordenadas.
            t = (x / (SL - 1)) * 0.35 + (y / (SA - 1)) * 0.65
            if t < 0.28:
                c = luz
            elif t < 0.38:
                c = luz if (x + y) % 2 == 0 else meio     # o pontilhado da passagem
            elif t < 0.62:
                c = meio
            elif t < 0.72:
                c = meio if (x + y) % 2 == 0 else sombra
            elif t < 0.86:
                c = sombra
            else:
                c = fundo
            px[x, y] = c + (255,)

    if aceso:
        # O brilho: dois pixels quase brancos no canto de cima, perto do contorno.
        for (x, y) in ((2, 1), (1, 2), (3, 1)):
            if dentro[y][x] and not borda(x, y):
                px[x, y] = clarear(cor, 0.85) + (255,)

    colunas = texto_em_pixels(simbolo)
    largura = len(colunas)
    x0 = (SL - largura) // 2 + (1 if (SL - largura) % 2 and largura < 5 else 0)
    y0 = 2
    tinta_px = set()
    for i, col in enumerate(colunas):
        for l, ch in enumerate(col):
            if ch == "1":
                tinta_px.add((x0 + i, y0 + l))
    for (x, y) in tinta_px:
        px[x, y] = tinta + (255,)
    return im


def gerar_selos():
    quadros = []
    for tipo, cor, simbolo in MENCOES:
        quadros.append(selo(tipo, cor, simbolo, True))
        quadros.append(selo(tipo, cor, simbolo, False))
    folha = Image.new("RGBA", (SL * len(quadros), SA), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        folha.paste(q, (i * SL, 0))
    gravar("DPIBHSelos", folha, SL, SA, len(quadros))


# -------------------------------------------------------------- o losango ----

LL, LA = 256, 128          # o quadro (o jogo desenha em 128 x 64)
CX, CY = LL / 2, LA / 2

ESTADOS = [
    # nome,             cor de base,      fechada
    ("aberta",          (64, 86, 214),    False),
    ("foco",            (226, 62, 62),    False),
    ("aprovada",        (56, 158, 112),   False),
    ("fechada",         (70, 74, 100),    True),
    ("fechada-foco",    (112, 118, 160),  True),
]


def distancia(x, y):
    """0 no centro, 1 na borda do losango."""
    return abs(x + 0.5 - CX) / (LL / 2 - 2) + abs(y + 0.5 - CY) / (LA / 2 - 2)


def losango(cor, fechada):
    im = Image.new("RGBA", (LL, LA), (0, 0, 0, 0))
    px = im.load()
    contorno = (10, 10, 22)
    aro = clarear(cor, 0.55)
    tons = {  # as quatro faces: a de cima-esquerda recebe a luz
        "ce": clarear(cor, 0.22), "cd": cor, "be": escurecer(cor, 0.18), "bd": escurecer(cor, 0.36),
    }
    friso = clarear(cor, 0.35)
    for y in range(LA):
        for x in range(LL):
            d = distancia(x, y)
            if d > 1.0:
                continue
            if d > 0.955:
                px[x, y] = contorno + (255,)
                continue
            if d > 0.915:
                # O aro claro, mais escuro embaixo: a luz de cima.
                c = aro if y < CY else escurecer(aro, 0.25)
                px[x, y] = c + (255,)
                continue
            if d > 0.89:
                px[x, y] = escurecer(cor, 0.55) + (255,)
                continue
            face = ("c" if y + 0.5 < CY else "b") + ("e" if x + 0.5 < CX else "d")
            c = tons[face]
            # Um friso interno, um losango menor: a pedra lapidada.
            if 0.60 < d < 0.635:
                c = friso if face == "ce" else (clarear(c, 0.18) if face != "bd" else escurecer(c, 0.1))
            # A passagem entre as faces de cima, em pontilhado.
            if abs(x + 0.5 - CX) < 2.0 and y < CY and (x + y) % 2 == 0:
                c = tons["ce"]
            px[x, y] = c + (255,)

    if not fechada:
        # O brilho: um risco de luz paralelo a aresta de cima-esquerda.
        for y in range(LA):
            for x in range(LL):
                d = distancia(x, y)
                if 0.70 < d < 0.80 and x + 0.5 < CX - 22 and y + 0.5 < CY - 8:
                    if (x + y) % 2 == 0 or 0.73 < d < 0.77:
                        px[x, y] = clarear(cor, 0.62) + (255,)
        for (bx, by) in ((CX - 52, CY - 30), (CX - 50, CY - 31)):
            for dy in range(3):
                for dx in range(3):
                    px[int(bx) + dx, int(by) + dy] = (255, 255, 255, 255)
    else:
        corrente(px, cor)
        cadeado(px, cor)
    return im


def corrente(px, cor):
    escuro, meio, claro = escurecer(cor, 0.62), clarear(cor, 0.12), clarear(cor, 0.45)
    # Duas correntes em X, de um meio de aresta ao oposto, com elos alternando de pe e deitados.
    for sinal in (1, -1):
        x0, y0 = CX - LL * 0.22, CY - sinal * LA * 0.22
        x1, y1 = CX + LL * 0.22, CY + sinal * LA * 0.22
        comprimento = math.hypot(x1 - x0, y1 - y0)
        passos = int(comprimento // 13)
        for i in range(passos + 1):
            t = i / passos
            x, y = x0 + (x1 - x0) * t, y0 + (y1 - y0) * t
            if abs(x - CX) < 20 and abs(y - CY) < 20:
                continue   # o cadeado fica no meio
            deitado = i % 2 == 0
            rx, ry = (8, 4.5) if deitado else (4, 6.5)
            for yy in range(int(y - ry - 2), int(y + ry + 3)):
                for xx in range(int(x - rx - 2), int(x + rx + 3)):
                    if not (0 <= xx < LL and 0 <= yy < LA) or distancia(xx, yy) > 0.88:
                        continue
                    e = ((xx + 0.5 - x) / rx) ** 2 + ((yy + 0.5 - y) / ry) ** 2
                    if 0.40 < e < 1.0:
                        luz = (yy + 0.5 - y) < 0
                        px[xx, yy] = (claro if luz else meio) + (255,)
                    elif 1.0 <= e < 1.6:
                        px[xx, yy] = escuro + (255,)


def cadeado(px, cor):
    metal, luz, sombra, escuro = (196, 170, 92), (246, 226, 150), (142, 112, 52), (40, 30, 24)
    # O arco.
    for yy in range(int(CY - 22), int(CY - 4)):
        for xx in range(int(CX - 14), int(CX + 14)):
            e = math.hypot(xx + 0.5 - CX, yy + 0.5 - (CY - 6))
            if 8 <= e <= 13 and yy < CY - 4:
                px[xx, yy] = (escuro if e > 11.5 or e < 9 else (luz if xx < CX else metal)) + (255,)
    # O corpo.
    for yy in range(int(CY - 6), int(CY + 16)):
        for xx in range(int(CX - 16), int(CX + 16)):
            bx, by = xx - (CX - 16), yy - (CY - 6)
            if bx < 2 or bx >= 30 or by < 2 or by >= 20:
                c = escuro
            elif by < 6:
                c = luz
            elif by > 15:
                c = sombra
            else:
                c = metal if (bx > 4 or (bx + by) % 2) else luz
            px[xx, yy] = c + (255,)
    # O buraco da chave.
    for yy in range(int(CY), int(CY + 11)):
        for xx in range(int(CX - 3), int(CX + 3)):
            r = math.hypot(xx + 0.5 - CX, yy + 0.5 - (CY + 3))
            if r <= 3 or (yy > CY + 3 and abs(xx + 0.5 - CX) <= 1.5):
                px[xx, yy] = escuro + (255,)


def gerar_losango():
    quadros = [losango(cor, fechada) for _, cor, fechada in ESTADOS]
    folha = Image.new("RGBA", (LL * len(quadros), LA), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        folha.paste(q, (i * LL, 0))
    gravar("DPIBHLosango", folha, LL, LA, len(quadros))


if __name__ == "__main__":
    gerar_selos()
    gerar_losango()
    print("ok: Assets/Icons/DPIBHSelos e DPIBHLosango")
