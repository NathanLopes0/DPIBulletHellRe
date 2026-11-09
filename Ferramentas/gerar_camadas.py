#!/usr/bin/env python3
"""
Gera a arte de placeholder em CAMADAS para a criacao de personagem.

Cada peca sai como um atlas de 128x128 com quatro quadros de 64x64, na mesma
disposicao do DPIBHPlayer.png que o jogo ja le:

    quadro 0 = (0,0)    quadro 1 = (64,0)
    quadro 2 = (0,64)   quadro 3 = (64,64)

Tudo sai em LUMINANCIA (cinza + alfa), porque a cor e escolhida pelo aluno e
aplicada por multiplicacao na hora de compor: cinza 255 devolve a cor escolhida
tal e qual, e os tons mais escuros viram a sombra dela. Vale para o cabelo, para
a roupa, para o sapato, para a mochila e TAMBEM para o corpo, cujo tom de pele e
uma escolha como as outras.

O sapato ja saiu em cor final, numa camada fixa, para nao clarear junto com a
pele. Virou categoria, numa peca propria - o que tambem o separa da pele -, e o
tenis grafite padrao e o sapato de antes.

A ordem de composicao e: corpo, sapato, calca, camisa, mochila, cabelo. A mochila
vai entre a camisa e o cabelo: o cabelo comprido cai por cima dela.

O PERSONAGEM E VISTO DE COSTAS. Ele olha para cima, para o professor, que fica
no topo da tela - e por isso o sprite original do jogo nao tem rosto: o que
aparece e a nuca. Nao ha olho nem boca para desenhar, e o cabelo cobre a cabeca
inteira em vez de parar numa franja. Isso APAGOU uma camada: a peca "rosto",
que so existia para os olhos nao serem tingidos junto com a pele.

A animacao segue a do sprite original: quadros 1 e 3 sao a pose neutra, 0 e 2
sao os passos, e a CABECA NAO SE MEXE em nenhum deles - por isso o cabelo e uma
imagem so, repetida nos quatro quadros.
"""

from PIL import Image
import os

L = 64  # lado do quadro

# ---------------------------------------------------------------- cores fixas
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

    # A NUCA, e nao um rosto: visto de costas, o que aparece abaixo do cabelo e
    # a sombra da base da cabeca. E a PROPRIA pele mais escura, entao acompanha
    # o tom escolhido em vez de ser uma cor fixa por cima dele.
    ret(d, CABECA_X0 + 2, CABECA_Y1 - 3, CABECA_X1 - 2, CABECA_Y1, SOMBRA)

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


# --------------------------------------------------------------------- cabelo
#
# DE COSTAS o cabelo cobre a cabeca INTEIRA, parando um pouco acima da nuca -
# nao ha franja, porque franja e coisa de quem esta de frente.
#
# A cabeca nao se mexe, entao o cabelo e o MESMO nos quatro quadros.

# Ate onde o cabelo desce. Abaixo disto aparece a nuca.
CABELO_Y1 = CABECA_Y1 - 4


def _calota(d, y0=CABECA_Y0 - 2, y1=CABELO_Y1):
    """A massa de cabelo que cobre o cranio, com os cantos de cima comidos."""
    ret(d, CABECA_X0 - 1, y0, CABECA_X1 + 1, y1, BASE)
    # a parte de baixo mais escura, para a cabeca nao sair chapada
    ret(d, CABECA_X0 - 1, y1 - 4, CABECA_X1 + 1, y1, SOMBRA)
    for canto in (CABECA_X0 - 1, CABECA_X1 + 1):
        s = 1 if canto == CABECA_X0 - 1 else -1
        for i in range(3):
            for j in range(3 - i):
                d.pop((canto + s * i, y0 + j), None)
    return d


def cabelo_curto():
    d = _calota({})
    # barra irregular embaixo, para o corte nao sair reto como um capacete
    for x in range(CABECA_X0 - 1, CABECA_X1 + 2):
        if (x // 3) % 2 == 0:
            ret(d, x, CABELO_Y1 + 1, x, CABELO_Y1 + 1, SOMBRA)
    return contornar(d, BORDA)


def cabelo_longo():
    # De costas, cabelo comprido e uma massa SO: ele desce pelas costas em vez
    # de emoldurar um rosto que nao esta a vista.
    d = _calota({})

    # DESCE ATE O OMBRO, E NAO ALEM, e mais estreito que o torso. Na primeira
    # versao ele ia ate a cintura e era mais largo que o corpo: cobria a camisa
    # inteira, e escolher roupa deixava de ter efeito visivel.
    ret(d, CABECA_X0 + 2, 14, CABECA_X1 - 2, TORSO_Y0 + 3, BASE)
    ret(d, CABECA_X0 + 2, TORSO_Y0, CABECA_X1 - 2, TORSO_Y0 + 3, SOMBRA)
    # ponta levemente afinada, para nao terminar num corte reto
    for x in (CABECA_X0 + 2, CABECA_X1 - 2):
        d.pop((x, TORSO_Y0 + 3), None)
    return contornar(d, BORDA)


def cabelo_coque():
    d = _calota({})
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
    d = _calota({}, y0=CABECA_Y0)

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


def cabelo_raspado():
    """Cabelo bem curto, colado no cranio. De costas e so uma sombra na cabeca."""
    d = {}
    ret(d, CABECA_X0, CABECA_Y0 - 1, CABECA_X1, CABELO_Y1 - 2, BASE)
    ret(d, CABECA_X0, CABELO_Y1 - 5, CABECA_X1, CABELO_Y1 - 2, SOMBRA)
    for canto in (CABECA_X0, CABECA_X1):
        s = 1 if canto == CABECA_X0 else -1
        for i in range(3):
            for j in range(3 - i):
                d.pop((canto + s * i, CABECA_Y0 - 1 + j), None)
    return contornar(d, BORDA)


def cabelo_careca():
    """Sem cabelo: a cabeca do corpo, na cor da pele, aparece inteira."""
    return {}


def cabelo_medio():
    """Na altura do pescoco, reto embaixo: cobre a nuca, mas nao chega ao ombro."""
    d = _calota({}, y1=CABECA_Y1 + 1)
    ret(d, CABECA_X0 - 1, CABECA_Y1 - 3, CABECA_X1 + 1, CABECA_Y1 + 1, SOMBRA)
    # as pontas, um pouco desfiadas
    for x in range(CABECA_X0 - 1, CABECA_X1 + 2, 4):
        d.pop((x, CABECA_Y1 + 1), None)
    return contornar(d, BORDA)


def cabelo_chanel():
    """O chanel: mais cheio embaixo, cortado reto na altura do queixo."""
    d = _calota({}, y1=CABECA_Y1)
    ret(d, CABECA_X0 - 2, 14, CABECA_X0 - 1, CABECA_Y1, BASE)
    ret(d, CABECA_X1 + 1, 14, CABECA_X1 + 2, CABECA_Y1, BASE)
    ret(d, CABECA_X0 - 2, CABECA_Y1 - 2, CABECA_X1 + 2, CABECA_Y1, SOMBRA)
    return contornar(d, BORDA)


def cabelo_rabo():
    """Rabo de cavalo: preso embaixo da cabeca, descendo grosso pelo meio das costas."""
    d = _calota({})
    # o elastico, na base do cabelo, e o rabo que sai dele e afina ate o meio das costas
    ret(d, CX - 3, CABELO_Y1 - 2, CX + 2, CABELO_Y1 - 1, BORDA)
    for y in range(CABELO_Y1, TORSO_Y0 + 9):
        meia = 4 if y < CABELO_Y1 + 6 else (3 if y < CABELO_Y1 + 12 else 2)
        if y >= TORSO_Y0 + 7:
            meia = 1
        ret(d, CX - meia, y, CX + meia - 1, y, BASE)
        # a mecha do meio, mais escura, para o rabo ter volume
        d[(CX - 1, y)] = SOMBRA
    return contornar(d, BORDA)


def cabelo_marias():
    """Maria-chiquinha: dois tufos presos atras das orelhas, caindo ate o ombro."""
    d = _calota({})
    for lado in (-1, 1):
        borda = CABECA_X0 - 1 if lado < 0 else CABECA_X1 + 1
        # o elastico, colado na cabeca
        ret(d, min(borda, borda + 2 * lado), 19, max(borda, borda + 2 * lado), 20, BORDA)
        # o tufo: tres de largura, descendo e abrindo um pixel
        for y in range(21, TORSO_Y0 + 4):
            abre = 1 if y > 28 else 0
            x0 = borda + (1 + abre) * lado
            ret(d, min(x0, x0 + 2 * lado), y, max(x0, x0 + 2 * lado), y, BASE if y < TORSO_Y0 + 1 else SOMBRA)
    return contornar(d, BORDA)


def cabelo_black():
    """Black power: o volume redondo passa da cabeca para todos os lados."""
    d = {}
    cx, cy, rx, ry = 31.5, 15.5, 16.0, 15.5
    for y in range(0, CABELO_Y1 + 2):
        for x in range(14, 50):
            dx, dy = (x - cx) / rx, (y - cy) / ry
            # a borda ondulada: o raio muda com o angulo
            if dx * dx + dy * dy <= 1.0 - 0.06 * ((x * 7 + y * 3) % 3):
                d[(x, y)] = BASE
    # a parte de baixo, sobre a nuca, mais escura
    for (x, y) in list(d):
        if y >= CABELO_Y1 - 3:
            d[(x, y)] = SOMBRA
    # a textura dos cachos: pequenos arcos de sombra, sem formar grade
    for (x, y) in list(d):
        if (x * 7 + y * 13 + (x * y) % 5) % 17 == 0 and (x + 1, y + 1) in d:
            d[(x, y)] = SOMBRA
            d[(x + 1, y + 1)] = SOMBRA
    return contornar(d, BORDA)


def cabelo_cacheado():
    """Cacheado, na altura do ombro: mais cheio embaixo, com a borda em cachos e as voltas marcadas."""
    d = _calota({}, y1=CABECA_Y1)
    # o volume abre para os lados conforme desce
    for y in range(12, CABECA_Y1 + 3):
        abre = 1 + (y - 12) // 6
        ret(d, CABECA_X0 - 1 - abre, y, CABECA_X1 + 1 + abre, y, BASE)
    # a borda de baixo em cachos: meias-luas de tres pixels
    for x in range(CABECA_X0 - 4, CABECA_X1 + 5, 3):
        ret(d, x, CABECA_Y1 + 3, x + 1, CABECA_Y1 + 3, BASE)
    # as voltas dos cachos: pequenos arcos de sombra, espalhados sem grade
    voltas = [(23, 10), (29, 8), (36, 11), (41, 9), (21, 16), (27, 15), (33, 17), (39, 15),
              (24, 21), (30, 22), (36, 21), (42, 22), (19, 27), (26, 28), (32, 27), (38, 28), (44, 27)]
    for x, y in voltas:
        for px, py in ((x, y), (x + 1, y + 1), (x - 1, y + 1)):
            if (px, py) in d:
                d[(px, py)] = SOMBRA
    return contornar(d, BORDA)


def cabelo_tranca():
    """Uma tranca so, do alto da nuca ate o meio das costas, em gomos."""
    d = _calota({})
    y = 22
    while y < TORSO_Y1 - 1:
        lado = ((y - 22) // 3) % 2
        ret(d, CX - 3 + lado, y, CX + 1 + lado, y + 2, BASE)
        ret(d, CX - 3 + lado, y + 2, CX + 1 + lado, y + 2, SOMBRA)
        y += 3
    ret(d, CX - 2, y, CX + 1, y + 1, SOMBRA)                # o elastico na ponta
    ret(d, CX - 1, y + 2, CX, y + 3, BASE)                  # o pincel
    return contornar(d, BORDA)


def cabelo_dreads():
    """Dreads: mechas grossas e separadas, caindo da nuca ate o ombro."""
    d = _calota({})
    for i, x in enumerate(range(CABECA_X0 - 1, CABECA_X1 + 1, 3)):
        fim = TORSO_Y0 + 1 + (i * 5) % 4
        ret(d, x, CABELO_Y1 - 2, x + 1, fim, BASE)
        ret(d, x, fim - 1, x + 1, fim, SOMBRA)
    return contornar(d, BORDA)


def cabelo_moicano():
    """Moicano: as laterais raspadas, e a crista no meio, espetada para cima."""
    d = {}
    # o raspado dos lados, bem escuro: so a sombra do cabelo
    ret(d, CABECA_X0, CABECA_Y0 + 1, CABECA_X1, CABELO_Y1 - 2, BORDA)
    for canto in (CABECA_X0, CABECA_X1):
        s = 1 if canto == CABECA_X0 else -1
        for i in range(3):
            for j in range(3 - i):
                d.pop((canto + s * i, CABECA_Y0 + 1 + j), None)
    # a crista: do alto da cabeca ate a nuca, com as pontas acima
    ret(d, CX - 3, CABECA_Y0 - 1, CX + 2, CABELO_Y1, BASE)
    for x, h in ((CX - 3, 3), (CX - 1, 5), (CX + 1, 4)):
        ret(d, x, CABECA_Y0 - h, x + 1, CABECA_Y0 - 1, BASE)
    ret(d, CX - 3, CABELO_Y1 - 3, CX + 2, CABELO_Y1, SOMBRA)
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


def camisa_manga_longa(quadro):
    """Camiseta de manga comprida: as mangas descem ate o punho."""
    d = {}
    ret(d, TORSO_X0 - 1, TORSO_Y0 - 1, TORSO_X1 + 1, TORSO_Y1, BASE)
    ret(d, TORSO_X0 - 1, TORSO_Y1 - 2, TORSO_X1 + 1, TORSO_Y1, SOMBRA)
    _mangas(d, quadro, TORSO_Y1 - 1)
    ret(d, CX - 3, TORSO_Y0 - 1, CX + 2, TORSO_Y0, SOMBRA)
    return contornar(d, BORDA)


def camisa_listrada(quadro):
    """Camiseta listrada: faixas de sombra atravessando as costas."""
    d = camisa_camiseta(quadro)
    for (x, y), cor in list(d.items()):
        if cor == BASE and (y - TORSO_Y0) % 4 == 2:
            d[(x, y)] = SOMBRA
    return d


def camisa_social(quadro):
    """Camisa social: a gola dobrada, a pala nas costas e as mangas compridas."""
    d = {}
    ret(d, TORSO_X0 - 1, TORSO_Y0 - 1, TORSO_X1 + 1, TORSO_Y1 + 1, BASE)
    _mangas(d, quadro, TORSO_Y1 - 1)
    # a pala: a costura que atravessa os ombros
    ret(d, TORSO_X0 - 1, TORSO_Y0 + 2, TORSO_X1 + 1, TORSO_Y0 + 2, SOMBRA)
    # a gola em pe, atras do pescoco
    ret(d, CX - 4, TORSO_Y0 - 3, CX + 3, TORSO_Y0 - 1, BASE)
    ret(d, CX - 4, TORSO_Y0 - 1, CX + 3, TORSO_Y0 - 1, SOMBRA)
    # a prega no meio das costas
    ret(d, CX - 1, TORSO_Y0 + 3, CX, TORSO_Y1, SOMBRA)
    return contornar(d, BORDA)


def camisa_jaqueta(quadro):
    """Jaqueta: mais larga que o corpo, com a gola, a barra e a costura do meio."""
    d = {}
    ret(d, TORSO_X0 - 2, TORSO_Y0 - 2, TORSO_X1 + 2, TORSO_Y1 + 2, BASE)
    _mangas(d, quadro, TORSO_Y1)
    ret(d, CX - 5, TORSO_Y0 - 3, CX + 4, TORSO_Y0 - 1, SOMBRA)     # a gola
    ret(d, TORSO_X0 - 2, TORSO_Y1, TORSO_X1 + 2, TORSO_Y1 + 2, SOMBRA)   # a barra
    ret(d, CX - 1, TORSO_Y0, CX - 1, TORSO_Y1 - 1, SOMBRA)         # a costura
    return contornar(d, BORDA)


# O numero da camisa de time, em pixels: tres de largura, cinco de altura.
_ALGARISMOS = {
    '1': ['.#.', '##.', '.#.', '.#.', '###'],
    '0': ['###', '#.#', '#.#', '#.#', '###'],
}


def camisa_time(quadro):
    """Camisa de time, com o 10 nas costas."""
    d = camisa_camiseta(quadro)
    # a gola em V, e o numero no meio das costas
    x0, y0 = CX - 4, TORSO_Y0 + 3
    for i, alg in enumerate('10'):
        for dy, linha in enumerate(_ALGARISMOS[alg]):
            for dx, c in enumerate(linha):
                if c == '#':
                    d[(x0 + i * 5 + dx, y0 + dy)] = BORDA
    return d


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


def calca_short(quadro):
    """Short: so a cintura e um palmo de perna."""
    d = {}
    ret(d, TORSO_X0, TORSO_Y1 - 1, TORSO_X1, PERNA_Y0 + 1, BASE)
    ret(d, TORSO_X0, PERNA_Y0, TORSO_X1, PERNA_Y0 + 1, SOMBRA)
    return contornar(d, BORDA)


def calca_pantalona(quadro):
    """Pantalona: as pernas abrem conforme descem, ate cobrir o sapato pela metade."""
    d = {}
    p = passo(quadro)
    ret(d, TORSO_X0, TORSO_Y1 - 1, TORSO_X1, PERNA_Y0 + 1, BASE)
    for (x0, x1), dy, s in ((PERNA_E, p['pe_e'], -1), (PERNA_D, p['pe_d'], 1)):
        for y in range(PERNA_Y0, PE_Y0 + 1 + dy):
            abre = (y - PERNA_Y0) // 3
            xa, xb = (x0 - 1 - abre, x1 + 1) if s < 0 else (x0 - 1, x1 + 1 + abre)
            ret(d, xa, y, xb, y, BASE)
        ret(d, x0 - 3, PE_Y0 - 1 + dy, x1 + 3, PE_Y0 + dy, SOMBRA)
    return contornar(d, BORDA)


def calca_rasgada(quadro):
    """Calca rasgada: a comprida, com os joelhos abertos - a pele aparece por baixo."""
    d = calca_longa(quadro)
    p = passo(quadro)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        joelho = PERNA_Y0 + 3
        for x in range(x0 + 1, x1):
            d.pop((x, joelho), None)
            d.pop((x, joelho + 1), None)
        d[(x0, joelho)] = BORDA
        d[(x1, joelho + 1)] = BORDA
    return d


def calca_saia_longa(quadro):
    """Saia longa: abre da cintura ate o tornozelo."""
    d = {}
    ret(d, TORSO_X0, TORSO_Y1 - 1, TORSO_X1, PERNA_Y0, BASE)
    for i, y in enumerate(range(PERNA_Y0, PE_Y0)):
        abre = min(i // 2 + 1, 5)
        ret(d, TORSO_X0 - abre, y, TORSO_X1 + abre, y, BASE)
    ret(d, TORSO_X0 - 5, PE_Y0 - 2, TORSO_X1 + 5, PE_Y0 - 1, SOMBRA)
    return contornar(d, BORDA)


# --------------------------------------------------------------------- sapato
#
# O sapato era uma camada fixa, em cor final. Virou categoria: a peca sai em luminancia, e a cor e
# escolhida - o tenis grafite padrao e o sapato de antes.
def sapato_tenis(quadro):
    d = {}
    p = passo(quadro)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PE_Y0 + dy, x1 + 1, PE_Y1 + dy, BASE)
        ret(d, x0 - 1, PE_Y1 + dy, x1 + 1, PE_Y1 + dy, SOMBRA)
    return contornar(d, BORDA)


def sapato_cano_alto(quadro):
    """Tenis de cano alto: sobe pela canela, com a sola clara."""
    d = {}
    p = passo(quadro)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PE_Y0 - 3 + dy, x1 + 1, PE_Y1 + dy, BASE)
        ret(d, x0 - 1, PE_Y0 - 2 + dy, x1 + 1, PE_Y0 - 2 + dy, SOMBRA)
    d = contornar(d, BORDA)
    # a sola branca, que nao se tinge: cinza claro fixo nao existe aqui, entao vai a base
    return d


def sapato_bota(quadro):
    """Bota: cobre a canela inteira, com o salto marcado."""
    d = {}
    p = passo(quadro)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PERNA_Y0 + 3 + dy, x1 + 1, PE_Y1 + dy, BASE)
        ret(d, x0 - 1, PERNA_Y0 + 3 + dy, x1 + 1, PERNA_Y0 + 4 + dy, SOMBRA)
        ret(d, x0 - 1, PE_Y1 + dy, x1 + 1, PE_Y1 + dy, SOMBRA)
    return contornar(d, BORDA)


def sapato_chinelo(quadro):
    """Chinelo: so a sola e a tira - o pe aparece."""
    d = {}
    p = passo(quadro)
    for (x0, x1), dy in ((PERNA_E, p['pe_e']), (PERNA_D, p['pe_d'])):
        ret(d, x0 - 1, PE_Y1 + dy, x1 + 1, PE_Y1 + dy, BASE)
        ret(d, x0, PE_Y0 + dy, x0, PE_Y1 - 1 + dy, SOMBRA)
        ret(d, x1, PE_Y0 + dy, x1, PE_Y1 - 1 + dy, SOMBRA)
    return contornar(d, BORDA)


# --------------------------------------------------------------------- mochila
#
# De costas, a mochila e o que mais aparece depois do cabelo. Ela vai entre a camisa e o cabelo: o
# cabelo comprido cai por cima dela.
def mochila_nenhuma(quadro):
    return {}


def mochila_mochila(quadro):
    """A mochila: o corpo nas costas, o bolso da frente e as alcas nos ombros."""
    d = {}
    ret(d, TORSO_X0 + 1, TORSO_Y0 - 1, TORSO_X1 - 1, TORSO_Y1 + 1, BASE)
    ret(d, TORSO_X0 + 3, TORSO_Y0 + 6, TORSO_X1 - 3, TORSO_Y1, SOMBRA)       # o bolso
    ret(d, TORSO_X0 + 3, TORSO_Y0 + 6, TORSO_X1 - 3, TORSO_Y0 + 6, BORDA)    # o ziper do bolso
    ret(d, TORSO_X0 + 1, TORSO_Y0 - 1, TORSO_X1 - 1, TORSO_Y0, SOMBRA)       # a tampa
    ret(d, CX - 2, TORSO_Y0 - 3, CX + 1, TORSO_Y0 - 2, SOMBRA)               # a alca de mao
    return contornar(d, BORDA)


def mochila_bolsa(quadro):
    """A bolsa de lado: a alca atravessa as costas, e a bolsa fica no quadril."""
    d = {}
    for i in range(TORSO_Y1 - TORSO_Y0 + 2):
        x = TORSO_X0 + i
        ret(d, x, TORSO_Y0 - 1 + i, x + 1, TORSO_Y0 - 1 + i, SOMBRA)
    ret(d, TORSO_X1 - 3, TORSO_Y1 - 3, TORSO_X1 + 4, TORSO_Y1 + 3, BASE)
    ret(d, TORSO_X1 - 3, TORSO_Y1 - 3, TORSO_X1 + 4, TORSO_Y1 - 2, SOMBRA)
    return contornar(d, BORDA)


# ------------------------------------------------------------------- escrever
PECAS = {
    'corpo':            corpo,
    'sapato_tenis':     sapato_tenis,
    'sapato_cano_alto': sapato_cano_alto,
    'sapato_bota':      sapato_bota,
    'sapato_chinelo':   sapato_chinelo,
    'mochila_nenhuma':  mochila_nenhuma,
    'mochila_mochila':  mochila_mochila,
    'mochila_bolsa':    mochila_bolsa,
    'cabelo_curto':     lambda q: cabelo_curto(),
    'cabelo_longo':     lambda q: cabelo_longo(),
    'cabelo_coque':     lambda q: cabelo_coque(),
    'cabelo_espetado':  lambda q: cabelo_espetado(),
    'cabelo_raspado':   lambda q: cabelo_raspado(),
    'cabelo_careca':    lambda q: cabelo_careca(),
    'cabelo_medio':     lambda q: cabelo_medio(),
    'cabelo_chanel':    lambda q: cabelo_chanel(),
    'cabelo_rabo':      lambda q: cabelo_rabo(),
    'cabelo_marias':    lambda q: cabelo_marias(),
    'cabelo_black':     lambda q: cabelo_black(),
    'cabelo_cacheado':  lambda q: cabelo_cacheado(),
    'cabelo_tranca':    lambda q: cabelo_tranca(),
    'cabelo_dreads':    lambda q: cabelo_dreads(),
    'cabelo_moicano':   lambda q: cabelo_moicano(),
    'camisa_camiseta':  camisa_camiseta,
    'camisa_regata':    camisa_regata,
    'camisa_moletom':   camisa_moletom,
    'camisa_manga_longa': camisa_manga_longa,
    'camisa_listrada':  camisa_listrada,
    'camisa_social':    camisa_social,
    'camisa_jaqueta':   camisa_jaqueta,
    'camisa_time':      camisa_time,
    'calca_longa':      calca_longa,
    'calca_bermuda':    calca_bermuda,
    'calca_saia':       calca_saia,
    'calca_short':      calca_short,
    'calca_pantalona':  calca_pantalona,
    'calca_rasgada':    calca_rasgada,
    'calca_saia_longa': calca_saia_longa,
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
