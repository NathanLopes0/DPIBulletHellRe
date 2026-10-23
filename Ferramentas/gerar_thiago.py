#!/usr/bin/env python3
"""Arte PROVISORIA do Thiago (INF 220) e do projetil dele.

Segue o mesmo estilo de cartao que o DPIBHJulio.png ja usa no repositorio: e
deliberadamente um placeholder, para nao se passar por arte final. Quando o
sprite de verdade existir, basta trocar o PNG - o json de quadro continua valendo.
"""
from PIL import Image, ImageDraw
import json, os, sys

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

CARTAO   = (74, 94, 128, 255)
BORDA    = (150, 180, 220, 255)
TINTA    = (228, 238, 250, 255)
CELULA   = (122, 210, 180, 255)
CABECALHO= (92, 160, 200, 255)


def quadro_json(nome, w, h):
    return {
        "frames": [{
            "filename": f"{nome}.png 0.aseprite",
            "frame": {"x": 0, "y": 0, "w": w, "h": h},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": w, "h": h},
            "sourceSize": {"w": w, "h": h},
            "duration": 120,
        }],
        "meta": {"app": "gerar_thiago.py", "version": "1.0",
                 "image": f"{nome}.png", "format": "RGBA8888",
                 "size": {"w": w, "h": h}, "scale": "1"},
    }


def thiago():
    """O cartao do professor: uma tabela de 3x3 com a primeira linha realcada."""
    im = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([4, 4, 123, 123], radius=10, fill=CARTAO, outline=BORDA, width=2)

    # A tabela, com cabecalho - o icone que diz "banco de dados" sem escrever.
    x0, y0, cw, ch, cols, rows = 24, 26, 27, 16, 3, 3
    for r in range(rows):
        for c in range(cols):
            x, y = x0 + c * cw, y0 + r * ch
            d.rectangle([x, y, x + cw - 3, y + ch - 3],
                        fill=CABECALHO if r == 0 else CELULA,
                        outline=(40, 52, 70, 255))

    d.text((40, 92), "THIAGO", fill=TINTA)
    return im


def celula():
    """O projetil: uma celula de tabela, 16x16, com borda marcada."""
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, 15, 15], fill=CELULA, outline=(34, 44, 60, 255))
    d.rectangle([3, 3, 12, 12], outline=(255, 255, 255, 110))
    return im


def grava(im, nome, pasta):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    png = os.path.join(destino, nome + ".png")
    im.save(png)
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(quadro_json(nome, im.width, im.height), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}")


# ---------------------------------------------------------------------------
# A TABELA VISIVEL (Banco.h): o campo e a tabela "alunos", e o que esta escrito
# nas casas e o que as consultas leem.
# ---------------------------------------------------------------------------

CARLITO = os.path.join(RAIZ, "Assets", "Fonts", "Carlito-Regular.ttf")
PAPEL = (232, 238, 246, 210)
PAPEL_BORDA = (92, 112, 142, 230)
TINTA_DADO = (30, 40, 58, 255)
CAB = (58, 88, 138, 235)
CAB_BORDA = (150, 180, 220, 255)

# A MESMA ORDEM de Banco.h: os nomes e os cursos em ordem alfabetica, e as
# colunas na ordem da tabela.
NOMES = ["Ana", "Bia", "Caio", "Davi", "Eva", "Gil", "Iara", "Lia"]
CURSOS = ["FIS", "INF", "MAT"]
COLUNAS = ["mat", "nome", "curso", "nota", "ano"]

LARGO, ALTO = 48, 24


def chip(texto, fundo, borda, tinta, tamanho):
    """Uma casa da tabela: o valor escrito num cartao de planilha."""
    s = 4
    im = Image.new("RGBA", (LARGO * s, ALTO * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1 * s, 1 * s, (LARGO - 2) * s, (ALTO - 2) * s], radius=3 * s,
                        fill=fundo, outline=borda, width=1 * s)
    from PIL import ImageFont
    d.text((LARGO * s // 2, ALTO * s // 2 + 1 * s), texto, fill=tinta,
           font=ImageFont.truetype(CARLITO, tamanho * s), anchor="mm")
    return im.resize((LARGO, ALTO), Image.LANCZOS)


SUJO = (250, 206, 200, 225)
SUJO_BORDA = (196, 64, 52, 240)
TINTA_SUJA = (150, 24, 18, 255)


def dados():
    """Os quadros da folha, na ordem de Banco::Cara: 0 a 99 os numeros, depois
    os oito nomes, os tres cursos e os cinco cabecalhos - e, a partir do 116,
    os numeros SUJOS de 0 a 99: o valor escrito por uma transacao que ainda nao
    fez COMMIT (Transacao.h), em vermelho."""
    quadros = [chip(str(v), PAPEL, PAPEL_BORDA, TINTA_DADO, 17) for v in range(100)]
    quadros += [chip(n, PAPEL, PAPEL_BORDA, TINTA_DADO, 16) for n in NOMES]
    quadros += [chip(c, PAPEL, PAPEL_BORDA, TINTA_DADO, 16) for c in CURSOS]
    quadros += [chip(c, CAB, CAB_BORDA, (240, 246, 255, 255), 15) for c in COLUNAS]
    quadros += [chip(str(v), SUJO, SUJO_BORDA, TINTA_SUJA, 17) for v in range(100)]
    linhas = (len(quadros) + 3) // 4
    folha = Image.new("RGBA", (LARGO * 4, ALTO * linhas), (0, 0, 0, 0))
    for i, q in enumerate(quadros):
        folha.alpha_composite(q, ((i % 4) * LARGO, (i // 4) * ALTO))
    return folha, len(quadros)


def aviso():
    """O marcador da casa que vai ser lida: um quadro do tamanho da casa (60x46,
    na escala 4 vira 240x184), em dois tons para pulsar."""
    folha = Image.new("RGBA", (60 * 2, 46), (0, 0, 0, 0))
    d = ImageDraw.Draw(folha)
    for i, alfa in enumerate((40, 85)):
        x = i * 60
        d.rectangle([x + 1, 1, x + 58, 44], fill=(255, 206, 84, alfa), outline=(255, 214, 96, 220))
    return folha, 2


def escrita():
    """A escrita de uma transacao (Transacao.h): o lapis do UPDATE, que voa
    do professor ate a casa e fica la ate o COMMIT ou o ROLLBACK."""
    s = 4
    im = Image.new("RGBA", (24 * s, 24 * s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse([2 * s, 2 * s, 21 * s, 21 * s], fill=(214, 70, 56, 235), outline=(255, 214, 200, 255), width=2 * s)
    # o lapis, inclinado
    d.line([7 * s, 16 * s, 16 * s, 7 * s], fill=(255, 240, 230, 255), width=3 * s)
    d.polygon([(6 * s, 17 * s), (8 * s, 14 * s), (9 * s, 18 * s)], fill=(255, 240, 230, 255))
    return im.resize((24, 24), Image.LANCZOS), 1


def folha_json(nome, w, h, largo, alto, quadros):
    frames = []
    colunas = w // largo
    for i in range(quadros):
        x, y = (i % colunas) * largo, (i // colunas) * alto
        frames.append({
            "filename": f"{nome} {i}.placeholder",
            "frame": {"x": x, "y": y, "w": largo, "h": alto},
            "rotated": False, "trimmed": False,
            "spriteSourceSize": {"x": 0, "y": 0, "w": largo, "h": alto},
            "sourceSize": {"w": largo, "h": alto},
            "duration": 220,
        })
    return {"frames": frames,
            "meta": {"app": "gerar_thiago.py", "version": "1.0", "image": f"{nome}.png",
                     "format": "RGBA8888", "size": {"w": w, "h": h}, "scale": "1"}}


def grava_folha(im, nome, pasta, largo, alto, quadros):
    destino = os.path.join(RAIZ, pasta)
    os.makedirs(destino, exist_ok=True)
    im.save(os.path.join(destino, nome + ".png"))
    with open(os.path.join(destino, nome + ".json"), "w", encoding="utf-8") as f:
        json.dump(folha_json(nome, im.width, im.height, largo, alto, quadros), f, indent=1)
    print(f"  {pasta}/{nome}.png  {im.width}x{im.height}  ({quadros} quadro(s))")


if __name__ == "__main__":
    grava(thiago(), "DPIBHThiago", "Assets/Teachers")
    grava(celula(), "DPIBHThiagoCelula", "Assets/Teachers/Projectiles")
    folha, n = dados()
    grava_folha(folha, "DPIBHThiagoDados", "Assets/Teachers/Projectiles", LARGO, ALTO, n)
    folha, n = aviso()
    grava_folha(folha, "DPIBHThiagoAviso", "Assets/Teachers/Projectiles", 60, 46, n)
    folha, n = escrita()
    grava_folha(folha, "DPIBHThiagoEscrita", "Assets/Teachers/Projectiles", 24, 24, n)
