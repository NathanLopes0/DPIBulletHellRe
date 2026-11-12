#!/usr/bin/env python3
"""Arte PROVISORIA do Luizinho (INF 340 - Linguagens de Programacao) e dos
projeteis dele.

Mesmo estilo de cartao do Julio, do Thiago, do Vladimir, do Vitor, da Maria
Lucia e do Lucas: e deliberadamente um placeholder, para nao se passar por arte
final. Trocar o PNG basta - o json de quadro continua valendo.

O MOTIVO DO CARTAO E UMA COLUNA E UM PERGAMINHO: o professor que conta de onde
vem cada palavra da materia, e quase sempre ela vem do latim. A coluna e o
pergaminho dizem "latim" sem escrever uma palavra.

OS PROJETEIS, um par de animacoes por assunto:

    Resposta  X1..X9    o valor de x que o bloco dono atira na fase 1, as
                        amarracoes: o algarismo num hexagono ambar
                        (DPIBHLuizinhoRespostas).
    Valor     Numero0   o numero num chip azul (o operando, e o anel do
              ...20     resultado com o n escrito), o texto num verde com as
              Texto0    aspas (o operando), e as letras da palavra do
              ...9      resultado, sem aspas, com as aspas em chips proprios
              Letra0..9 (DPIBHLuizinhoValores).
              Aspas
    Operador  Mais      o losango onde a conta roda; MaisErro e VezesErro
              Vezes     quando ela da erro (DPIBHLuizinhoOperadores).
    Erro      TypeError o estilhaco do erro de execucao, e o "erro de tipo"
              Compilacao da compilacao (DPIBHLuizinhoErros).
    Volta     uma volta do laco da fase 3, o programa Termina: um anel com
              a ponta de seta (DPIBHLuizinhoVoltas).
    Contradicao  o caco do Termina que se contradiz e estoura: o simbolo da
              contradicao num chip vermelho (DPIBHLuizinhoContradicao).
    Excecao   o nome do tipo num chip vermelho: a excecao da final, que sobe
              pela pilha (DPIBHLuizinhoExcecoes).
    Destroco  Destroco  o pedaco da faixa desempilhada, e o anel do finally
              Finally   (DPIBHLuizinhoPilha).
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")

CARTAO = (74, 58, 50, 255)
BORDA  = (206, 176, 112, 255)
TINTA  = (242, 234, 214, 255)
TRACO  = (34, 28, 24, 255)

PEDRA  = (176, 170, 158, 255)
SULCO  = (92, 86, 78, 255)
COURO  = (128, 76, 44, 255)


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
        "meta": {"app": "gerar_luizinho.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def luizinho():
    """O cartao do professor: uma coluna a esquerda e um pergaminho a direita."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A COLUNA: capitel, fuste com caneluras e base.
    d.rectangle([14, 16, 50, 22], fill=PEDRA, outline=TRACO)          # abaco
    d.ellipse([12, 20, 22, 30], fill=PEDRA, outline=TRACO)            # voluta
    d.ellipse([42, 20, 52, 30], fill=PEDRA, outline=TRACO)
    d.rectangle([20, 24, 44, 82], fill=PEDRA, outline=TRACO)          # fuste
    for x in (26, 32, 38):
        d.line([(x, 28), (x, 78)], fill=SULCO, width=1)
    d.rectangle([16, 82, 48, 88], fill=PEDRA, outline=TRACO)          # base

    # O PERGAMINHO: um rolo aberto, com as linhas do texto.
    d.rectangle([62, 26, 110, 80], fill=TINTA, outline=TRACO)
    d.ellipse([58, 20, 68, 32], fill=COURO, outline=TRACO)
    d.ellipse([104, 20, 114, 32], fill=COURO, outline=TRACO)
    d.ellipse([58, 74, 68, 86], fill=COURO, outline=TRACO)
    d.ellipse([104, 74, 114, 86], fill=COURO, outline=TRACO)
    for i, y in enumerate(range(36, 74, 7)):
        d.line([(68, y), (104 - (i % 2) * 10, y)], fill=SULCO, width=2)

    d.text((40, 98), "LUIZINHO", fill=TINTA)
    return im


# ---- A FASE 2: OS SISTEMAS DE TIPOS -----------------------------------------

VW = 28   # o chip do valor: o colisor e metade disto

NUMERO = ((30, 52, 116, 240), (120, 170, 255, 255), (220, 232, 255, 255))   # fundo, borda, tinta
TEXTO  = ((22, 70, 40, 240), (110, 220, 130, 255), (214, 255, 220, 255))


def chip(texto, cores, tamanho=19):
    """Um valor num chip: o numero no azul, o texto no verde."""
    s = 4
    fundo, borda, tinta = cores
    im = Image.new("RGBA", (VW * s, VW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([2 * s, 2 * s, (VW - 3) * s, (VW - 3) * s], radius=7 * s,
                        fill=fundo, outline=borda, width=2 * s)
    d.text((VW * s // 2, VW * s // 2), texto, fill=tinta,
           font=ImageFont.truetype(CARLITO, tamanho * s), anchor="mm")
    return im.resize((VW, VW), Image.LANCZOS)


MAIOR_NUMERO = 20   # Coercao::kMaiorNumero


def valores_quadros():
    """Os quadros, na ordem das animacoes: Numero0 a Numero20, Texto0 a Texto9
    (o algarismo entre aspas: o operando), Letra0 a Letra9 (a letra da palavra,
    sem aspas) e Aspas."""
    q = [(f"Numero{n}", chip(str(n), NUMERO, 19 if n < 10 else 16)) for n in range(MAIOR_NUMERO + 1)]
    q += [(f"Texto{n}", chip(f'"{n}"', TEXTO, 15)) for n in range(10)]
    q += [(f"Letra{n}", chip(str(n), TEXTO, 19)) for n in range(10)]
    q += [("Aspas", chip('"', TEXTO, 24))]
    return q


OW = 40   # o operador


def operador(simbolo, erro):
    """O operador onde a conta roda: um losango com o simbolo; vermelho quando
    ela da erro."""
    s = 4
    im = Image.new("RGBA", (OW * s, OW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fundo, borda, tinta = (((110, 20, 24, 240), (255, 96, 80, 255), (255, 226, 220, 255)) if erro
                           else ((60, 52, 20, 235), (255, 214, 72, 255), (255, 244, 200, 255)))
    m = OW * s // 2
    r = (OW // 2 - 2) * s
    d.polygon([(m, m - r), (m + r, m), (m, m + r), (m - r, m)], fill=fundo, outline=borda)
    d.polygon([(m, m - r + 2 * s), (m + r - 2 * s, m), (m, m + r - 2 * s), (m - r + 2 * s, m)], outline=borda)
    # O ASTERISCO DA CARLITO E UM SOBRESCRITO: maior e mais baixo, ele fica no
    # meio do losango como o +.
    if simbolo == "*":
        d.text((m, m + 7 * s), simbolo, fill=tinta, font=ImageFont.truetype(CARLITO, 36 * s), anchor="mm")
    else:
        d.text((m, m), simbolo, fill=tinta, font=ImageFont.truetype(CARLITO, 22 * s), anchor="mm")
    return im.resize((OW, OW), Image.LANCZOS)


OPERADORES = [("Mais", "+", False), ("Vezes", "*", False), ("MaisErro", "+", True), ("VezesErro", "*", True)]


EW, EH = 84, 22   # o estilhaco do erro: largo, e o colisor e a altura


def estilhaco(texto, cores):
    s = 4
    fundo, borda, tinta = cores
    im = Image.new("RGBA", (EW * s, EH * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1 * s, 1 * s, (EW - 2) * s, (EH - 2) * s], radius=5 * s,
                        fill=fundo, outline=borda, width=2 * s)
    d.text((EW * s // 2, EH * s // 2), texto, fill=tinta,
           font=ImageFont.truetype(CARLITO, 15 * s), anchor="mm")
    return im.resize((EW, EH), Image.LANCZOS)


ERROS = [("TypeError", "TypeError", ((150, 24, 30, 240), (255, 110, 90, 255), (255, 232, 226, 255))),
         ("Compilacao", "erro de tipo", ((70, 20, 60, 235), (220, 120, 230, 255), (250, 226, 255, 255)))]


# ---- AS FASES 1 E 3: AS AMARRACOES E O PROGRAMA TERMINA ----------------------

# A fase 1: o ambar do canto do bloco dono (Luizinho.cpp, kCantoDoDono).
AW = 24   # o valor atirado: o colisor e metade disto
AMARRA = ((96, 64, 12, 240), (255, 206, 92, 255), (255, 244, 214, 255))   # fundo, borda, tinta
VALORES_DE_X = range(1, 10)


def resposta(valor):
    """O valor de x que o bloco dono atira: o algarismo num hexagono ambar."""
    import math
    s = 4
    im = Image.new("RGBA", (AW * s, AW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fundo, borda, tinta = AMARRA
    c = AW * s / 2

    def hexagono(r):
        return [(c + r * math.cos(math.radians(30 + 60 * k)), c + r * math.sin(math.radians(30 + 60 * k))) for k in range(6)]

    d.polygon(hexagono((AW / 2 - 1) * s), fill=borda)
    d.polygon(hexagono((AW / 2 - 3) * s), fill=fundo)
    d.text((c, c), str(valor), fill=tinta, font=ImageFont.truetype(CARLITO, 16 * s), anchor="mm")
    return im.resize((AW, AW), Image.LANCZOS)


# A fase 3: a volta do laco, e o caco da contradicao.
LW = 16   # a volta: o colisor e metade disto
CW = 24   # o caco da contradicao


def volta():
    """Uma volta do laco: um anel quase fechado, com a ponta de seta - o laco que gira."""
    import math
    s = 4
    im = Image.new("RGBA", (LW * s, LW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    c = LW * s / 2
    r = 5.2 * s
    cor = (96, 210, 255, 255)
    d.ellipse([c - r - 1.5 * s, c - r - 1.5 * s, c + r + 1.5 * s, c + r + 1.5 * s], fill=(20, 50, 80, 200))
    d.arc([c - r, c - r, c + r, c + r], start=60, end=350, fill=cor, width=int(2.4 * s))
    fim = math.radians(350)
    px, py = c + r * math.cos(fim), c + r * math.sin(fim)
    # A ponta de seta, na tangente do fim do arco (sentido horario na tela).
    tx, ty = -math.sin(fim), math.cos(fim)
    nx, ny = math.cos(fim), math.sin(fim)
    d.polygon([(px + tx * 3.2 * s, py + ty * 3.2 * s),
               (px + nx * 2.8 * s - tx * 1.2 * s, py + ny * 2.8 * s - ty * 1.2 * s),
               (px - nx * 2.8 * s - tx * 1.2 * s, py - ny * 2.8 * s - ty * 1.2 * s)], fill=cor)
    d.ellipse([c - 1.6 * s, c - 1.6 * s, c + 1.6 * s, c + 1.6 * s], fill=(220, 240, 255, 255))
    return im.resize((LW, LW), Image.LANCZOS)


def contradicao():
    """O caco do Termina que se contradiz: o simbolo da contradicao, o falso da logica, num chip
    vermelho."""
    s = 4
    im = Image.new("RGBA", (CW * s, CW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1 * s, 1 * s, (CW - 2) * s, (CW - 2) * s], radius=6 * s,
                        fill=(150, 24, 30, 240), outline=(255, 120, 100, 255), width=2 * s)
    c = CW * s / 2
    branco = (255, 236, 228, 255)
    d.line([(c, c - 6.5 * s), (c, c + 5 * s)], fill=branco, width=int(2.4 * s))
    d.line([(c - 6.5 * s, c + 5.5 * s), (c + 6.5 * s, c + 5.5 * s)], fill=branco, width=int(2.4 * s))
    return im.resize((CW, CW), Image.LANCZOS)


# ---- A FINAL: AS EXCECOES ---------------------------------------------------

XW, XH = 168, 24   # a excecao: o nome do tipo num chip; e enfeite, sem colisor
EXCECOES = ["Exception", "IOException", "RuntimeException", "NullPointerException", "Reprovado"]


def excecao(nome):
    """A excecao que sobe pela pilha: o nome do tipo num chip vermelho."""
    s = 4
    im = Image.new("RGBA", (XW * s, XH * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    fonte = ImageFont.truetype(CARLITO, 15 * s)
    largura = d.textlength(nome, font=fonte) / s + 20
    x0 = (XW - largura) / 2
    d.rounded_rectangle([x0 * s, 1 * s, (XW - x0) * s, (XH - 2) * s], radius=6 * s,
                        fill=(150, 24, 30, 240), outline=(255, 120, 100, 255), width=2 * s)
    d.text((XW * s // 2, XH * s // 2), nome, fill=(255, 236, 228, 255), font=fonte, anchor="mm")
    return im.resize((XW, XH), Image.LANCZOS)


PW = 18   # o destroco e o anel do finally: o colisor e metade disto


def destroco():
    """Um pedaco da faixa desempilhada: uma chave quebrada."""
    s = 4
    im = Image.new("RGBA", (PW * s, PW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(2 * s, 4 * s), (14 * s, 1 * s), (16 * s, 13 * s), (5 * s, 16 * s)],
              fill=(52, 60, 86, 245), outline=(150, 170, 220, 255))
    d.text((PW * s // 2, PW * s // 2), "}", fill=(214, 226, 250, 255),
           font=ImageFont.truetype(CARLITO, 13 * s), anchor="mm")
    return im.resize((PW, PW), Image.LANCZOS)


def do_finally():
    """O que o finally solta: ele roda de qualquer jeito."""
    s = 4
    im = Image.new("RGBA", (PW * s, PW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([1 * s, 1 * s, (PW - 2) * s, (PW - 2) * s], fill=(150, 100, 20, 245), outline=(255, 206, 92, 255),
              width=2 * s)
    d.text((PW * s // 2, PW * s // 2), "f", fill=(255, 244, 210, 255),
           font=ImageFont.truetype(CARLITO, 13 * s), anchor="mm")
    return im.resize((PW, PW), Image.LANCZOS)


PILHA = [("Destroco", destroco), ("Finally", do_finally)]


def tira(quadros, w, h, colunas):
    linhas = (len(quadros) + colunas - 1) // colunas
    f = Image.new("RGBA", (w * colunas, h * linhas), (0, 0, 0, 0))
    for i, im in enumerate(quadros):
        f.alpha_composite(im, ((i % colunas) * w, (i // colunas) * h))
    return f


def tira_json(nome, f, w, h, n):
    colunas = f.width // w
    frames = []
    for i in range(n):
        x, y = (i % colunas) * w, (i // colunas) * h
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": w, "h": h},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
            "sourceSize": {"w": w, "h": h},
            "duration": 120,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_luizinho.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": f.width, "h": f.height}, "scale": "1"}}


def grava_tira(quadros, nome, w, h, colunas):
    f = tira(quadros, w, h, colunas)
    destino = os.path.join(RAIZ, "Assets/Teachers/Projectiles")
    f.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as arq:
        json.dump(tira_json(nome, f, w, h, len(quadros)), arq, indent=1)
    print(f"  Assets/Teachers/Projectiles/{nome}.png  {f.width}x{f.height}  ({len(quadros)} quadro(s))")


def animacoes():
    """As animacoes de projeteis.json das folhas, para copiar de la para ca
    quando a folha mudar."""
    return {
        "Valor": {nome: [i] for i, (nome, _) in enumerate(valores_quadros())},
        "Operador": {nome: [i] for i, (nome, _, _) in enumerate(OPERADORES)},
        "Erro": {nome: [i] for i, (nome, _, _) in enumerate(ERROS)},
        "Resposta": {f"X{v}": [i] for i, v in enumerate(VALORES_DE_X)},
        "Volta": {"Volta": [0]},
        "Contradicao": {"Contradicao": [0]},
        "Excecao": {nome: [i] for i, nome in enumerate(EXCECOES)},
        "Destroco": {nome: [i] for i, (nome, _) in enumerate(PILHA)},
    }


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    import sys
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(luizinho(), "DPIBHLuizinho", "Assets/Teachers")
    grava_tira([resposta(v) for v in VALORES_DE_X], "DPIBHLuizinhoRespostas", AW, AW, len(VALORES_DE_X))
    grava_tira([im for _, im in valores_quadros()], "DPIBHLuizinhoValores", VW, VW, 10)
    grava_tira([operador(sim, erro) for _, sim, erro in OPERADORES], "DPIBHLuizinhoOperadores", OW, OW, 4)
    grava_tira([estilhaco(t, c) for _, t, c in ERROS], "DPIBHLuizinhoErros", EW, EH, 2)
    grava_tira([volta()], "DPIBHLuizinhoVoltas", LW, LW, 1)
    grava_tira([contradicao()], "DPIBHLuizinhoContradicao", CW, CW, 1)
    grava_tira([excecao(n) for n in EXCECOES], "DPIBHLuizinhoExcecoes", XW, XH, 1)
    grava_tira([f() for _, f in PILHA], "DPIBHLuizinhoPilha", PW, PW, len(PILHA))
