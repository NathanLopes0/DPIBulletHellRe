#!/usr/bin/env python3
"""Arte PROVISORIA do Vitor (INF 310) e dos projeteis dele.

Mesmo estilo de cartao do Julio, do Thiago e do Vladimir: e deliberadamente um
placeholder, para nao se passar por arte final. Trocar o PNG basta - o json de
quadro continua valendo.

O MOTIVO DO CARTAO E UM FORK/JOIN: um fluxo que se abre em dois e volta a se
juntar. E para programacao concorrente o que o automato de dois estados e para
linguagens formais - qualquer pessoa que cursou a materia reconhece de longe, e
ele ja anuncia a fase 3, em que o chefe se divide de verdade.

OS PROJETEIS SAO AS DUAS COISAS QUE A MATERIA TRATA, e eles se comportam
diferente (a licao que o Vladimir custou a aprender - dois simbolos que diferem
so na cor sao pintura):

    Tarefa  o quadrado que ATRAVESSA. E trabalho correndo: rapido e reto.
    Trava   o losango que FICA. E um recurso retido: freia e ocupa espaco,
            e o jogador tem de contornar em vez de esperar passar.
"""
from PIL import Image, ImageDraw
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO = (74, 94, 128, 255)
BORDA  = (150, 180, 220, 255)
TINTA  = (228, 238, 250, 255)
TRACO  = (40, 52, 70, 255)

# Uma cor por papel, como no Vladimir: a cor carrega de longe e a forma
# confirma de perto. Aqui a FORMA tambem difere (quadrado contra losango),
# porque as duas se comportam diferente e a silhueta avisa antes da cor.
COR_TAREFA = (126, 206, 160, 255)   # verde: trabalho andando
COR_TRAVA  = (224, 122, 122, 255)   # vermelho: recurso retido


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
        "meta": {"app": "gerar_vitor.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def vitor():
    """O cartao do professor: um fork/join."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # O tronco de entrada.
    d.line([16, 56, 40, 56], fill=TINTA, width=3)

    # A barra de FORK: onde um fluxo vira dois.
    d.rectangle([40, 30, 46, 82], fill=COR_TAREFA, outline=TRACO)

    # Os dois ramos, correndo em paralelo.
    d.line([46, 40, 82, 40], fill=TINTA, width=3)
    d.line([46, 72, 82, 72], fill=TINTA, width=3)

    # A barra de JOIN: onde os dois voltam a ser um.
    d.rectangle([82, 30, 88, 82], fill=COR_TAREFA, outline=TRACO)

    # O tronco de saida.
    d.line([88, 56, 112, 56], fill=TINTA, width=3)
    d.polygon([(106, 50), (114, 56), (106, 62)], fill=TINTA)

    d.text((42, 96), "VITOR", fill=TINTA)
    return im


def tarefa():
    """A TAREFA: um quadrado. Atravessa a tela - trabalho correndo."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([5, 5, 26, 26], fill=COR_TAREFA, outline=TRACO, width=2)
    # Duas listras que lembram uma barra de progresso.
    d.line([9, 12, 22, 12], fill=(255, 255, 255, 120), width=2)
    d.line([9, 18, 18, 18], fill=(255, 255, 255, 90), width=2)
    return im


def trava():
    """A TRAVA: um losango. Freia e fica - recurso retido."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(16, 3), (29, 16), (16, 29), (3, 16)], fill=COR_TRAVA, outline=TRACO)
    # O furo do cadeado, para a silhueta nao ser so "losango".
    d.ellipse([12, 12, 20, 20], fill=TRACO)
    return im


def folha():
    """Quatro quadros de cada, em 4x2 - o mesmo arranjo dos outros chefes.

    QUATRO, e nao um, porque o leitor de animacao recebe uma LISTA de quadros
    por animacao. Com um quadro so a animacao existe e nao anima; com quatro da
    para pulsar depois sem mexer em arquivo nenhum.
    """
    f = Image.new("RGBA", (128, 64), (0, 0, 0, 0))
    for i in range(4):
        f.alpha_composite(tarefa(), (i * 32, 0))
    for i in range(4):
        f.alpha_composite(trava(), (i * 32, 32))
    return f


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    grava(vitor(), "DPIBHVitor", "Assets/Teachers")
    grava(folha(), "DPIBHVitorTarefa", "Assets/Teachers/Projectiles", quadros=8, lado=32)
