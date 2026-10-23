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

    Tarefa    o quadrado que ATRAVESSA. E trabalho correndo: rapido e reto.
    Trava     o losango que FICA. E um recurso retido: freia e ocupa espaco,
              e o jogador tem de contornar em vez de esperar passar.
    Corrente  os elos que PERSEGUEM. Sao a piada de "concorrentes": quem leva
              uma quase nao perde nota e fica PRESO - andando a menos da
              metade da velocidade por alguns segundos.

AS INSTRUCOES DA FASE 1 (DPIBHVitorThreads), uma cor por thread:

    Instrucao T1..T5  o chip de uma instrucao, na cor da thread dela e com o
              numero dela. A fase 1 e a sincronizacao: cada thread e uma
              coluna de instrucoes descendo ate a secao critica, e quem le o
              placar ("fila: T3 T1 T5") precisa achar a coluna pela cor.

AS LINHAS SEGUINTES SAO AS REDES, a outra materia que ele da (do 127 em diante):

    Nuvem     a nuvem da computacao em nuvem, em cima dele na fase 2: ociosa
              ela so balanca; PROCESSANDO, as luzinhas dela acendem em fila.
    Pacote    o pacote de rede que a nuvem manda: o cabecalho colorido e o
              corpo, piscando.
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")

CARTAO = (74, 94, 128, 255)
BORDA  = (150, 180, 220, 255)
TINTA  = (228, 238, 250, 255)
TRACO  = (40, 52, 70, 255)

# Uma cor por papel, como no Vladimir: a cor carrega de longe e a forma
# confirma de perto. Aqui a FORMA tambem difere (quadrado contra losango),
# porque as duas se comportam diferente e a silhueta avisa antes da cor.
COR_TAREFA = (126, 206, 160, 255)   # verde: trabalho andando
COR_TRAVA  = (224, 122, 122, 255)   # vermelho: recurso retido

# A corrente e FERRO, e de proposito nao e uma terceira cor viva: ela nao
# avisa "perigo de nota", avisa "peso". Cinza claro com um toque de azul, que e
# a mesma familia da cor que o aluno ganha enquanto esta preso.
COR_ELO    = (186, 192, 208, 255)
COR_ELO_ESCURO = (120, 128, 148, 255)

# AS CINCO THREADS DA FASE 1. Cinco matizes bem separados - laranja, ciano,
# rosa, lima e violeta -, e nenhum deles e o vermelho de "perigo" nem o cinza
# da corrente. O NUMERO no chip confirma de perto o que a cor diz de longe.
# O Vitor pinta o nome de cada coluna com estas mesmas cores (Vitor.cpp).
CORES_DAS_THREADS = [
    (240, 160, 60, 255),
    (90, 200, 235, 255),
    (235, 110, 190, 255),
    (160, 220, 90, 255),
    (170, 140, 245, 255),
]

COR_NUVEM  = (214, 226, 242, 255)
COR_NUVEM_SOMBRA = (150, 166, 192, 255)
COR_LUZ    = (120, 230, 255, 255)   # as luzinhas do processamento
COR_CABECALHO = (250, 190, 70, 255)
COR_CORPO  = (90, 140, 220, 255)


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


def corrente():
    """A CORRENTE: dois elos entrelacados. Persegue, prende e quase nao machuca.

    DOIS ELOS, e nao um: um elo sozinho e um anel, e anel ja e a silhueta da
    Trava. Sao os dois entrelacados que se leem como corrente de longe, que e a
    distancia em que o jogador precisa reconhecer o que vem vindo.
    """
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    # elo de tras, mais escuro, deslocado para baixo e para a direita
    d.ellipse([11, 12, 27, 28], outline=COR_ELO_ESCURO, width=4)
    # elo da frente
    d.ellipse([4, 3, 20, 19], outline=COR_ELO, width=4)
    d.ellipse([4, 3, 20, 19], outline=TRACO, width=1)
    return im


def nuvem(fase, processando):
    """A NUVEM: tres bolhas e uma base. Processando, as luzinhas acendem."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    dy = (0, 1, 0, -1)[fase] if not processando else 0
    for caixa in ([3, 12, 15, 24], [9, 6, 23, 20], [17, 10, 29, 22]):
        d.ellipse([caixa[0], caixa[1] + dy, caixa[2], caixa[3] + dy], fill=COR_NUVEM, outline=TRACO)
    d.rectangle([7, 16 + dy, 25, 23 + dy], fill=COR_NUVEM)
    d.line([6, 24 + dy, 26, 24 + dy], fill=COR_NUVEM_SOMBRA, width=2)
    if processando:
        for k in range(4):
            cor = COR_LUZ if k <= fase else COR_NUVEM_SOMBRA
            d.rectangle([9 + 4 * k, 18, 11 + 4 * k, 20], fill=cor)
    return im


def pacote(fase):
    """O PACOTE: um cabecalho e um corpo, piscando."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([8, 9, 23, 23], fill=COR_CORPO, outline=TRACO, width=2)
    d.rectangle([9, 10, 22, 13], fill=COR_CABECALHO if fase % 2 == 0 else (255, 230, 150, 255))
    d.line([11, 17, 20, 17], fill=(255, 255, 255, 140), width=1)
    d.line([11, 20, 17, 20], fill=(255, 255, 255, 110), width=1)
    return im


def instrucao(t, fase):
    """A INSTRUCAO da thread t: um chip na cor dela, com o numero. Pisca o
    cantinho de cima, como o cursor de quem esta executando."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cor = CORES_DAS_THREADS[t]
    d.rounded_rectangle([5, 5, 26, 26], radius=5, fill=cor, outline=TRACO, width=2)
    fonte = ImageFont.truetype(CARLITO, 17)
    texto = str(t + 1)
    caixa = d.textbbox((0, 0), texto, font=fonte)
    w, h = caixa[2] - caixa[0], caixa[3] - caixa[1]
    d.text((16 - w / 2 - caixa[0], 16 - h / 2 - caixa[1]), texto, fill=TRACO, font=fonte,
           stroke_width=1, stroke_fill=TRACO)
    if fase % 2 == 0:
        d.rectangle([20, 8, 23, 10], fill=(255, 255, 255, 200))
    return im


def folha_das_threads():
    """Quatro quadros por thread, uma thread por linha."""
    f = Image.new("RGBA", (128, 160), (0, 0, 0, 0))
    for t in range(5):
        for i in range(4):
            f.alpha_composite(instrucao(t, i), (i * 32, t * 32))
    return f


def folha():
    """Quatro quadros de cada, uma coisa por linha - o mesmo arranjo dos
    outros chefes.

    QUATRO, e nao um, porque o leitor de animacao recebe uma LISTA de quadros
    por animacao. Com um quadro so a animacao existe e nao anima; com quatro da
    para pulsar depois sem mexer em arquivo nenhum.

    As linhas novas vao sempre no FIM: os quadros de antes ficam onde estavam.
    """
    f = Image.new("RGBA", (128, 192), (0, 0, 0, 0))
    for i in range(4):
        f.alpha_composite(tarefa(), (i * 32, 0))
    for i in range(4):
        f.alpha_composite(trava(), (i * 32, 32))
    for i in range(4):
        f.alpha_composite(corrente(), (i * 32, 64))
    for i in range(4):
        f.alpha_composite(nuvem(i, False), (i * 32, 96))
    for i in range(4):
        f.alpha_composite(nuvem(i, True), (i * 32, 128))
    for i in range(4):
        f.alpha_composite(pacote(i), (i * 32, 160))
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
    grava(folha(), "DPIBHVitorTarefa", "Assets/Teachers/Projectiles", quadros=24, lado=32)
    grava(folha_das_threads(), "DPIBHVitorThreads", "Assets/Teachers/Projectiles", quadros=20, lado=32)
