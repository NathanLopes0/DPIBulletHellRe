#!/usr/bin/env python3
"""
Gera Assets/Sounds/sinal.wav: o som do sinal que um aluno da ao parceiro na dupla (Botao 3).

Dois toques curtos subindo (do agudo para o mais agudo), "plim-plim", no formato dos outros sons
do jogo: mono, 22050 Hz, 16 bits. Curto e baixo de proposito - ele toca no meio da batalha, por cima
da musica e dos tiros, e tem de se ouvir sem assustar.

Uso:  python3 Ferramentas/gerar_sinal.py [destino]
"""

import math
import os
import struct
import sys
import wave

TAXA = 22050
VOLUME = 0.32            # da escala inteira de 16 bits
TOQUES = [               # (frequencia em Hz, inicio em s, duracao em s)
    (1046.5, 0.00, 0.09),    # do
    (1568.0, 0.07, 0.12),    # sol
]
DURACAO = 0.20


def amostra(t):
    total = 0.0
    for freq, inicio, dur in TOQUES:
        u = t - inicio
        if u < 0.0 or u > dur:
            continue
        # Ataque rapido e queda exponencial: um toque de sino, sem estalo no comeco.
        envelope = min(1.0, u / 0.006) * math.exp(-u * 28.0)
        # Um harmonico leve dobra a frequencia e deixa o som mais "plim" que "bip".
        onda = math.sin(2 * math.pi * freq * u) + 0.25 * math.sin(4 * math.pi * freq * u)
        total += envelope * onda
    return total


def main(destino):
    n = int(TAXA * DURACAO)
    quadros = bytearray()
    for i in range(n):
        v = amostra(i / TAXA) * VOLUME / 1.25
        v = max(-1.0, min(1.0, v))
        quadros += struct.pack('<h', int(v * 32767))
    os.makedirs(os.path.dirname(destino) or '.', exist_ok=True)
    with wave.open(destino, 'wb') as saida:
        saida.setnchannels(1)
        saida.setsampwidth(2)
        saida.setframerate(TAXA)
        saida.writeframes(bytes(quadros))
    print(f'{destino}: {n} amostras, {DURACAO:.2f} s')


if __name__ == '__main__':
    raiz = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(raiz, 'Assets', 'Sounds', 'sinal.wav'))
