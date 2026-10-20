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


def folha():
    """Quatro quadros de cada, em 4x2 - o mesmo arranjo dos outros chefes.

    QUATRO, e nao um, porque o leitor de animacao recebe uma LISTA de quadros
    por animacao. Com um quadro so a animacao existe e nao anima; com quatro da
    para pulsar depois sem mexer em arquivo nenhum.
    """
    f = Image.new("RGBA", (128, 64), (0, 0, 0, 0))
    for i in range(4):
        f.alpha_composite(requisito(), (i * 32, 0))
    for i in range(4):
        f.alpha_composite(debito(), (i * 32, 32))
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
    grava(folha(), "DPIBHMariaLuciaRequisito", "Assets/Teachers/Projectiles", quadros=8, lado=32)
