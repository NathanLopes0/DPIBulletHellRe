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

    Gota      X, Y      a variavel do escopo da fase 1: o nome dela num chip de
                        editor, x em ambar e y em ciano (DPIBHLuizinhoGotas).
              AvisoX,   o mesmo chip apagado: onde a chuva vai cair.
              AvisoY
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
    Celula    Aviso     um int de um bloco do heap da fase 3: o contorno de
              Viva      onde ele vai nascer, o 0 do Java, o "?" do C++ (que
              Crua      nao inicia), o campo prox de um no, o lixo, o lixo
              Prox      marcado pelo coletor e o que vazou
              Lixo      (DPIBHLuizinhoMemoria).
              Marcada
              Vazou
    Estilhaco Corrompido o caco do ponteiro pendente e do bad_alloc
                        (DPIBHLuizinhoEstilhacos).
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


# ---- A FASE 1: O ESCOPO ----------------------------------------------------

# As cores das duas variaveis do programa (Escopo.h): x ambar, y ciano.
CORES_DAS_VARIAVEIS = {"X": (255, 196, 72), "Y": (96, 204, 240)}
GW = 28   # o quadro da gota: o colisor e metade disto (Escopo::kRaioDaGota)


def gota(nome, acesa):
    """UMA GOTA DA VARIAVEL: o nome dela num chip de editor. Apagada (o aviso
    de onde vai chover), so o contorno e a letra fraca; acesa, cheia."""
    s = 4
    r, g, b = CORES_DAS_VARIAVEIS[nome]
    im = Image.new("RGBA", (GW * s, GW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if acesa:
        d.rounded_rectangle([2 * s, 2 * s, (GW - 3) * s, (GW - 3) * s], radius=6 * s,
                            fill=(34, 30, 24, 235), outline=(r, g, b, 255), width=2 * s)
        tinta = (r, g, b, 255)
    else:
        d.rounded_rectangle([3 * s, 3 * s, (GW - 4) * s, (GW - 4) * s], radius=6 * s,
                            fill=(r, g, b, 34), outline=(r, g, b, 120), width=1 * s)
        tinta = (r, g, b, 120)
    d.text((GW * s // 2, GW * s // 2 - 2 * s), nome.lower(), fill=tinta,
           font=ImageFont.truetype(CARLITO, 22 * s), anchor="mm")
    return im.resize((GW, GW), Image.LANCZOS)


# A ordem dos quadros, a das animacoes de projeteis.json.
GOTAS = [("AvisoX", "X", False), ("X", "X", True), ("AvisoY", "Y", False), ("Y", "Y", True)]


def gotas():
    f = Image.new("RGBA", (GW * len(GOTAS), GW), (0, 0, 0, 0))
    for i, (_, nome, acesa) in enumerate(GOTAS):
        f.alpha_composite(gota(nome, acesa), (i * GW, 0))
    return f


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


# ---- A FASE 3: A GERENCIA DE MEMORIA ----------------------------------------

CW = 30   # a celula do heap: o colisor e metade disto (Memoria::kLadoDaCelula)


def celula(texto, fundo, borda, tinta, x_vermelho=False, apagada=False):
    """Uma celula do heap: um int (ou o campo prox de um no) num chip."""
    s = 4
    im = Image.new("RGBA", (CW * s, CW * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([2 * s, 2 * s, (CW - 3) * s, (CW - 3) * s], fill=fundo, outline=borda,
                width=(1 if apagada else 2) * s)
    if texto:
        d.text((CW * s // 2, CW * s // 2), texto, fill=tinta,
               font=ImageFont.truetype(CARLITO, 18 * s), anchor="mm")
    if x_vermelho:
        d.line([(6 * s, 6 * s), ((CW - 7) * s, (CW - 7) * s)], fill=(220, 40, 40, 255), width=3 * s)
        d.line([((CW - 7) * s, 6 * s), (6 * s, (CW - 7) * s)], fill=(220, 40, 40, 255), width=3 * s)
    return im.resize((CW, CW), Image.LANCZOS)


# A ordem dos quadros, a das animacoes de projeteis.json.
CELULAS = [
    ("Aviso",   lambda: celula("", (200, 210, 230, 60), (225, 232, 255, 210), None, apagada=True)),
    ("Viva",    lambda: celula("0", (40, 70, 110, 240), (120, 170, 230, 255), (220, 232, 255, 255))),
    ("Crua",    lambda: celula("?", (40, 70, 110, 240), (120, 170, 230, 255), (255, 214, 150, 255))),
    ("Prox",    lambda: celula("\u2192", (40, 70, 110, 240), (120, 170, 230, 255), (150, 230, 255, 255))),
    ("Lixo",    lambda: celula("", (90, 90, 96, 220), (150, 150, 158, 230), None)),
    ("Marcada", lambda: celula("", (90, 90, 96, 220), (150, 150, 158, 230), None, x_vermelho=True)),
    ("Vazou",   lambda: celula("?", (110, 62, 26, 240), (236, 150, 64, 255), (255, 214, 160, 255))),
]


def estilhaco_de_memoria():
    """O que estoura do ponteiro pendente e do bad_alloc: um caco de memoria
    corrompida."""
    s = 4
    im = Image.new("RGBA", (16 * s, 16 * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(8 * s, 1 * s), (15 * s, 8 * s), (8 * s, 15 * s), (1 * s, 8 * s)],
              fill=(170, 30, 90, 240), outline=(255, 120, 180, 255))
    d.text((8 * s, 8 * s), "?", fill=(255, 230, 240, 255),
           font=ImageFont.truetype(CARLITO, 11 * s), anchor="mm")
    return im.resize((16, 16), Image.LANCZOS)


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
    """As animacoes de projeteis.json das folhas da fase 2, para copiar de la
    para ca quando a folha mudar."""
    return {
        "Valor": {nome: [i] for i, (nome, _) in enumerate(valores_quadros())},
        "Operador": {nome: [i] for i, (nome, _, _) in enumerate(OPERADORES)},
        "Erro": {nome: [i] for i, (nome, _, _) in enumerate(ERROS)},
        "Celula": {nome: [i] for i, (nome, _) in enumerate(CELULAS)},
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
    grava(gotas(), "DPIBHLuizinhoGotas", "Assets/Teachers/Projectiles", quadros=len(GOTAS), lado=GW)
    grava_tira([im for _, im in valores_quadros()], "DPIBHLuizinhoValores", VW, VW, 10)
    grava_tira([operador(sim, erro) for _, sim, erro in OPERADORES], "DPIBHLuizinhoOperadores", OW, OW, 4)
    grava_tira([estilhaco(t, c) for _, t, c in ERROS], "DPIBHLuizinhoErros", EW, EH, 2)
    grava_tira([f() for _, f in CELULAS], "DPIBHLuizinhoMemoria", CW, CW, len(CELULAS))
    grava_tira([estilhaco_de_memoria()], "DPIBHLuizinhoEstilhacos", 16, 16, 1)
    grava_tira([excecao(n) for n in EXCECOES], "DPIBHLuizinhoExcecoes", XW, XH, 1)
    grava_tira([f() for _, f in PILHA], "DPIBHLuizinhoPilha", PW, PW, len(PILHA))
