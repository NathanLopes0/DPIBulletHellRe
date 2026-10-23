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
    Serpente  Cabeca    a cabeca da serpente, que segue o aluno.
              Corpo     os gomos, que seguem a cabeca.

A Serpente mora em DPIBHLuizinhoLetras, nas duas ultimas linhas; as outras
seis sao as letras da etimologia e os tijolos do coletor antigo, que nenhuma
fase usa mais.
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
CERA   = (168, 128, 62, 255)
MOLDURA = (104, 72, 40, 255)
OURO   = (246, 206, 92, 255)
COURO  = (128, 76, 44, 255)
BRONZE = (196, 140, 72, 255)
TIJOLO = (178, 92, 62, 255)
MARCA  = (120, 18, 24, 255)
ESCAMA = (82, 150, 92, 255)
VENTRE = (172, 210, 120, 255)


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


def ladrilho(d, m, fundo, borda):
    d.rounded_rectangle([m, m, 31 - m, 31 - m], radius=4, fill=fundo, outline=borda, width=2)


def estatica(fase):
    """A LETRA ESTATICA: pedra com um "C" talhado. Brilha de leve."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    ladrilho(d, 3 + (fase % 2), PEDRA, TRACO)
    d.arc([9, 8, 23, 24], start=40, end=320, fill=SULCO, width=3)
    return im


def livre(fase):
    """A AMARRACAO QUE ESPERA: tabuinha de cera em branco, com o estilete
    piscando no meio, como um cursor."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    ladrilho(d, 3, MOLDURA, TRACO)
    d.rectangle([8, 8, 23, 23], fill=CERA)
    if fase % 2 == 0:
        d.line([(16, 11), (16, 20)], fill=TRACO, width=2)
    return im


def ligada(fase):
    """A AMARRACAO FEITA: a mesma tabuinha, agora com um "V" em ouro."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    ladrilho(d, 3, MOLDURA, TRACO)
    d.rectangle([8, 8, 23, 23], fill=CERA)
    cor = OURO if fase % 2 == 0 else (255, 236, 150, 255)
    d.line([(10, 10), (16, 22), (22, 10)], fill=cor, width=3)
    return im


def capsula(fase):
    """A CAPSA: o estojo cilindrico dos rolos, com a tampa e duas cintas."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    a = fase % 2
    d.rounded_rectangle([8, 4 + a, 23, 28 - a], radius=6, fill=COURO, outline=TRACO, width=2)
    d.rectangle([7, 7 + a, 24, 11 + a], fill=BRONZE, outline=TRACO)   # a tampa
    d.line([(9, 18), (22, 18)], fill=BRONZE, width=2)
    d.line([(9, 23), (22, 23)], fill=BRONZE, width=2)
    return im


def numero_romano(d, x, y, cor):
    """Um "XII" desenhado a traco, pequeno."""
    d.line([(x, y), (x + 5, y + 7)], fill=cor, width=2)
    d.line([(x + 5, y), (x, y + 7)], fill=cor, width=2)
    d.line([(x + 8, y), (x + 8, y + 7)], fill=cor, width=2)
    d.line([(x + 11, y), (x + 11, y + 7)], fill=cor, width=2)


def bloco(fase, marcado=False):
    """O BLOCO DE MEMORIA: um tijolo com junta e um numero romano, o endereco.
    Marcado, ganha um X escuro por cima - a marca do coletor."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = 4 + (fase % 2 if marcado else 0)
    d.rectangle([3, 7 + m // 2, 28, 24 - m // 2], fill=TIJOLO, outline=TRACO, width=2)
    numero_romano(d, 9, 12, TINTA)
    if marcado:
        d.line([(5, 7), (26, 24)], fill=MARCA, width=3)
        d.line([(26, 7), (5, 24)], fill=MARCA, width=3)
    return im


def cabeca(fase):
    """A CABECA DA SERPENTE, de cima: olhos e a lingua que entra e sai."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([5, 5, 26, 26], fill=ESCAMA, outline=TRACO, width=2)
    d.ellipse([10, 11, 14, 15], fill=TRACO)
    d.ellipse([17, 11, 21, 15], fill=TRACO)
    if fase % 2 == 0:
        d.line([(15, 26), (15, 30)], fill=MARCA, width=2)
    return im


def corpo(fase):
    """UM GOMO DO CORPO: escama verde com o ventre claro no meio."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = 7 + (fase % 2)
    d.ellipse([m, m, 31 - m, 31 - m], fill=ESCAMA, outline=TRACO, width=2)
    d.ellipse([13, 13, 18, 18], fill=VENTRE)
    return im


def folha():
    """Quatro quadros de cada, uma animacao por linha, numa folha 4x8."""
    linhas = (estatica, livre, ligada, capsula,
              bloco, lambda i: bloco(i, marcado=True), cabeca, corpo)
    f = Image.new("RGBA", (128, 32 * len(linhas)), (0, 0, 0, 0))
    for linha, desenho in enumerate(linhas):
        for i in range(4):
            f.alpha_composite(desenho(i), (i * 32, linha * 32))
    return f


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
    grava(folha(), "DPIBHLuizinhoLetras", "Assets/Teachers/Projectiles", quadros=32, lado=32)
    grava(gotas(), "DPIBHLuizinhoGotas", "Assets/Teachers/Projectiles", quadros=len(GOTAS), lado=GW)
    grava_tira([im for _, im in valores_quadros()], "DPIBHLuizinhoValores", VW, VW, 10)
    grava_tira([operador(sim, erro) for _, sim, erro in OPERADORES], "DPIBHLuizinhoOperadores", OW, OW, 4)
    grava_tira([estilhaco(t, c) for _, t, c in ERROS], "DPIBHLuizinhoErros", EW, EH, 2)
    grava_tira([f() for _, f in CELULAS], "DPIBHLuizinhoMemoria", CW, CW, len(CELULAS))
    grava_tira([estilhaco_de_memoria()], "DPIBHLuizinhoEstilhacos", 16, 16, 1)
