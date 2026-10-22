#!/usr/bin/env python3
"""Arte PROVISORIA da Maria Lucia (INF 221) e dos projeteis dela.

Mesmo estilo de cartao do Julio, do Thiago, do Vladimir e do Vitor: e
deliberadamente um placeholder, para nao se passar por arte final. Trocar o PNG
basta - o json de quadro continua valendo.

O MOTIVO DO CARTAO E UMA CLASSE EM UML: a caixa de tres reparticoes (nome,
atributos, metodos) com a seta de generalizacao saindo por cima. E para
Engenharia de Software o que o automato de dois estados e para Linguagens
Formais e o fork/join e para Concorrencia - qualquer pessoa que cursou a
materia reconhece de longe, e diz a materia sem escrever o nome dela.

O TRIANGULO E VAZADO DE PROPOSITO. Em UML, ponta cheia e associacao e ponta
vazada e heranca; desenhar cheio seria escrever a palavra errada num cartao
cujo trabalho inteiro e ser lido de longe.

OS PROJETEIS SAO AS DUAS COISAS QUE UM PROJETO ACUMULA, e eles se comportam
diferente (a licao que o Vladimir custou tres versoes a aprender - dois
simbolos que diferem so na cor sao pintura):

    Requisito  o post-it que ATRAVESSA. E pedido chegando: rapido e reto,
               passa e vai embora.
    Debito     o bloco rachado que FICA. E o que voce nao resolveu: para no
               meio do campo e continua la no segundo dezessete.

A diferenca com as Travas do Vitor e o motivo de existirem as duas: a trava e
uma parede que se contorna AGORA e reinicia a cada rajada; o debito nao esta no
seu caminho agora, esta no seu caminho DEPOIS, e nao reinicia.

AS LINHAS SEGUINTES SAO OS PROCESSOS E A IHC (do 119 em diante):

    Codigo      o cartao de terminal da Implementacao, com as linhas acendendo.
    Bug         o inseto dos Testes - as perninhas andam de um quadro ao outro.
    Cursor      a seta do mouse, e o clique: e a IHC, a materia que ela tambem da.
    Observador  o post-it com um olho, do padrao Observer: abre e pisca.
    Historia    o cartao azul de uma historia de usuario, nas sprints.
    Dialogo     uma janelinha com barra de titulo e o X, o modal da IHC.
    Notificacao o selo vermelho com o "1" - o simbolo de interface para "algo
                mudou" -, que e o aviso que cada olho do Observer manda. Nao e
                um post-it de proposito: o aviso amarelo se confundia com o
                proprio olho, que tambem e post-it.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO = (74, 94, 128, 255)
BORDA  = (150, 180, 220, 255)
TINTA  = (228, 238, 250, 255)
TRACO  = (40, 52, 70, 255)
PAPEL  = (226, 232, 243, 255)   # o branco da caixa de classe
RISCO  = (108, 126, 154, 255)   # as linhas que fingem texto dentro dela

# Uma cor por papel, como nos outros chefes: a cor carrega de longe e a forma
# confirma de perto. Aqui a FORMA tambem difere (post-it contra bloco), porque
# a silhueta avisa antes da cor no meio do tiroteio.
COR_REQUISITO = (238, 212, 114, 255)   # amarelo de post-it: pedido novo
COR_DOBRA     = (206, 176,  84, 255)   # a quina dobrada do post-it
COR_DEBITO    = (150,  86,  70, 255)   # ferrugem: codigo apodrecendo
COR_RACHA     = ( 74,  40,  34, 255)
COR_TERMINAL  = ( 28,  36,  44, 255)   # o fundo do terminal
COR_VERDE     = (110, 220, 130, 255)   # o texto do terminal
COR_BUG       = (150, 200,  70, 255)   # o inseto
COR_HISTORIA  = (130, 190, 240, 255)   # o cartao de historia
COR_JANELA    = (220, 226, 236, 255)
COR_TITULO    = ( 70, 110, 190, 255)
COR_FECHAR    = (210,  70,  70, 255)
COR_SELO      = (226,  56,  60, 255)   # o vermelho do selo de notificacao


def quadro_json(nome, w, h, quadros=1, lado=None):
    lado = lado or w
    frames = []
    for i in range(quadros):
        x, y = (i % 4) * lado, (i // 4) * lado
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
        "meta": {"app": "gerar_maria_lucia.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def maria_lucia():
    """O cartao da professora: uma classe em UML, com heranca."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A seta de GENERALIZACAO, saindo por cima: triangulo VAZADO.
    d.polygon([(64, 10), (57, 22), (71, 22)], outline=TINTA, fill=CARTAO)
    d.line([64, 22, 64, 30], fill=TINTA, width=2)

    # A CAIXA DE CLASSE, em tres reparticoes.
    d.rectangle([26, 30, 101, 86], fill=PAPEL, outline=TRACO, width=2)
    d.line([26, 46, 101, 46], fill=TRACO, width=2)   # nome | atributos
    d.line([26, 66, 101, 66], fill=TRACO, width=2)   # atributos | metodos

    # O nome da classe, centralizado e mais grosso - e o que um diagrama faz.
    d.line([46, 38, 81, 38], fill=TRACO, width=3)

    # Atributos e metodos: riscos curtos alinhados a esquerda.
    for y in (53, 59):
        d.line([32, y, 78, y], fill=RISCO, width=2)
    for y in (73, 79):
        d.line([32, y, 70, y], fill=RISCO, width=2)

    d.text((31, 96), "MARIA LUCIA", fill=TINTA)
    return im


def requisito():
    """O REQUISITO: um post-it. Atravessa a tela - pedido chegando."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # Quadrado com a quina de baixo-direita dobrada.
    d.polygon([(4, 4), (27, 4), (27, 19), (19, 27), (4, 27)],
              fill=COR_REQUISITO, outline=TRACO)
    d.polygon([(27, 19), (19, 27), (19, 19)], fill=COR_DOBRA, outline=TRACO)
    # Duas linhas de "texto escrito a mao".
    d.line([8, 11, 22, 11], fill=TRACO, width=1)
    d.line([8, 16, 18, 16], fill=TRACO, width=1)
    return im


def debito():
    """O DEBITO: um bloco rachado. Para e fica - o que voce nao resolveu."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([3, 7, 28, 24], fill=COR_DEBITO, outline=TRACO, width=2)
    # A rachadura, em ziguezague: e ela que diz "isto esta quebrado" em vez de
    # "isto e um tijolo".
    d.line([(15, 7), (18, 12), (13, 16), (17, 24)], fill=COR_RACHA, width=2)
    # Duas juntas de argamassa, para o bloco nao parecer solido demais.
    d.line([5, 13, 12, 13], fill=COR_RACHA, width=1)
    d.line([20, 18, 26, 18], fill=COR_RACHA, width=1)
    return im


def codigo(fase):
    """O CODIGO: um cartao de terminal, as linhas acendendo uma por quadro."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([4, 5, 27, 26], fill=COR_TERMINAL, outline=TRACO, width=2)
    for i, (y, w) in enumerate(((10, 14), (14, 10), (18, 16), (22, 8))):
        cor = COR_VERDE if i <= fase else (60, 110, 70, 255)
        d.line([8, y, 8 + w, y], fill=cor, width=2)
    return im


def bug(fase):
    """O BUG: um besouro visto de cima, com as perninhas alternando."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    a = 2 if fase % 2 == 0 else -2
    for y in (12, 17, 22):
        d.line([9, y, 4, y + a], fill=TRACO, width=2)
        d.line([22, y, 27, y - a], fill=TRACO, width=2)
    d.line([13, 8, 10, 3], fill=TRACO, width=1)       # antenas
    d.line([18, 8, 21, 3], fill=TRACO, width=1)
    d.ellipse([9, 7, 22, 27], fill=COR_BUG, outline=TRACO, width=2)
    d.line([15, 11, 15, 26], fill=TRACO, width=1)     # a divisao das asas
    return im


def cursor(fase):
    """O CURSOR: a seta do mouse. Nos quadros 2 e 3, o clique."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if fase >= 2:
        r = 4 + (fase - 2) * 3
        d.ellipse([6 - r, 6 - r, 6 + r, 6 + r], outline=(255, 255, 255, 200), width=1)
    d.polygon([(6, 4), (6, 24), (11, 19), (15, 27), (18, 26), (14, 18), (21, 18)],
              fill=(250, 250, 250, 255), outline=TRACO)
    return im


def observador(fase):
    """O OBSERVADOR: o post-it com um olho, que abre, olha e pisca."""
    im = requisito().copy()
    d = ImageDraw.Draw(im)
    d.rectangle([6, 8, 24, 18], fill=COR_REQUISITO)   # apaga o texto
    if fase == 3:
        d.line([9, 13, 21, 13], fill=TRACO, width=2)  # piscou
    else:
        d.ellipse([8, 8, 22, 18], fill=(255, 255, 255, 255), outline=TRACO)
        dx = (-2, 0, 2)[fase]
        d.ellipse([13 + dx, 10, 17 + dx, 16], fill=TRACO)
    return im


def historia(fase):
    """A HISTORIA: um cartao azul de historia de usuario, com o titulo."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    m = fase % 2
    d.rectangle([4, 5 + m, 27, 26 - m], fill=COR_HISTORIA, outline=TRACO, width=2)
    d.line([8, 11, 23, 11], fill=TRACO, width=2)
    d.line([8, 16, 20, 16], fill=TRACO, width=1)
    d.line([8, 20, 17, 20], fill=TRACO, width=1)
    return im


def dialogo(fase):
    """O DIALOGO: uma janelinha com a barra de titulo e o X de fechar."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([2, 4, 29, 27], fill=COR_JANELA, outline=TRACO, width=2)
    d.rectangle([3, 5, 28, 10], fill=COR_TITULO)
    d.rectangle([23, 5, 28, 10], fill=COR_FECHAR)
    d.line([24, 6, 27, 9], fill=(255, 255, 255, 255))
    d.line([27, 6, 24, 9], fill=(255, 255, 255, 255))
    # O botao "OK", que pulsa: e o que o dialogo quer que voce clique.
    cor = COR_TITULO if fase % 2 == 0 else (110, 150, 220, 255)
    d.rectangle([11, 19, 20, 24], fill=cor, outline=TRACO)
    d.line([7, 14, 24, 14], fill=RISCO, width=1)
    return im


def notificacao(fase):
    """A NOTIFICACAO: o selo vermelho com o "1", pulsando de leve."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    r = (8, 9, 8, 7)[fase]
    d.ellipse([16 - r, 16 - r, 16 + r, 16 + r], fill=COR_SELO, outline=(255, 255, 255, 255), width=2)
    # O "1": a haste e a bandeirinha.
    d.line([16, 12, 16, 20], fill=(255, 255, 255, 255), width=2)
    d.line([14, 13, 16, 11], fill=(255, 255, 255, 255), width=1)
    return im


def folha():
    """Quatro quadros de cada, uma coisa por linha, em 4x9.

    QUATRO, e nao um, porque o leitor de animacao recebe uma LISTA de quadros
    por animacao. Com um quadro so a animacao existe e nao anima; com quatro da
    para pulsar depois sem mexer em arquivo nenhum.

    As duas primeiras linhas sao as de sempre, e ficam nos mesmos quadros.
    """
    linhas = (lambda i: requisito(), lambda i: debito(), codigo, bug, cursor,
              observador, historia, dialogo, notificacao)
    f = Image.new("RGBA", (128, 32 * len(linhas)), (0, 0, 0, 0))
    for linha, desenho in enumerate(linhas):
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
    grava(maria_lucia(), "DPIBHMariaLucia", "Assets/Teachers")
    grava(folha(), "DPIBHMariaLuciaRequisito", "Assets/Teachers/Projectiles", quadros=36, lado=32)
