#!/usr/bin/env python3
"""Arte PROVISORIA do Goulart (INF 451 - Sistemas Operacionais) e dos projeteis
dele.

Mesmo estilo de cartao dos outros professores: e deliberadamente um
placeholder, para nao se passar por arte final. Trocar o PNG basta - o json de
quadro continua valendo.

O MOTIVO DO CARTAO E O TERMINAL: o professor do sistema operacional e a tela
de texto com o prompt e a lista de processos.

OS PROJETEIS (Assets/Teachers/Projectiles/DPIBHGoulart*), uma folha por assunto:

    Processo   P1 .. P9          o cartao do processo, na cor dele (a mesma
                                 do grafico de Gantt no Goulart.h)
    Titulo     Prontos ES        os titulos das filas (enfeite)
    Tiro       Instrucao Sinal   os tiros: a instrucao, o sinal (o quadro 1, que
               Bloco Pacote      nenhum tiro usa mais - fica para nao mudar os
                                 indices), o bloco lido do disco e o tiro do
                                 kernel, preso na caixa da VM
    Moldura    Livre P0..P7      a moldura da memoria fisica: livre, com a
               R0..R7            pagina e o bit R apagado, ou aceso
    Entrada    V0..V7 I0..I7     a entrada da tabela de paginas: valida ou
                                 invalida
    Pagina     Pg0 .. Pg7        a pagina que chega do disco, e a vitima
    Pedido     Novo Velho        o pedido de leitura na trilha do disco, e o
               Faminto           que esperou demais
    Vm         Vm1 .. Vm4        o sistema convidado de cada VM
    Trap       Trap              a instrucao privilegiada indo ao hipervisor

    python3 Ferramentas/gerar_goulart.py               # grava as folhas
    python3 Ferramentas/gerar_goulart.py --animacoes   # as animacoes de projeteis.json
"""
from PIL import Image, ImageDraw, ImageFont
import json, os, sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")

CARTAO = (28, 32, 50, 255)
BORDA = (120, 170, 255, 255)
TINTA = (232, 240, 255, 255)
TRACO = (12, 16, 28, 255)
VERDE = (130, 255, 160, 255)
AMBAR = (255, 200, 90, 255)

# As cores dos processos P1 a P9: as mesmas de Goulart::kCores.
CORES = [(255, 120, 100), (110, 200, 255), (140, 230, 140), (255, 200, 90), (200, 150, 255),
         (255, 140, 200), (120, 230, 210), (230, 230, 120), (180, 180, 210)]


def fonte(tamanho, negrito=False):
    caminho = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Bold.ttf") if negrito else CARLITO
    try:
        return ImageFont.truetype(caminho if os.path.exists(caminho) else CARLITO, tamanho)
    except OSError:
        return ImageFont.load_default()


def centrado(d, texto, cx, cy, f, cor):
    caixa = d.textbbox((0, 0), texto, font=f)
    w, h = caixa[2] - caixa[0], caixa[3] - caixa[1]
    d.text((cx - w / 2 - caixa[0], cy - h / 2 - caixa[1]), texto, font=f, fill=cor)


def escuro(c, f=0.45):
    return tuple(int(x * f) for x in c[:3]) + (255,)


def quadro_json(nome, w, h):
    return {"frames": [{
                "filename": f"{nome} 0.placeholder",
                "frame": {"x": 0, "y": 0, "w": w, "h": h},
                "rotated": False, "trimmed": False,
                "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
                "sourceSize": {"w": w, "h": h},
                "duration": 120}],
            "meta": {"app": "gerar_goulart.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def goulart():
    """O cartao do professor: o terminal, com o prompt e os processos."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)
    # A janela do terminal: a barra, os tres botoes e a tela preta.
    d.rounded_rectangle([14, 14, 113, 84], radius=5, fill=(10, 12, 20, 255), outline=BORDA, width=1)
    d.rectangle([15, 15, 112, 24], fill=(52, 60, 90, 255))
    for i, c in enumerate(((255, 110, 100), (255, 200, 90), (130, 230, 140))):
        d.ellipse([19 + 9 * i, 17, 24 + 9 * i, 22], fill=c + (255,))
    f = fonte(9, True)
    d.text((19, 28), "$ ps", font=f, fill=VERDE)
    d.text((19, 40), "PID  CPU", font=fonte(8), fill=TINTA)
    linhas = (("1", 0.95, CORES[0]), ("2", 0.40, CORES[1]), ("3", 0.70, CORES[2]))
    for k, (pid, uso, cor) in enumerate(linhas):
        y = 51 + 10 * k
        d.text((22, y - 1), pid, font=fonte(8, True), fill=cor + (255,))
        d.rectangle([40, y + 1, 40 + int(64 * uso), y + 6], fill=cor + (255,))
    d.rectangle([20, 78, 25, 81], fill=VERDE)   # o cursor
    centrado(d, "INF 451", 64, 96, fonte(12, True), AMBAR)
    centrado(d, "GOULART", 64, 110, fonte(11), TINTA)
    return im


# ---- A fase 1: o escalonador ---------------------------------------------------

PW, PH = 44, 28


def processo(k):
    cor = CORES[k]
    im = Image.new("RGBA", (PW, PH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1, 1, PW - 2, PH - 2], radius=6, fill=cor + (255,), outline=escuro(cor), width=2)
    # Os "pinos" do processo na CPU: tres riscos embaixo.
    for i in range(3):
        d.line([(12 + 10 * i, PH - 6), (12 + 10 * i, PH - 3)], fill=escuro(cor), width=2)
    centrado(d, f"P{k + 1}", PW / 2, PH / 2 - 2, fonte(14, True), TRACO)
    return im


TW, TH = 96, 22


def titulo(texto):
    im = Image.new("RGBA", (TW, TH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1, 1, TW - 2, TH - 2], radius=5, fill=(34, 42, 66, 230), outline=BORDA, width=1)
    centrado(d, texto, TW / 2, TH / 2, fonte(12, True), TINTA)
    return im


SW = 16


def tiro(tipo):
    im = Image.new("RGBA", (SW, SW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if tipo == "Instrucao":
        d.polygon([(8, 1), (15, 8), (8, 15), (1, 8)], fill=(110, 220, 255, 255), outline=(20, 60, 90, 255))
        d.rectangle([6, 6, 9, 9], fill=(240, 252, 255, 255))
    elif tipo == "Sinal":
        d.ellipse([1, 1, 14, 14], fill=(255, 190, 80, 255), outline=(110, 60, 10, 255), width=2)
        d.ellipse([5, 5, 10, 10], fill=(60, 30, 10, 255))
    elif tipo == "Bloco":
        d.rectangle([2, 3, 13, 12], fill=(120, 160, 255, 255), outline=(30, 40, 90, 255), width=1)
        d.line([(4, 6), (11, 6)], fill=(230, 240, 255, 255))
        d.line([(4, 9), (9, 9)], fill=(230, 240, 255, 255))
    else:  # Pacote
        d.rounded_rectangle([2, 2, 13, 13], radius=3, fill=(150, 255, 170, 255), outline=(20, 80, 40, 255), width=1)
        d.line([(5, 8), (10, 8)], fill=(20, 80, 40, 255), width=2)
    return im


TIROS = ["Instrucao", "Sinal", "Bloco", "Pacote"]

# ---- A fase 2: a paginacao -----------------------------------------------------

MW, MH = 60, 34


def moldura(pagina, r):
    im = Image.new("RGBA", (MW, MH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if pagina is None:
        for x in range(2, MW - 2, 6):   # tracejada: a moldura livre
            d.line([(x, 2), (x + 3, 2)], fill=(150, 170, 210, 200))
            d.line([(x, MH - 3), (x + 3, MH - 3)], fill=(150, 170, 210, 200))
        for y in range(2, MH - 2, 6):
            d.line([(2, y), (2, y + 3)], fill=(150, 170, 210, 200))
            d.line([(MW - 3, y), (MW - 3, y + 3)], fill=(150, 170, 210, 200))
        centrado(d, "livre", MW / 2, MH / 2, fonte(11), (170, 185, 220, 220))
        return im
    cor = (110, 200, 255) if r else (70, 110, 160)
    d.rounded_rectangle([1, 1, MW - 2, MH - 2], radius=5, fill=escuro(cor, 0.55), outline=cor + (255,), width=2)
    centrado(d, f"pág {pagina}", MW / 2 - 6, MH / 2, fonte(13, True), TINTA)
    # O bit R: aceso (amarelo) ou apagado.
    d.ellipse([MW - 16, MH / 2 - 5, MW - 6, MH / 2 + 5], fill=(255, 220, 90, 255) if r else (60, 70, 90, 255),
              outline=TRACO, width=1)
    centrado(d, "R", MW - 11, MH / 2, fonte(8, True), TRACO)
    return im


EW, EH = 52, 28


def entrada(pagina, valida):
    im = Image.new("RGBA", (EW, EH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([1, 1, EW - 2, EH - 2], fill=(26, 32, 52, 235), outline=(120, 150, 210, 255), width=1)
    d.line([(32, 2), (32, EH - 3)], fill=(120, 150, 210, 255))
    centrado(d, str(pagina), 16, EH / 2, fonte(13, True), TINTA)
    centrado(d, "1" if valida else "0", 42, EH / 2, fonte(13, True), VERDE if valida else (255, 110, 100, 255))
    return im


GW, GH = 22, 26


def pagina(n):
    im = Image.new("RGBA", (GW, GH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(2, 2), (14, 2), (20, 8), (20, 24), (2, 24)], fill=(236, 240, 250, 255), outline=(60, 70, 100, 255))
    d.polygon([(14, 2), (14, 8), (20, 8)], fill=(190, 200, 220, 255), outline=(60, 70, 100, 255))
    centrado(d, str(n), 11, 15, fonte(12, True), (30, 40, 80, 255))
    return im


# ---- A fase 3: o disco ---------------------------------------------------------

DW = 26


def pedido(estado):
    cores = {"Novo": (120, 200, 255), "Velho": (255, 200, 90), "Faminto": (255, 90, 80)}
    c = cores[estado]
    im = Image.new("RGBA", (DW, DW), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([2, 2, DW - 3, DW - 3], fill=escuro(c, 0.5), outline=c + (255,), width=2)
    d.pieslice([6, 6, DW - 7, DW - 7], start=-90, end=30, fill=c + (255,))   # o setor do disco
    d.ellipse([DW / 2 - 2, DW / 2 - 2, DW / 2 + 2, DW / 2 + 2], fill=TRACO)
    if estado == "Faminto":
        d.line([(8, 8), (11, 10)], fill=(255, 255, 255, 255), width=2)
        d.line([(DW - 9, 8), (DW - 12, 10)], fill=(255, 255, 255, 255), width=2)
    return im


# ---- A repescagem: a nuvem -----------------------------------------------------

VW, VH = 44, 36


def vm(k):
    cor = CORES[k]
    im = Image.new("RGBA", (VW, VH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([2, 2, VW - 3, VH - 9], radius=4, fill=(14, 18, 30, 255), outline=cor + (255,), width=2)
    d.rectangle([17, VH - 9, 26, VH - 5], fill=cor + (255,))
    d.rectangle([11, VH - 5, 32, VH - 3], fill=cor + (255,))
    centrado(d, f"VM{k + 1}", VW / 2, (VH - 9) / 2 + 1, fonte(11, True), cor + (255,))
    return im


def trap():
    im = Image.new("RGBA", (18, 18), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.polygon([(9, 1), (17, 16), (1, 16)], fill=(255, 110, 80, 255), outline=(90, 20, 10, 255))
    d.line([(9, 6), (9, 11)], fill=(255, 255, 255, 255), width=2)
    d.point((9, 13), fill=(255, 255, 255, 255))
    return im


# ---- Gravar --------------------------------------------------------------------

def tira(quadros, w, h, colunas):
    linhas = (len(quadros) + colunas - 1) // colunas
    f = Image.new("RGBA", (w * colunas, h * linhas), (0, 0, 0, 0))
    for i, im in enumerate(quadros):
        f.alpha_composite(im, ((i % colunas) * w, (i // colunas) * h))
    return f


def tira_json(nome, f, w, h, n):
    colunas = f.width // w
    frames = []
    for i in range(n):
        x, y = (i % colunas) * w, (i // colunas) * h
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": w, "h": h},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
            "sourceSize": {"w": w, "h": h},
            "duration": 120,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_goulart.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": f.width, "h": f.height}, "scale": "1"}}


def grava_tira(quadros, nome, w, h, colunas):
    f = tira(quadros, w, h, colunas)
    destino = os.path.join(RAIZ, "Assets/Teachers/Projectiles")
    f.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as arq:
        json.dump(tira_json(nome, f, w, h, len(quadros)), arq, indent=1)
    print(f"  Assets/Teachers/Projectiles/{nome}.png  {f.width}x{f.height}  ({len(quadros)} quadro(s))")


def grava(im, nome, pasta):
    destino = os.path.join(RAIZ, pasta)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}")


MOLDURAS = [("Livre", None, False)] + [(f"P{p}", p, False) for p in range(8)] + [(f"R{p}", p, True) for p in range(8)]
ENTRADAS = [(f"V{p}", p, True) for p in range(8)] + [(f"I{p}", p, False) for p in range(8)]
PEDIDOS = ["Novo", "Velho", "Faminto"]


def animacoes():
    return {
        "Processo": {f"P{k + 1}": [k] for k in range(9)},
        "Titulo": {"Prontos": [0], "ES": [1]},
        "Tiro": {nome: [i] for i, nome in enumerate(TIROS)},
        "Moldura": {nome: [i] for i, (nome, _, _) in enumerate(MOLDURAS)},
        "Entrada": {nome: [i] for i, (nome, _, _) in enumerate(ENTRADAS)},
        "Pagina": {f"Pg{p}": [p] for p in range(8)},
        "Pedido": {nome: [i] for i, nome in enumerate(PEDIDOS)},
        "Vm": {f"Vm{k + 1}": [k] for k in range(4)},
    }


if __name__ == "__main__":
    if "--animacoes" in sys.argv:
        print(json.dumps(animacoes(), ensure_ascii=False))
        sys.exit(0)
    grava(goulart(), "DPIBHGoulart", "Assets/Teachers")
    grava_tira([processo(k) for k in range(9)], "DPIBHGoulartProcessos", PW, PH, 9)
    grava_tira([titulo("PRONTOS"), titulo("E/S")], "DPIBHGoulartTitulos", TW, TH, 2)
    grava_tira([tiro(t) for t in TIROS], "DPIBHGoulartTiros", SW, SW, len(TIROS))
    grava_tira([moldura(p, r) for _, p, r in MOLDURAS], "DPIBHGoulartMolduras", MW, MH, 9)
    grava_tira([entrada(p, v) for _, p, v in ENTRADAS], "DPIBHGoulartEntradas", EW, EH, 8)
    grava_tira([pagina(p) for p in range(8)], "DPIBHGoulartPaginas", GW, GH, 8)
    grava_tira([pedido(e) for e in PEDIDOS], "DPIBHGoulartPedidos", DW, DW, 3)
    grava_tira([vm(k) for k in range(4)], "DPIBHGoulartVms", VW, VH, 4)
    grava_tira([trap()], "DPIBHGoulartTrap", 18, 18, 1)
