#!/usr/bin/env python3
"""
Arte PLACEHOLDER dos projeteis de Estruturas de Dados do Salles.

Mesmo processo da arte do Julio: desenho programatico, pixel art de borda dura,
paleta curta, e um atlas JSON no formato que o DrawAnimatedComponent le.

O que importa num bullet hell nao e detalhe, e SILHUETA e COR: o projetil
atravessa a tela em menos de um segundo. Por isso cada estrutura tem um contorno
diferente o bastante para ser reconhecida pelo canto do olho:

  ListaSimples  caixa com UM espeto a direita  -> o ponteiro next, assimetrico,
                                                  "esta indo para la"
  Arvore        no com DOIS filhos abaixo      -> a bifurcacao, simetrica

As quatro telas de cada animacao sao uma pulsacao curta. Na arvore ela alterna
entre os dois filhos, que le como a busca escolhendo um ramo.
"""
import json, os

L = 32          # lado da celula
COLS, ROWS = 4, 2

# Paleta. Separada das cores do Julio (cinza/ciano/rosa) de proposito: dois
# chefes com a mesma paleta viram o mesmo jogo.
AMBAR      = (232, 163,  61, 255)   # lista
AMBAR_CLARO= (255, 214, 140, 255)
AMBAR_BORDA= (150,  94,  20, 255)
VERDE      = ( 91, 191, 106, 255)   # arvore
VERDE_CLARO= (168, 235, 178, 255)
VERDE_BORDA= ( 42, 110,  54, 255)
VAZIO      = (0, 0, 0, 0)


def tela():
    return [[VAZIO for _ in range(L)] for _ in range(L)]


def retangulo(px, x0, y0, x1, y1, cor):
    for y in range(max(0, y0), min(L, y1 + 1)):
        for x in range(max(0, x0), min(L, x1 + 1)):
            px[y][x] = cor


def contorno(px, x0, y0, x1, y1, cor):
    for x in range(max(0, x0), min(L, x1 + 1)):
        if 0 <= y0 < L: px[y0][x] = cor
        if 0 <= y1 < L: px[y1][x] = cor
    for y in range(max(0, y0), min(L, y1 + 1)):
        if 0 <= x0 < L: px[y][x0] = cor
        if 0 <= x1 < L: px[y][x1] = cor


def disco(px, cx, cy, r, cor):
    for y in range(max(0, cy - r), min(L, cy + r + 1)):
        for x in range(max(0, cx - r), min(L, cx + r + 1)):
            if (x - cx) ** 2 + (y - cy) ** 2 <= (r + 0.5) ** 2:
                px[y][x] = cor


def disco_contornado(px, cx, cy, r, miolo, borda):
    """Disco com contorno de UM pixel.

    Desenhar dois discos de raios diferentes nao da isso: o circulo inteiro em
    coordenadas de pixel tem degraus, e a diferenca entre r e r-1 sai com um
    pixel em alguns angulos e tres em outros - o no parecia espetado."""
    dentro = lambda x, y, raio: (x - cx) ** 2 + (y - cy) ** 2 <= (raio + 0.5) ** 2
    for y in range(max(0, cy - r), min(L, cy + r + 1)):
        for x in range(max(0, cx - r), min(L, cx + r + 1)):
            if not dentro(x, y, r):
                continue
            vizinho_fora = any(not dentro(x + dx, y + dy, r)
                               for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            px[y][x] = borda if vizinho_fora else miolo


def linha(px, x0, y0, x1, y1, cor, grossura=1):
    passos = max(abs(x1 - x0), abs(y1 - y0))
    if passos == 0: passos = 1
    for i in range(passos + 1):
        x = round(x0 + (x1 - x0) * i / passos)
        y = round(y0 + (y1 - y0) * i / passos)
        for dx in range(grossura):
            for dy in range(grossura):
                if 0 <= x + dx < L and 0 <= y + dy < L:
                    px[y + dy][x + dx] = cor


# ---------------------------------------------------------------------------
# ListaSimples: a caixa do no, com o espeto do "next" a direita.
#
# O espeto ANDA um pixel a cada quadro e volta. Le como o ponteiro avancando,
# e da vida ao sprite sem mudar a silhueta, que precisa ficar reconhecivel.
# ---------------------------------------------------------------------------
def lista_simples(quadro):
    px = tela()
    avanco = [0, 1, 2, 1][quadro]

    # corpo do no
    retangulo(px, 3, 9, 19, 22, AMBAR)
    contorno(px, 3, 9, 19, 22, AMBAR_BORDA)
    # o campo de valor, so uma marca clara dentro
    retangulo(px, 6, 12, 12, 15, AMBAR_CLARO)

    # o ponteiro next: haste e ponta
    x0 = 20 + avanco
    linha(px, x0, 15, x0 + 6, 15, AMBAR, grossura=2)
    linha(px, x0 + 4, 13, x0 + 6, 15, AMBAR_CLARO, grossura=2)
    linha(px, x0 + 4, 17, x0 + 6, 15, AMBAR_CLARO, grossura=2)
    return px


# ---------------------------------------------------------------------------
# Arvore: o no raiz com dois filhos abaixo.
#
# Os filhos acendem alternados ao longo dos quatro quadros - esquerdo, os dois,
# direito, os dois. Le como a busca escolhendo um ramo, que e exatamente o que a
# fase 3 faz: cada projetil desce da raiz ate uma folha diferente.
# ---------------------------------------------------------------------------
def arvore(quadro):
    px = tela()
    aceso_esq = quadro in (0, 1, 3)
    aceso_dir = quadro in (1, 2, 3)

    # os ramos primeiro, para os nos ficarem por cima
    linha(px, 15, 11, 8, 21, VERDE_BORDA, grossura=2)
    linha(px, 17, 11, 24, 21, VERDE_BORDA, grossura=2)

    # raiz
    disco_contornado(px, 16, 8, 6, VERDE, VERDE_BORDA)
    disco(px, 16, 8, 2, VERDE_CLARO)

    # filhos
    for cx, aceso in ((8, aceso_esq), (24, aceso_dir)):
        disco_contornado(px, cx, 24, 5, VERDE_CLARO if aceso else VERDE, VERDE_BORDA)
    return px


def montar(destino_png, destino_json, animacoes):
    from PIL import Image
    folha = Image.new("RGBA", (L * COLS, L * ROWS), (0, 0, 0, 0))
    frames = []
    for linha_idx, (nome, fn) in enumerate(animacoes):
        for q in range(COLS):
            px = fn(q)
            cel = Image.new("RGBA", (L, L))
            cel.putdata([px[y][x] for y in range(L) for x in range(L)])
            x0, y0 = q * L, linha_idx * L
            folha.paste(cel, (x0, y0))
            frames.append({
                "filename": f"{nome} {q}.placeholder",
                "frame": {"x": x0, "y": y0, "w": L, "h": L},
            })
    folha.save(destino_png)
    json.dump({
        "frames": frames,
        "meta": {"app": "placeholder", "size": {"w": L * COLS, "h": L * ROWS}, "scale": "1"},
    }, open(destino_json, "w"), indent=2)
    print(f"  {destino_png}  {L*COLS}x{L*ROWS}, {len(frames)} quadros")


if __name__ == "__main__":
    import sys
    saida = sys.argv[1] if len(sys.argv) > 1 else "."
    montar(os.path.join(saida, "DPIBHSallesEstruturas.png"),
           os.path.join(saida, "DPIBHSallesEstruturas.json"),
           [("ListaSimples", lista_simples), ("Arvore", arvore)])
