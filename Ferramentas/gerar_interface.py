#!/usr/bin/env python3
"""Arte da interface que sai de codigo, e nao do Aseprite.

  DPIBHSelos      os selos das mencoes (SelosDeMencao), em pixel art de 13 x 10
                  desenhada em 2x: uma moeda para as notas (B, P, O, 100), uma
                  placa para o Dificil, a dupla e o TCC, e um escudo para os
                  desafios. Cada selo tem o quadro aceso, na cor da mencao, e o
                  apagado. Os quadros vao na ordem de Mencoes::kTodas: o aceso
                  da mencao i e o quadro 2i, o apagado o 2i + 1.
  DPIBHLosango    o losango de cada materia da selecao de fases, em pixel art de
                  64 x 32 desenhada em 2x. Os quadros: aberta (0), em foco (1 a
                  8, o reflexo que passa pela pedra), aprovada (9), fechada (10)
                  e fechada em foco (11).

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
    ("escudo", (140, 225, 160), "Q"),   # quietinho da turma
    ("escudo", (255, 150, 205), "I"),   # intocavel
    ("escudo", (100, 225, 215), "A"),   # prestando atencao
    ("escudo", (255, 160, 60),  "F"),   # sentar na frente
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
    "Q": ["010", "101", "101", "110", "011"],
    "A": ["010", "101", "111", "101", "101"],
    "F": ["111", "100", "110", "100", "100"],
    "1": ["01", "11", "01", "01", "01"],
    "0": ["010", "101", "101", "101", "010"],
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
#
# Pixel art de 64 x 32, desenhada em 2x (128 x 64) pelo pixel mais proximo. As tecnicas:
#   - aresta 2:1, a escada limpa de dois pixels por degrau, sem pixel solto;
#   - rampa de sete tons com mudanca de matiz: a sombra puxa para o azul-violeta, a luz para o
#     amarelo, e nao so escurece e clareia;
#   - contorno seletivo: escuro do lado da sombra, um tom da propria cor do lado da luz;
#   - luz de borda no lado de cima e da esquerda, e luz refletida embaixo, um tom acima da sombra;
#   - lapidacao: a mesa (o losango do meio) e quatro facetas, cada uma num tom;
#   - pontilhado de xadrez so na passagem entre a mesa e as facetas;
#   - o brilho especular em tres pixels, e no foco um reflexo que atravessa a pedra.

import colorsys

LL, LA = 64, 32            # o quadro, em pixels de arte (o jogo desenha em 2x: 128 x 64)
MX, MY = LL // 2, LA // 2

QUADROS_DO_REFLEXO = 8     # o foco e uma animacao: o reflexo passa e a pedra descansa

ESTADOS = [
    # nome,             cor de base,      fechada
    ("aberta",          (60, 92, 220),    False),
    ("foco",            (232, 58, 64),    False),
    ("aprovada",        (46, 170, 108),   False),
    ("fechada",         (84, 88, 116),    True),
    ("fechada-foco",    (128, 134, 178),  True),
]


def rampa(base):
    """Sete tons, do contorno ao especular, com a matiz andando: a sombra fria, a luz quente."""
    r, g, b = (c / 255 for c in base)
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    tons = []
    #          valor  saturacao  matiz (para onde anda, e quanto)
    passos = [(0.30, 1.10, 0.70, 0.30),    # 0 contorno da sombra
              (0.52, 1.05, 0.70, 0.20),    # 1 sombra
              (0.74, 1.00, 0.70, 0.10),    # 2 meia-sombra
              (1.00, 1.00, None, 0.00),    # 3 a cor
              (1.18, 0.82, 0.14, 0.08),    # 4 luz
              (1.32, 0.55, 0.14, 0.14),    # 5 luz de borda
              (1.45, 0.18, 0.14, 0.20)]    # 6 especular
    for fv, fs, alvo, quanto in passos:
        hh = h
        if alvo is not None and s > 0.08:
            # Anda pelo caminho mais curto do circulo das matizes.
            d = ((alvo - h + 0.5) % 1.0) - 0.5
            hh = (h + d * quanto) % 1.0
        rr, gg, bb = colorsys.hsv_to_rgb(hh, min(1.0, s * fs), min(1.0, v * fv))
        tons.append((round(rr * 255), round(gg * 255), round(bb * 255)))
    return tons


def meia_largura(y):
    """A aresta 2:1: em cada linha o losango cresce (ou encolhe) dois pixels de cada lado."""
    return 2 * (y + 1) if y < MY else 2 * (LA - y)


def dentro(x, y):
    if not (0 <= x < LL and 0 <= y < LA):
        return False
    w = meia_largura(y)
    return MX - w <= x <= MX + w - 1


def na_borda(x, y):
    return dentro(x, y) and any(not dentro(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))


def distancia(x, y):
    """0 no centro, 1 na aresta."""
    return abs(x + 0.5 - MX) / MX + abs(y + 0.5 - MY) / MY


def losango(cor, fechada, reflexo=None):
    t = rampa(cor)
    im = Image.new("RGBA", (LL, LA), (0, 0, 0, 0))
    px = im.load()
    for y in range(LA):
        for x in range(LL):
            if not dentro(x, y):
                continue
            cima, esquerda = y < MY, x < MX
            if na_borda(x, y):
                # Contorno seletivo: o lado da luz leva um tom da cor, o da sombra o mais escuro.
                c = t[1] if (cima and esquerda) else t[0]
                if cima and not esquerda:
                    c = t[1] if (x + y) % 2 else t[0]   # a quina de cima-direita meio a meio
                px[x, y] = c + (255,)
                continue
            # A segunda volta: luz de borda em cima-esquerda, refletida embaixo-direita.
            colado = any(na_borda(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            d = distancia(x, y)
            mesa = d < 0.50
            if mesa:
                faces = {(True, True): t[4], (True, False): t[3], (False, True): t[3], (False, False): t[2]}
            else:
                faces = {(True, True): t[4], (True, False): t[3], (False, True): t[2], (False, False): t[1]}
            c = faces[(cima, esquerda)]
            # A passagem da mesa para as facetas, em xadrez de um pixel.
            if 0.47 <= d < 0.56 and (x + y) % 2 == 0:
                c = faces[(cima, esquerda)] if mesa else {(True, True): t[5], (True, False): t[4],
                                                          (False, True): t[3], (False, False): t[2]}[(cima, esquerda)]
            if colado:
                if cima and esquerda:
                    c = t[5]
                elif cima:
                    c = t[4]
                elif not esquerda:
                    c = t[2]   # a luz refletida: um tom acima da sombra, colada na aresta
            px[x, y] = c + (255,)

    if not fechada:
        # O especular: tres pixels no canto de cima-esquerda da mesa.
        for (x, y) in ((MX - 10, MY - 5), (MX - 9, MY - 5), (MX - 11, MY - 4)):
            px[x, y] = t[6] + (255,)
        # E um risco curto de luz rente a aresta de cima-esquerda.
        for k in range(4):
            x, y = MX - 18 + 2 * k, MY - 8 - k
            if dentro(x, y) and not na_borda(x, y):
                px[x, y] = t[6] + (255,)
        if reflexo is not None:
            # O reflexo do foco: uma faixa inclinada, de dois tons, que atravessa a pedra.
            for y in range(LA):
                for x in range(LL):
                    if not dentro(x, y) or na_borda(x, y):
                        continue
                    k = x + 2 * y - reflexo
                    if 0 <= k < 3:
                        px[x, y] = t[6] + (255,)
                    elif 3 <= k < 6 or -3 <= k < 0:
                        px[x, y] = t[5] + (255,)
    else:
        corrente(px, t)
        cadeado(px)
    return im


def corrente(px, t):
    # Duas correntes em X, do meio de uma aresta ao meio da oposta: elos de frente (um anel de
    # 3 x 3) a cada 5 pixels, ligados pelo elo visto de lado (um traco de 2), sem buraco entre eles.
    escuro, meio, claro = t[0], t[4], t[6]

    def ponto(x, y, c):
        if dentro(x, y) and not na_borda(x, y) and not (abs(x - MX) <= 5 and abs(y - MY) <= 5):
            px[x, y] = c + (255,)

    for sobe in (False, True):
        for i in range(-6, 7):
            if i == 0:
                continue
            cx = MX + 5 * i
            cy = MY + (-1 if sobe else 1) * round(2.5 * i) - (1 if sobe else 0)
            for dx, dy in ((-1, -1), (0, -1), (1, -1), (-1, 0), (1, 0), (-1, 1), (0, 1), (1, 1)):
                luz = dy < 0 or (dy == 0 and dx < 0)
                ponto(cx + dx, cy + dy, claro if luz else meio)
            ponto(cx + 1, cy + 2, escuro)
            ponto(cx + 2, cy + 1, escuro)
            # O elo de lado, ate o proximo anel.
            ny = cy + (-1 if sobe else 1) * (round(2.5 * (i + 1)) - round(2.5 * i))
            mx, my = cx + 2, (cy + ny) // 2 + (0 if sobe else 1)
            ponto(mx, my, meio)
            ponto(mx + 1, my, meio)
            ponto(mx, my + 1, escuro)
            ponto(mx + 1, my + 1, escuro)


def cadeado(px):
    ouro = rampa((214, 168, 60))
    # O arco (a alca), 5 de largura, e o corpo, 9 x 6, com a fechadura.
    arco = ["..xxxxx..",
            ".x.....x.",
            ".x.....x."]
    corpo = ["XXXXXXXXX",
             "XhhhhhhmX",
             "XhmmkmmmX",
             "XmmmkmmsX",
             "XmssssssX",
             "XXXXXXXXX"]
    x0, y0 = MX - 5, MY - 6
    for j, linha in enumerate(arco):
        for i, ch in enumerate(linha):
            if ch == "x":
                px[x0 + i, y0 + j] = (ouro[2] if i > 4 else ouro[4]) + (255,)
    cores = {"X": ouro[0], "h": ouro[5], "m": ouro[3], "s": ouro[1], "k": (30, 20, 26)}
    for j, linha in enumerate(corpo):
        for i, ch in enumerate(linha):
            px[x0 + i, y0 + 3 + j] = cores[ch] + (255,)


def gerar_losango():
    quadros = []
    for nome, cor, fechada in ESTADOS:
        if nome == "foco":
            # O reflexo entra pela esquerda e sai pela direita; os ultimos quadros descansam.
            for k in range(QUADROS_DO_REFLEXO):
                faixa = 24 + k * 14 if k < 6 else None
                quadros.append(losango(cor, fechada, faixa))
        else:
            quadros.append(losango(cor, fechada))
    folha = Image.new("RGBA", (LL * len(quadros), LA), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        folha.paste(q, (i * LL, 0))
    gravar("DPIBHLosango", folha, LL, LA, len(quadros))


if __name__ == "__main__":
    gerar_selos()
    gerar_losango()
    print("ok: Assets/Icons/DPIBHSelos e DPIBHLosango")
