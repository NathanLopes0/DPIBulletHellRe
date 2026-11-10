#!/usr/bin/env python3
"""
Gera os sons sinteticos do jogo, no formato dos outros (mono, 22050 Hz, 16 bits):

- Assets/Sounds/sinal.wav: o sinal que um aluno da ao parceiro na dupla (Botao 3). Dois toques
  curtos subindo, "plim-plim". Curto e baixo de proposito: toca no meio da batalha, por cima da
  musica e dos tiros, e tem de se ouvir sem assustar.
- Assets/Sounds/toque.wav: o toca aqui do resultado da dupla aprovada. Uma palma: tres estalos de
  ruido em 15 ms, como as maos que se encontram, e uma cauda curta.

Uso:  python3 Ferramentas/gerar_sons.py [pasta de destino]
"""

import math
import os
import random
import struct
import sys
import wave

TAXA = 22050

# ---- O sinal ----

VOLUME_DO_SINAL = 0.32           # da escala inteira de 16 bits
TOQUES_DO_SINAL = [              # (frequencia em Hz, inicio em s, duracao em s)
    (1046.5, 0.00, 0.09),        # do
    (1568.0, 0.07, 0.12),        # sol
]
DURACAO_DO_SINAL = 0.20


def amostra_do_sinal(t):
    total = 0.0
    for freq, inicio, dur in TOQUES_DO_SINAL:
        u = t - inicio
        if u < 0.0 or u > dur:
            continue
        # Ataque rapido e queda exponencial: um toque de sino, sem estalo no comeco.
        envelope = min(1.0, u / 0.006) * math.exp(-u * 28.0)
        # Um harmonico leve dobra a frequencia e deixa o som mais "plim" que "bip".
        onda = math.sin(2 * math.pi * freq * u) + 0.25 * math.sin(4 * math.pi * freq * u)
        total += envelope * onda
    return total * VOLUME_DO_SINAL / 1.25


# ---- O toque ----

VOLUME_DO_TOQUE = 0.55
ESTALOS = [0.000, 0.006, 0.013]  # os inicios dos estalos, em s
DURACAO_DO_TOQUE = 0.16


def amostras_do_toque():
    # A semente fixa: o mesmo arquivo a cada geracao.
    sorteio = random.Random(20261110)
    n = int(TAXA * DURACAO_DO_TOQUE)
    saida = []
    anterior_entrada = 0.0
    anterior_saida = 0.0
    for i in range(n):
        t = i / TAXA
        envelope = 0.0
        for inicio in ESTALOS:
            u = t - inicio
            if u >= 0.0:
                envelope += math.exp(-u * 380.0)
        envelope = 0.6 * envelope + 0.4 * math.exp(-t * 32.0)
        ruido = sorteio.uniform(-1.0, 1.0) * envelope
        # Um passa-altas de um polo: tira o grave, e a palma fica estalada, nao abafada.
        filtrado = 0.86 * (anterior_saida + ruido - anterior_entrada)
        anterior_entrada, anterior_saida = ruido, filtrado
        saida.append(filtrado * VOLUME_DO_TOQUE)
    return saida


def gravar(destino, amostras):
    quadros = bytearray()
    for v in amostras:
        v = max(-1.0, min(1.0, v))
        quadros += struct.pack('<h', int(v * 32767))
    os.makedirs(os.path.dirname(destino) or '.', exist_ok=True)
    with wave.open(destino, 'wb') as saida:
        saida.setnchannels(1)
        saida.setsampwidth(2)
        saida.setframerate(TAXA)
        saida.writeframes(bytes(quadros))
    print(f'{destino}: {len(amostras)} amostras, {len(amostras) / TAXA:.2f} s')


def main(pasta):
    sinal = [amostra_do_sinal(i / TAXA) for i in range(int(TAXA * DURACAO_DO_SINAL))]
    gravar(os.path.join(pasta, 'sinal.wav'), sinal)
    gravar(os.path.join(pasta, 'toque.wav'), amostras_do_toque())


if __name__ == '__main__':
    raiz = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(raiz, 'Assets', 'Sounds'))
