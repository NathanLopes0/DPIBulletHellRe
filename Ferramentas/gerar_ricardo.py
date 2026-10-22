#!/usr/bin/env python3
"""Arte PROVISORIA dos projeteis novos do Ricardo (INF 250 - Organizacao de
Computadores; ele tambem da Sistemas Logicos e Arquitetura, e pesquisa
hardware).

O cartao do professor (DPIBHRicardo.png, com "10110" na camiseta) e o Arduino
(DPIBHRicardoProjectile.png) sao a arte original do repositorio e NAO sao
gerados aqui: este script so desenha a folha nova, DPIBHRicardoLogica.png. Mesmo
estilo de placeholder dos outros gerar_*.py - trocar o PNG basta, o json de
quadro continua valendo.

OS PROJETEIS SAO AS PECAS DE UM CIRCUITO:

    Bit     um bit de uma palavra de 16, do verde da camiseta dele. O UM e
            nivel alto: a celula acesa, que pulsa. O ZERO e nivel baixo: so o
            contorno apagado - e o que se atravessa.
    Porta   uma porta logica com a entrada B escrita nela, apontada para BAIXO:
            o dado desce pela tela, entra por cima e sai por baixo. AND, OR e
            XOR com B = 0 e com B = 1, e o NOT, que nao tem B.
    Estagio um estagio do pipeline da fase 3: a moldura com o nome dele em
            cima - IF, ID, EX, MEM, WB.
    Instrucao  um chip com a instrucao de assembly que anda pelo pipeline -
            lw, sw, add, addi, bne, j - e a BOLHA, o estagio vazio.
    Placa   a placa da fase final: o ARDUINO DA ARTE ORIGINAL
            (DPIBHRicardoProjectile.png, lido daqui e ampliado), apagada e com
            o LED do pino 13 aceso.
    Led     o tiro da placa acesa: a luz vermelha do LED.
"""
from PIL import Image, ImageDraw, ImageFont
import json, os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

VERDE   = (90, 230, 120, 255)    # o verde dos digitos da camiseta
BRILHO  = (190, 255, 200, 255)
CELULA  = (20, 34, 40, 235)      # o fundo da celula acesa
APAGADO = (70, 110, 90, 150)     # o zero: so o contorno

TRACO   = (225, 232, 240, 255)   # o desenho da porta
CORPO   = (34, 38, 66, 200)      # o miolo da porta, translucido
FIO     = (150, 160, 175, 255)
B_UM    = (90, 230, 120, 255)
B_ZERO  = (150, 160, 175, 255)


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
            "duration": 160,
        })
    return {
        "frames": frames,
        "meta": {"app": "gerar_ricardo.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def um(d, x0, y0, cor, escala=1):
    """O DIGITO 1 em pixels: a haste, a bandeirinha e o pe."""
    e = escala
    d.rectangle([x0 + 3 * e, y0, x0 + 5 * e - 1, y0 + 10 * e - 1], fill=cor)
    d.rectangle([x0 + 1 * e, y0 + 1 * e, x0 + 3 * e - 1, y0 + 3 * e - 1], fill=cor)
    d.rectangle([x0 + 1 * e, y0 + 9 * e, x0 + 7 * e - 1, y0 + 10 * e - 1], fill=cor)


def zero(d, x0, y0, cor, escala=1):
    """O DIGITO 0 em pixels: um oval de traco grosso."""
    e = escala
    d.rectangle([x0 + 1 * e, y0, x0 + 6 * e - 1, y0 + 1 * e - 1], fill=cor)
    d.rectangle([x0 + 1 * e, y0 + 9 * e, x0 + 6 * e - 1, y0 + 10 * e - 1], fill=cor)
    d.rectangle([x0, y0 + 1 * e, x0 + 2 * e - 1, y0 + 9 * e - 1], fill=cor)
    d.rectangle([x0 + 5 * e, y0 + 1 * e, x0 + 7 * e - 1, y0 + 9 * e - 1], fill=cor)


def bit_um(brilha):
    """O UM: a celula acesa, com borda e digito verdes; no quadro de brilho a
    borda clareia - o bit pulsa."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    borda = BRILHO if brilha else VERDE
    d.rounded_rectangle([2, 2, 29, 29], radius=6, fill=CELULA, outline=borda, width=2)
    um(d, 9, 6, BRILHO if brilha else VERDE, escala=2)
    return im


def bit_zero():
    """O ZERO: nivel baixo. Sem fundo, so o contorno e o digito apagados."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([2, 2, 29, 29], radius=6, outline=APAGADO, width=1)
    zero(d, 9, 6, APAGADO, escala=2)
    return im


def entradas(d, duas, b):
    """Os fios de cima: A sempre (o bit que chega); B, se a porta tiver, com o
    valor escrito ao lado."""
    if duas:
        d.line([11, 0, 11, 7], fill=FIO, width=1)
        d.line([21, 0, 21, 7], fill=FIO, width=1)
        (um if b else zero)(d, 24, 0, B_UM if b else B_ZERO)
    else:
        d.line([16, 0, 16, 7], fill=FIO, width=1)
    d.line([16, 27, 16, 31], fill=FIO, width=1)


def porta_and(b):
    """AND apontada para baixo: topo reto, lados retos, fundo redondo."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    entradas(d, True, b)
    d.rectangle([7, 7, 25, 16], fill=CORPO)
    d.pieslice([7, 7, 25, 27], 0, 180, fill=CORPO)
    d.line([7, 7, 25, 7], fill=TRACO, width=1)
    d.line([7, 7, 7, 16], fill=TRACO, width=1)
    d.line([25, 7, 25, 16], fill=TRACO, width=1)
    d.arc([7, 7, 25, 27], 0, 180, fill=TRACO, width=1)
    return im


def corpo_or(d, y0=7):
    """O escudo do OR apontado para baixo: o topo curvo para dentro e os lados
    que se encontram numa ponta embaixo."""
    topo = [(x, y0 + 4.0 * (1.0 - ((x - 16) / 9.0) ** 2) ** 0.5) for x in range(7, 26)]
    d.polygon(topo + [(24, y0 + 9), (16, 27), (8, y0 + 9)], fill=CORPO)
    d.arc([7, y0 - 4, 25, y0 + 4], 0, 180, fill=TRACO, width=1)
    d.line([7, y0, 8, y0 + 9], fill=TRACO, width=1)
    d.line([25, y0, 24, y0 + 9], fill=TRACO, width=1)
    d.line([8, y0 + 9, 16, 27], fill=TRACO, width=1)
    d.line([24, y0 + 9, 16, 27], fill=TRACO, width=1)


def porta_or(b):
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    entradas(d, True, b)
    corpo_or(d)
    return im


def porta_xor(b):
    """O XOR e o OR com o segundo arco por cima."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    entradas(d, True, b)
    corpo_or(d, y0=9)
    d.arc([7, 2, 25, 8], 0, 180, fill=TRACO, width=1)
    return im


def porta_not():
    """NOT: o triangulo apontado para baixo e a bolinha da negacao na ponta."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    entradas(d, False, 0)
    d.polygon([(7, 7), (25, 7), (16, 22)], fill=CORPO, outline=TRACO)
    d.ellipse([13, 22, 19, 27], outline=TRACO, width=1)
    return im


ESTAGIO = (120, 140, 170, 255)   # a moldura do estagio
ROTULO  = (200, 215, 235, 255)
CHIP    = (24, 30, 48, 255)
BORDA_CHIP = (90, 230, 120, 255)  # o verde da camiseta
PINO    = (120, 140, 160, 255)
BOLHA   = (150, 190, 230, 140)


def estagio(nome):
    """O ESTAGIO: a moldura tracejada, com o nome em cima."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for x in range(1, 31, 3):
        d.point((x, 6), fill=ESTAGIO)
        d.point((x, 30), fill=ESTAGIO)
    for y in range(6, 31, 3):
        d.point((1, y), fill=ESTAGIO)
        d.point((30, y), fill=ESTAGIO)
    fonte = ImageFont.load_default(size=8)
    d.text((16, 3), nome, fill=ROTULO, font=fonte, anchor="mm")
    return im


def instrucao(texto):
    """A INSTRUCAO: um chip com a instrucao escrita, e as perninhas."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for x in (9, 15, 21):
        d.line([x, 8, x, 10], fill=PINO, width=1)
        d.line([x, 22, x, 24], fill=PINO, width=1)
    d.rounded_rectangle([3, 10, 28, 22], radius=3, fill=CHIP, outline=BORDA_CHIP, width=1)
    fonte = ImageFont.load_default(size=9)
    d.text((16, 16), texto, fill=BORDA_CHIP, font=fonte, anchor="mm")
    return im


def bolha():
    """A BOLHA: o estagio vazio - uma bolha translucida escrita nop."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([6, 7, 25, 26], outline=BOLHA, width=1)
    d.point((11, 12), fill=BOLHA)
    d.point((12, 11), fill=BOLHA)
    fonte = ImageFont.load_default(size=8)
    d.text((16, 17), "nop", fill=BOLHA, font=fonte, anchor="mm")
    return im


LED_ACESO = (255, 70, 60, 255)
LED_BRILHO = (255, 200, 170, 255)
LED_HALO = (255, 90, 70, 110)


def placa(acesa):
    """A PLACA: o Arduino original, ampliado de 16 para 32 px; acesa, o LED do
    pino 13 - no canto de cima, perto dos pinos digitais - brilha."""
    original = Image.open(os.path.join(RAIZ, "Assets", "Teachers", "Projectiles",
                                       "DPIBHRicardoProjectile.png")).convert("RGBA")
    im = original.resize((32, 32), Image.NEAREST)
    d = ImageDraw.Draw(im)
    if acesa:
        d.ellipse([20, 0, 31, 11], fill=LED_HALO)
        d.rectangle([24, 4, 27, 7], fill=LED_ACESO)
        d.rectangle([25, 4, 26, 5], fill=LED_BRILHO)
    else:
        d.rectangle([24, 4, 27, 7], fill=(90, 30, 30, 255))
    return im


def led():
    """O LED: a luz vermelha, com o halo."""
    im = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([7, 7, 24, 24], fill=LED_HALO)
    d.ellipse([11, 11, 20, 20], fill=LED_ACESO)
    d.ellipse([13, 12, 16, 15], fill=LED_BRILHO)
    return im


def folha():
    """Quatro quadros por linha, de 32 px. As linhas novas vao sempre no FIM:
    os quadros de antes ficam onde estavam.

        0 Zero   1 Um   2 Um (brilho)   3 AND B=0
        4 AND B=1   5 OR B=0   6 OR B=1   7 XOR B=0
        8 XOR B=1   9 NOT   10 IF   11 ID
        12 EX   13 MEM   14 WB   15 lw
        16 sw   17 add   18 addi   19 bne
        20 j   21 bolha   22 Placa apagada   23 Placa acesa
        24 Led
    """
    desenhos = [bit_zero(), bit_um(False), bit_um(True),
                porta_and(0), porta_and(1), porta_or(0), porta_or(1),
                porta_xor(0), porta_xor(1), porta_not()]
    desenhos += [estagio(n) for n in ("IF", "ID", "EX", "MEM", "WB")]
    desenhos += [instrucao(n) for n in ("lw", "sw", "add", "addi", "bne", "j")]
    desenhos += [bolha()]
    desenhos += [placa(False), placa(True), led()]
    linhas = (len(desenhos) + 3) // 4
    f = Image.new("RGBA", (128, 32 * linhas), (0, 0, 0, 0))
    for i, im in enumerate(desenhos):
        f.alpha_composite(im, ((i % 4) * 32, (i // 4) * 32))
    return f, len(desenhos)


def grava(im, nome, pasta, quadros=1, lado=None):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height, quadros, lado), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    im, n = folha()
    grava(im, "DPIBHRicardoLogica", "Assets/Teachers/Projectiles", quadros=n, lado=32)
