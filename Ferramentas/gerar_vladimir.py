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
from PIL import Image, ImageDraw, ImageFont
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
    # UM QUADRO SO E A FOLHA INTEIRA, quadrada ou nao: o diagrama do automato
    # e mais largo que alto, e um quadro quadrado o desenharia esticado.
    largo, alto = (lado, lado) if lado else (w, h)
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


# A VARIAVEL S DA GRAMATICA, e a palavra vazia. O S e quadrado e roxo, e nao
# redondo: ele NAO E LETRA do alfabeto - e o que ainda vai virar letras -, e o
# aluno precisa ver isso de longe. A vazia e um epsilon apagado, que some logo.
COR_S = (190, 150, 240, 255)
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")


def variavel():
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([2, 2, 29, 29], radius=6, fill=COR_S, outline=TRACO, width=2)
    d.text((16, 16), "S", fill=TRACO, font=ImageFont.truetype(CARLITO, 20), anchor="mm")
    return im


def vazia():
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([4, 4, 27, 27], outline=(200, 200, 220, 150), width=2)
    d.text((16, 15), "\u03b5", fill=(220, 220, 240, 200), font=ImageFont.truetype(CARLITO, 20), anchor="mm")
    return im


def alfabeto():
    """A folha dos simbolos, em 4x4: 4 quadros de "a", 4 de "b", 4 do S e 4 da
    palavra vazia.

    QUATRO DE CADA, e nao um, porque o leitor de animacao do projeto recebe uma
    LISTA de quadros por animacao ("animacoes": {"A": [0,1,2,3]}). Com um quadro
    so a animacao existe e nao anima; com quatro da para pulsar depois sem mexer
    em arquivo nenhum.

    AS DUAS LINHAS DE BAIXO VIERAM DEPOIS, com a gramatica da fase 2, e ficam
    DEPOIS das letras: os quadros 0 a 7 continuam onde estavam.
    """
    folha = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    for i in range(4):
        folha.alpha_composite(simbolo("a", COR_A), (i * 32, 0))
        folha.alpha_composite(simbolo("b", COR_B), (i * 32, 32))
        folha.alpha_composite(variavel(), (i * 32, 64))
        folha.alpha_composite(vazia(), (i * 32, 96))
    return folha


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


# ---------------------------------------------------------------------------
# A FASE 1: O AUTOMATO FINITO DESENHADO NO CAMPO
#
# Os tres estados sao aneis GRANDES, maiores que o cartao do professor (128 px):
# ele pousa DENTRO do estado em que esta, e o anel fica aparecendo em volta.
# O nome do estado fica embaixo, fora do cartao, para ser lido com ele em cima.
#
# O DIAGRAMA E UMA FIGURA SO, e nao uma seta por projetil: os estados tem lugar
# fixo no campo (Automato::kCentros, em Vladimir.cpp), entao as transicoes
# tambem tem - e uma figura so e o que deixa o desenho ser o da disciplina, com
# os lacos e as setas curvas, em vez de seis tiros parados.
# ---------------------------------------------------------------------------

LADO_DO_ESTADO = 192
RAIO_DO_ESTADO = 84

# Onde os estados ficam no campo (1200 x 800) e onde o diagrama comeca. Tem de
# bater com Vladimir::kCentrosDosEstados e kCentroDoDiagrama - ha teste.
CENTROS = [(220, 170), (600, 260), (980, 170)]
DIAGRAMA_X0, DIAGRAMA_Y0 = 100, 30
DIAGRAMA_W, DIAGRAMA_H = 1000, 400


def fonte(tamanho):
    return ImageFont.load_default(size=tamanho)


def estado(q, aceita, ativo):
    """Um estado do automato: o anel, o anel duplo se aceita, e o nome embaixo."""
    s = 2   # desenhado no dobro e reduzido: o anel sai liso
    im = Image.new("RGBA", (LADO_DO_ESTADO * s, LADO_DO_ESTADO * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    c = LADO_DO_ESTADO * s // 2
    r = RAIO_DO_ESTADO * s
    cor = (235, 245, 255, 255) if ativo else ESTADO
    if ativo:
        d.ellipse([c - r, c - r, c + r, c + r], fill=(122, 170, 220, 60))
    d.ellipse([c - r, c - r, c + r, c + r], outline=cor, width=(8 if ativo else 5) * s // 2 * 2)
    if aceita:
        r2 = r - 12 * s
        d.ellipse([c - r2, c - r2, c + r2, c + r2], outline=ACEITA if not ativo else cor, width=4 * s)
    im = im.resize((LADO_DO_ESTADO, LADO_DO_ESTADO), Image.LANCZOS)

    # O NOME, numa placa embaixo do anel: q0, q1, q2.
    d = ImageDraw.Draw(im)
    c = LADO_DO_ESTADO // 2
    d.rounded_rectangle([c - 22, 162, c + 22, 186], radius=6, fill=CARTAO, outline=cor, width=2)
    d.text((c, 174), f"q{q}", fill=TINTA, font=fonte(16), anchor="mm")
    return im


def estados():
    """A folha dos estados: q0, q1, q2 e os tres acesos, em 4 colunas."""
    folha = Image.new("RGBA", (LADO_DO_ESTADO * 4, LADO_DO_ESTADO * 2), (0, 0, 0, 0))
    for i in range(6):
        q, ativo = i % 3, i >= 3
        x, y = (i % 4) * LADO_DO_ESTADO, (i // 4) * LADO_DO_ESTADO
        folha.alpha_composite(estado(q, q == 2, ativo), (x, y))
    return folha


def _bezier(p0, p1, p2, n=48):
    pts = []
    for i in range(n + 1):
        u = i / n
        x = (1 - u) ** 2 * p0[0] + 2 * (1 - u) * u * p1[0] + u * u * p2[0]
        y = (1 - u) ** 2 * p0[1] + 2 * (1 - u) * u * p1[1] + u * u * p2[1]
        pts.append((x, y))
    return pts


def _na_borda(centro, rumo, folga=0):
    import math
    (cx, cy), (rx, ry) = centro, rumo
    n = math.hypot(rx, ry) or 1.0
    r = RAIO_DO_ESTADO + folga
    return (cx + rx / n * r, cy + ry / n * r)


def _seta(d, pts, cor, s):
    import math
    d.line(pts, fill=cor, width=3 * s, joint="curve")
    (x1, y1), (x2, y2) = pts[-4], pts[-1]
    a = math.atan2(y2 - y1, x2 - x1)
    L, W = 16 * s, 8 * s
    d.polygon([(x2, y2),
               (x2 - L * math.cos(a) + W * math.sin(a), y2 - L * math.sin(a) - W * math.cos(a)),
               (x2 - L * math.cos(a) - W * math.sin(a), y2 - L * math.sin(a) + W * math.cos(a))], fill=cor)


def _rotulo(d, onde, letra, s):
    cor = COR_A if letra == "a" else COR_B
    x, y = onde
    d.ellipse([x - 13 * s, y - 13 * s, x + 13 * s, y + 13 * s], fill=cor, outline=TRACO, width=2 * s)
    d.text((x, y), letra, fill=TRACO, font=fonte(18 * s), anchor="mm")


def diagrama():
    """As transicoes de "termina em ab", desenhadas entre os tres estados.

    Esmaecido de proposito: e o mapa, e nao um tiro - o que machuca sao as
    letras. Cada seta leva a letra que a dispara, na cor dela.
    """
    s = 2
    im = Image.new("RGBA", (DIAGRAMA_W * s, DIAGRAMA_H * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    q = [((x - DIAGRAMA_X0) * s, (y - DIAGRAMA_Y0) * s) for x, y in CENTROS]
    cor = (200, 215, 235, 150)
    R = RAIO_DO_ESTADO * s

    def borda(i, alvo, folga=6):
        (cx, cy) = q[i]
        return _na_borda((cx / s, cy / s), (alvo[0] - cx, alvo[1] - cy), folga)

    def em_s(p):
        return (p[0] * s, p[1] * s)

    def arco(i, j, curva, letra):
        meio = ((q[i][0] + q[j][0]) / 2, (q[i][1] + q[j][1]) / 2)
        dx, dy = q[j][0] - q[i][0], q[j][1] - q[i][1]
        import math
        n = math.hypot(dx, dy)
        ctrl = (meio[0] - dy / n * curva * s, meio[1] + dx / n * curva * s)
        a, b = em_s(borda(i, ctrl)), em_s(borda(j, ctrl))
        pts = _bezier(a, ctrl, b)
        _seta(d, pts, cor, s)
        _rotulo(d, pts[len(pts) // 2], letra, s)

    def laco(i, para_cima, letra):
        cx, cy = q[i]
        sinal = -1 if para_cima else 1
        a = (cx - 34 * s, cy + sinal * (R - 10 * s))
        b = (cx + 34 * s, cy + sinal * (R - 10 * s))
        ctrl = (cx, cy + sinal * (R + 110 * s))
        pts = _bezier(a, ctrl, b)
        _seta(d, pts, cor, s)
        _rotulo(d, (cx, cy + sinal * (R + 52 * s)), letra, s)

    arco(0, 1, 40, "a")     # q0 -a-> q1
    arco(1, 2, 40, "b")     # q1 -b-> q2
    arco(2, 1, 40, "a")     # q2 -a-> q1
    arco(2, 0, 150, "b")    # q2 -b-> q0, por cima
    laco(0, False, "b")     # q0 -b-> q0
    laco(1, False, "a")     # q1 -a-> q1

    # A SETA DE ENTRADA, que marca o estado inicial.
    cx, cy = q[0]
    pts = [(cx - R - 70 * s, cy), (cx - R - 40 * s, cy), (cx - R - 8 * s, cy), (cx - R - 4 * s, cy)]
    _seta(d, pts, cor, s)
    return im.resize((DIAGRAMA_W, DIAGRAMA_H), Image.LANCZOS)


# ---- O COMPILADOR (a fase 2, Compilador.h) ----------------------------------
#
# DUAS FAMILIAS DE FICHA, porque sao duas coisas: o LEXEMA - o pedaco do codigo
# fonte, num cartao claro como papel - e a INSTRUCAO que o gerador de codigo
# emite, num terminal escuro de letra verde. Os identificadores a e b nao tem
# ficha: sao as proprias letras do Vladimir (alfabeto()), que caem e trocam no
# Intercambio como toda letra.

LEXEMAS = [("X", "x"), ("Igual", "="), ("Mais", "+"), ("Menos", "-"), ("Vezes", "*"),
           ("Fim", ";")] + [(f"N{d}", str(d)) for d in range(1, 10)]
INSTRUCOES = ["LOAD", "PUSH", "ADD", "SUB", "MUL", "STORE"]
LADO_DA_FICHA = 48


def ficha(texto, terminal, borda=None):
    """Uma ficha de 44 x 26 num quadro de 48: o lexema no cartao claro, ou a
    instrucao no terminal escuro. A borda colorida e a das folhas a e b da
    arvore, na cor da letra."""
    s = 4
    L = LADO_DA_FICHA
    im = Image.new("RGBA", (L * s, L * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fundo = (24, 32, 30, 255) if terminal else (232, 236, 244, 255)
    borda = borda or ((110, 220, 140, 255) if terminal else (90, 110, 150, 255))
    tinta = (140, 240, 160, 255) if terminal else (34, 44, 64, 255)
    d.rounded_rectangle([2 * s, 11 * s, (L - 3) * s, (L - 12) * s], radius=4 * s,
                        fill=fundo, outline=borda, width=2 * s)
    tamanho = 15 if terminal else 19
    d.text((L * s // 2, L * s // 2), texto, fill=tinta,
           font=ImageFont.truetype(CARLITO, tamanho * s), anchor="mm")
    return im.resize((L, L), Image.LANCZOS)


def fichas():
    """Os lexemas, depois as instrucoes e as folhas a e b, quatro por linha."""
    todas = ([ficha(t, False) for _, t in LEXEMAS] + [ficha(i, True) for i in INSTRUCOES]
             + [ficha("a", False, COR_A), ficha("b", False, COR_B)])
    L = LADO_DA_FICHA
    linhas = (len(todas) + 3) // 4
    f = Image.new("RGBA", (4 * L, linhas * L), (0, 0, 0, 0))
    for k, im in enumerate(todas):
        f.alpha_composite(im, ((k % 4) * L, (k // 4) * L))
    return f, len(todas)


if __name__ == "__main__":
    grava(vladimir(), "DPIBHVladimir", "Assets/Teachers")
    grava(alfabeto(), "DPIBHVladimirSimbolo", "Assets/Teachers/Projectiles",
          quadros=16, lado=32)
    grava(estados(), "DPIBHVladimirEstado", "Assets/Teachers/Projectiles",
          quadros=6, lado=LADO_DO_ESTADO)
    grava(diagrama(), "DPIBHVladimirDiagrama", "Assets/Teachers/Projectiles")
    folha, n = fichas()
    grava(folha, "DPIBHVladimirFicha", "Assets/Teachers/Projectiles", quadros=n, lado=LADO_DA_FICHA)

