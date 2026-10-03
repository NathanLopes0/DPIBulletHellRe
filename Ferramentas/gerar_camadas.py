#!/usr/bin/env python3
"""
Gera a arte de placeholder em CAMADAS para a criacao de personagem.

Cada peca sai como um atlas de 128x128 com quatro quadros de 64x64, na mesma
disposicao do DPIBHPlayer.png que o jogo ja le:

    quadro 0 = (0,0)    quadro 1 = (64,0)
    quadro 2 = (0,64)   quadro 3 = (64,64)

Quase tudo sai em LUMINANCIA (cinza + alfa), porque a cor e escolhida pelo aluno
e aplicada por multiplicacao na hora de compor: cinza 255 devolve a cor escolhida
tal e qual, e os tons mais escuros viram a sombra dela. Vale para o cabelo, para
a roupa e TAMBEM para o corpo, cujo tom de pele e uma escolha como as outras.

Duas pecas fogem disso e saem em cor final, justamente por nao acompanharem o
tom de pele: "rosto" (os olhos) e "sapato". Se saissem junto com o corpo, mudar
o tom de pele clarearia os olhos e os sapatos junto.

A ordem de composicao e: corpo, rosto, sapato, calca, camisa, cabelo.

A animacao segue a do sprite original: quadros 1 e 3 sao a pose neutra, 0 e 2
sao os passos, e a CABECA NAO SE MEXE em nenhum deles - por isso o cabelo e uma
imagem so, repetida nos quatro quadros.
"""

from PIL import Image
import os

L = 64  # lado do quadro

# ---------------------------------------------------------------- cores fixas
OLHO       = ( 48,  36,  34, 255)
ESCLERA    = (242, 238, 232, 255)
SAPATO     = ( 74,  70,  92, 255)
SAPATO_S   = ( 52,  49,  66, 255)
CONTORNO   = ( 38,  30,  30, 255)

# tons de luminancia das pecas tingiveis
BASE   = (255, 255, 255, 255)
SOMBRA = (185, 185, 185, 255)
BORDA  = (105, 105, 105, 255)


class ForaDoQuadro(Exception):
    pass


def ret(d, x0, y0, x1, y1, cor):
    """Retangulo cheio, inclusivo nas duas pontas.

    RECLAMA quando o desenho sai do quadro, em vez de cortar calado. Cortando,
    desenhei espetos de nove pixels acima de uma cabeca que so tinha cinco de
    folga e o cabelo saiu reto - e a imagem nao dizia que faltava nada.
    """
    if x0 < 0 or y0 < 0 or x1 >= L or y1 >= L:
        raise ForaDoQuadro('retangulo (%d,%d)-(%d,%d) sai do quadro de %d'
                           % (x0, y0, x1, y1, L))
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            d[(x, y)] = cor


def contornar(d, cor=CONTORNO):
    """Poe contorno em volta do que ja foi desenhado.

    Um pixel vazio que encosta (em cruz) num pixel cheio vira contorno. E o que
    da a leitura de pixel art: sem isso as pecas se misturam uma na outra quando
    sao da mesma cor.
    """
    novo = {}
    for (x, y) in d:
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            p = (x + dx, y + dy)
            if p not in d and 0 <= p[0] < L and 0 <= p[1] < L:
                novo[p] = cor
    d.update(novo)
    return d


# ------------------------------------------------------------------ geometria
#
# O orcamento vertical do personagem, em pixels do quadro. Veio das proporcoes
# do DPIBHPlayer.png original: cabeca enorme, corpo curto.
CABECA_Y0, CABECA_Y1 = 5, 32
TORSO_Y0,  TORSO_Y1  = 36, 48
PERNA_Y0,  PERNA_Y1  = 49, 57
PE_Y0,     PE_Y1     = 58, 60

CX = 32                      # centro do personagem
CABECA_X0, CABECA_X1 = 20, 43
TORSO_X0,  TORSO_X1  = 24, 39

# O vao entre as pernas e de quatro pixels, e nao de dois: com dois, o contorno
# de uma perna encostava no da outra e o corpo inteiro lia como um bloco so -
# ficava impossivel ver onde a camisa acabava e a calca comecava.
PERNA_E = (25, 29)           # perna esquerda, em x
PERNA_D = (34, 38)
BRACO_E = (20, 23)
BRACO_D = (40, 43)


def passo(quadro):
    """Quanto cada perna sobe neste quadro, e quanto cada braco desce.

    Quadros 1 e 3 sao a pose parada. Com nove pixels de perna nao cabe passada
    de verdade: levantar um pe dois pixels e o que le como andar neste tamanho.
    """
    if quadro == 0:
        return {'pe_e': -2, 'pe_d': 0, 'braco_e': 1, 'braco_d': -1}
    if quadro == 2:
        return {'pe_e': 0, 'pe_d': -2, 'braco_e': -1, 'braco_d': 1}
    return {'pe_e': 0, 'pe_d': 0, 'braco_e': 0, 'braco_d': 0}


# ---------------------------------------------------------------------- corpo
#
# O CORPO TAMBEM SAI EM LUMINANCIA, porque o tom de pele e escolhido como
# qualquer outra cor. Por isso os olhos saem numa peca separada: tingidos junto
# com a pele, eles clareariam ou escureceriam com ela e deixariam de ser olhos.
# Os sapatos saem separados pelo mesmo motivo.
def corpo(quadro):
    d = {}
    p = passo(quadro)

    # cabeca: um bloco com os cantos comidos, para arredondar
    ret(d, CABECA_X0, CABECA_Y0, CABECA_X1, CABECA_Y1, BASE)
    for canto_x, canto_y in ((CABECA_X0, CABECA_Y0), (CABECA_X1, CABECA_Y0),
                             (CABECA_X0, CABECA_Y1), (CABECA_X1, CABECA_Y1)):
        sx = 1 if canto_x == CABECA_X0 else -1
        sy = 1 if canto_y == CABECA_Y0 else -1
        for i in range(3):
            for j in range(3 - i):
                d.pop((canto_x + sx * i, canto_y + sy * j), None)

    # orelhas
    ret(d, CABECA_X0 - 2, 17, CABECA_X0 - 1, 21, BASE)
    ret(d, CABECA_X1 + 1, 17, CABECA_X1 + 2, 21, BASE)

    # sombra do queixo e a boca - ambas sao a PROPRIA pele mais escura, entao
    # acompanham o tom escolhido em vez de serem uma cor fixa por cima dele.
    ret(d, CABECA_X0 + 2, CABECA_Y1 - 1, CABECA_X1 - 2, CABECA_Y1, SOMBRA)
    ret(d, 30, 28, 33, 28, SOMBRA)

    # pescoco e torso
    ret(d, 28, 33, 35, 35, SOMBRA)
    ret(d, TORSO_X0, TORSO_Y0, TORSO_X1, TORSO_Y1, BASE)

    # bracos, cada um com o seu deslocamento
    for (x0, x1), dy in ((BRACO_E, p['braco_e']), (BRACO_D, p['braco_d'])):
        ret(d, x0, TORSO_Y0 + 1 + dy, x1, TORSO_Y1 + dy, BASE)

    # pernas e pes descalcos: o sapato e uma camada por cima, para que um dia
    # "descalco" seja uma opcao sem redesenhar o corpo.
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0, PERNA_Y0, x1, PE_Y1 + dy, BASE)

    return contornar(d, BORDA)


def rosto(quadro):
    """Os olhos. Cor fixa, porque olho nao muda com o tom de pele.

    COM ESCLERA CLARA, e nao so a pupila escura: a pupila sozinha sumia nos
    tons de pele mais escuros, porque o contorno da pele escura fica tao escuro
    quanto ela. O branco ao lado garante o olho em qualquer tom.

    A cabeca nao se mexe em quadro nenhum, entao esta peca e igual nos quatro.
    """
    d = {}
    for x0, pupila in ((24, 25), (36, 38)):
        ret(d, x0, 22, x0 + 2, 24, ESCLERA)
        ret(d, pupila, 22, pupila + 1, 23, OLHO)
    return d


def sapato(quadro):
    """Os sapatos. Cor fixa, e acompanham o pe que levanta."""
    d = {}
    p = passo(quadro)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PE_Y0 + dy, x1 + 1, PE_Y1 + dy, SAPATO)
        ret(d, x0 - 1, PE_Y1 + dy, x1 + 1, PE_Y1 + dy, SAPATO_S)
    return contornar(d, CONTORNO)


# --------------------------------------------------------------------- cabelo
#
# A cabeca nao se mexe, entao o cabelo e o MESMO nos quatro quadros.
def cabelo_curto():
    d = {}
    ret(d, CABECA_X0 - 1, CABECA_Y0 - 2, CABECA_X1 + 1, 18, BASE)
    # franja irregular, para nao virar um capacete reto
    for x in range(CABECA_X0 - 1, CABECA_X1 + 2):
        if (x // 2) % 2 == 0:
            ret(d, x, 19, x, 20, BASE)
    ret(d, CABECA_X0 - 1, 16, CABECA_X1 + 1, 18, SOMBRA)
    for canto in (CABECA_X0 - 1, CABECA_X1 + 1):
        s = 1 if canto == CABECA_X0 - 1 else -1
        for i in range(3):
            for j in range(3 - i):
                d.pop((canto + s * i, CABECA_Y0 - 2 + j), None)
    return contornar(d, BORDA)


def cabelo_longo():
    d = cabelo_curto()
    d = {k: v for k, v in d.items() if v != BORDA}
    # Mechas descendo pelos lados, ate o ombro. Quatro pixels de largura, e nao
    # dois: com dois elas liam como antena, nao como cabelo.
    for x0 in (CABECA_X0 - 3, CABECA_X1 - 1):
        ret(d, x0, 10, x0 + 4, TORSO_Y0 + 4, BASE)
        ret(d, x0, TORSO_Y0, x0 + 4, TORSO_Y0 + 4, SOMBRA)
        # ponta afinada, para a mecha nao terminar num corte reto
        d.pop((x0, TORSO_Y0 + 4), None)
        d.pop((x0 + 4, TORSO_Y0 + 4), None)
    return contornar(d, BORDA)


def cabelo_coque():
    d = cabelo_curto()
    d = {k: v for k, v in d.items() if v != BORDA}
    # O coque ocupa toda a folga que existe acima da cabeca - cinco pixels - e
    # nem um a mais. Ver ForaDoQuadro.
    ret(d, CX - 5, 0, CX + 4, CABECA_Y0 - 1, BASE)
    ret(d, CX - 5, CABECA_Y0 - 2, CX + 4, CABECA_Y0 - 1, SOMBRA)
    for canto_x, s in ((CX - 5, 1), (CX + 4, -1)):
        d.pop((canto_x, 0), None)
        d.pop((canto_x + s, 0), None)
        d.pop((canto_x, 1), None)
    return contornar(d, BORDA)


def cabelo_espetado():
    d = {}
    ret(d, CABECA_X0 - 1, CABECA_Y0, CABECA_X1 + 1, 18, BASE)
    ret(d, CABECA_X0 - 1, 15, CABECA_X1 + 1, 18, SOMBRA)

    # Espetos de tres pixels de largura com dois de folga entre eles: colados,
    # os contornos de dois espetos vizinhos preenchiam o vao e o cabelo virava
    # uma coroa serrilhada. A altura cabe nos quatro pixels que sobram.
    alturas = [2, 4, 3, 4, 2, 3, 4, 2]
    x = CABECA_X0 - 1
    for h in alturas:
        if x + 2 > CABECA_X1 + 1:
            break
        ret(d, x, CABECA_Y0 - h, x + 2, CABECA_Y0, BASE)
        d.pop((x, CABECA_Y0 - h), None)          # ponta afinada
        d.pop((x + 2, CABECA_Y0 - h), None)
        x += 5
    return contornar(d, BORDA)


# --------------------------------------------------------------------- camisa
def _mangas(d, quadro, ate):
    """Mangas sobre os bracos, descendo ate a linha dada."""
    p = passo(quadro)
    for (x0, x1), dy in ((BRACO_E, p['braco_e']), (BRACO_D, p['braco_d'])):
        ret(d, x0 - 1, TORSO_Y0 + dy, x1 + 1, ate + dy, BASE)
        ret(d, x0 - 1, ate - 1 + dy, x1 + 1, ate + dy, SOMBRA)


def camisa_camiseta(quadro):
    d = {}
    ret(d, TORSO_X0 - 1, TORSO_Y0 - 1, TORSO_X1 + 1, TORSO_Y1, BASE)
    ret(d, TORSO_X0 - 1, TORSO_Y1 - 2, TORSO_X1 + 1, TORSO_Y1, SOMBRA)
    _mangas(d, quadro, TORSO_Y0 + 5)
    # gola
    ret(d, CX - 3, TORSO_Y0 - 1, CX + 2, TORSO_Y0, SOMBRA)
    return contornar(d, BORDA)


def camisa_regata(quadro):
    d = {}
    ret(d, TORSO_X0 + 1, TORSO_Y0 - 1, TORSO_X1 - 1, TORSO_Y1, BASE)
    ret(d, TORSO_X0 + 1, TORSO_Y1 - 2, TORSO_X1 - 1, TORSO_Y1, SOMBRA)
    # alcas no lugar das mangas
    ret(d, TORSO_X0 + 1, TORSO_Y0 - 2, TORSO_X0 + 3, TORSO_Y0, BASE)
    ret(d, TORSO_X1 - 3, TORSO_Y0 - 2, TORSO_X1 - 1, TORSO_Y0, BASE)
    return contornar(d, BORDA)


def camisa_moletom(quadro):
    d = {}
    ret(d, TORSO_X0 - 2, TORSO_Y0 - 2, TORSO_X1 + 2, TORSO_Y1 + 1, BASE)
    ret(d, TORSO_X0 - 2, TORSO_Y1 - 2, TORSO_X1 + 2, TORSO_Y1 + 1, SOMBRA)
    _mangas(d, quadro, TORSO_Y1)
    # capuz atras do pescoco
    ret(d, CX - 5, TORSO_Y0 - 3, CX + 4, TORSO_Y0 - 1, SOMBRA)
    # cordao
    ret(d, CX - 2, TORSO_Y0 + 1, CX - 2, TORSO_Y0 + 4, SOMBRA)
    ret(d, CX + 1, TORSO_Y0 + 1, CX + 1, TORSO_Y0 + 4, SOMBRA)
    return contornar(d, BORDA)


# ---------------------------------------------------------------------- calca
def calca_longa(quadro):
    d = {}
    p = passo(quadro)
    ret(d, TORSO_X0, TORSO_Y1 - 1, TORSO_X1, PERNA_Y0 + 1, BASE)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PERNA_Y0, x1 + 1, PERNA_Y1 + dy, BASE)
        ret(d, x0 - 1, PERNA_Y1 - 1 + dy, x1 + 1, PERNA_Y1 + dy, SOMBRA)
    return contornar(d, BORDA)


def calca_bermuda(quadro):
    d = {}
    p = passo(quadro)
    ret(d, TORSO_X0, TORSO_Y1 - 1, TORSO_X1, PERNA_Y0 + 1, BASE)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PERNA_Y0, x1 + 1, PERNA_Y0 + 3, BASE)
        ret(d, x0 - 1, PERNA_Y0 + 2, x1 + 1, PERNA_Y0 + 3, SOMBRA)
    return contornar(d, BORDA)


def calca_saia(quadro):
    d = {}
    ret(d, TORSO_X0, TORSO_Y1 - 1, TORSO_X1, PERNA_Y0, BASE)
    # abre para fora conforme desce
    for i, y in enumerate(range(PERNA_Y0, PERNA_Y0 + 5)):
        ret(d, TORSO_X0 - i, y, TORSO_X1 + i, y, BASE)
    ret(d, TORSO_X0 - 4, PERNA_Y0 + 4, TORSO_X1 + 4, PERNA_Y0 + 4, SOMBRA)
    return contornar(d, BORDA)


# ------------------------------------------------------------------- escrever
PECAS = {
    'corpo':            corpo,
    'rosto':            rosto,
    'sapato':           sapato,
    'cabelo_curto':     lambda q: cabelo_curto(),
    'cabelo_longo':     lambda q: cabelo_longo(),
    'cabelo_coque':     lambda q: cabelo_coque(),
    'cabelo_espetado':  lambda q: cabelo_espetado(),
    'camisa_camiseta':  camisa_camiseta,
    'camisa_regata':    camisa_regata,
    'camisa_moletom':   camisa_moletom,
    'calca_longa':      calca_longa,
    'calca_bermuda':    calca_bermuda,
    'calca_saia':       calca_saia,
}

CANTOS = [(0, 0), (L, 0), (0, L), (L, L)]


def atlas(func):
    im = Image.new('RGBA', (L * 2, L * 2), (0, 0, 0, 0))
    px = im.load()
    for q, (cx, cy) in enumerate(CANTOS):
        for (x, y), cor in func(q).items():
            px[cx + x, cy + y] = cor
    return im


def json_do_atlas(nome):
    """O mesmo formato de atlas do Piskel que o DrawAnimatedComponent ja le."""
    quadros = []
    for q, (cx, cy) in enumerate(CANTOS):
        quadros.append(
            '    "%s%d.png": { "frame": { "x": %d, "y": %d, "w": %d, "h": %d },'
            ' "rotated": false, "trimmed": false,'
            ' "spriteSourceSize": { "x": 0, "y": 0, "w": %d, "h": %d },'
            ' "sourceSize": { "w": %d, "h": %d } }'
            % (nome, q, cx, cy, L, L, L, L, L, L))
    return ('{\n  "frames": {\n' + ',\n'.join(quadros) + '\n  },\n'
            '  "meta": { "app": "gerar_camadas.py", "version": "1.0",'
            ' "image": "%s.png", "format": "RGBA8888",'
            ' "size": { "w": %d, "h": %d } }\n}\n' % (nome, L * 2, L * 2))


def main(destino):
    os.makedirs(destino, exist_ok=True)
    for nome, func in PECAS.items():
        atlas(func).save(os.path.join(destino, nome + '.png'))
        with open(os.path.join(destino, nome + '.json'), 'w', encoding='utf-8') as f:
            f.write(json_do_atlas(nome))
        print('  ' + nome)
    print('%d pecas em %s' % (len(PECAS), destino))


if __name__ == '__main__':
    import sys
    main(sys.argv[1] if len(sys.argv) > 1 else '.')
