#!/usr/bin/env python3
"""O RELATORIO DAS TENTATIVAS: o que os alunos fizeram no gabinete, numa pagina so.

    python3 Ferramentas/relatorio_tentativas.py                 # le Saves/, escreve relatorio-tentativas.html
    python3 Ferramentas/relatorio_tentativas.py PASTA -o SAIDA.html
    python3 Ferramentas/relatorio_tentativas.py --desde 2026-11-01 --versao 4968d99

Le os arquivos que o jogo grava em Saves/ (Documentacao/dados-das-tentativas.md): tentativas.csv e
tentativas_fases.csv. So a biblioteca padrao: roda no Ubuntu do gabinete sem instalar nada.

O que ele responde:
  - por materia e dificuldade: quantos jogam, quantos passam, a nota, quanto tempo, e em qual tentativa
    o aluno passa pela primeira vez;
  - onde a nota cai: cada fase de cada materia, com a variacao media da nota, os acertos sofridos, as
    fases limpas e quantas tentativas terminaram nela - as fases mais duras primeiro;
  - a curva de aprendizado: a nota media na 1a, 2a, 3a... tentativa de cada aluno;
  - as medalhas: quantos alunos ja tem cada mencao e cada desafio, por materia (contados a partir das
    tentativas, com as regras de Mencoes.h);
  - quando o gabinete e usado: por dia da semana e por hora.

Uma partida em dupla tem duas linhas em tentativas.csv, uma por aluno, com o mesmo id: as contas de
partidas (aprovacao, nota, duracao, horario) contam cada id uma vez; as de alunos (quantos, em que
tentativa passou, a curva, as medalhas) contam cada linha. Os visitantes entram nas contas de partidas,
mas nao nas de alunos: varias pessoas jogam como visitante, e conta-las como um aluno so seria errado.

O que o CSV nao tem: qual projetil acertou. Os acertos sofridos estao por fase (tentativas_fases.csv),
e cada fase e um ataque do professor - entao a tabela "onde a nota cai" e o mais perto disso.
"""
import argparse
import csv
import html
import os
import statistics
import sys
from collections import Counter, defaultdict

VISITANTE = 'visitante'
DIAS = ['segunda', 'terca', 'quarta', 'quinta', 'sexta', 'sabado', 'domingo']


def numero(s, padrao=0.0):
    try:
        return float(s)
    except (TypeError, ValueError):
        return padrao


def ler_csv(caminho):
    if not os.path.exists(caminho):
        return []
    with open(caminho, encoding='utf-8', newline='') as f:
        return list(csv.DictReader(f))


def filtrar(linhas, desde=None, versao=None):
    saida = []
    for l in linhas:
        if desde and l.get('data', l.get('id', '')[:8]) < desde:
            continue
        if versao and l.get('versao_do_jogo') != versao:
            continue
        saida.append(l)
    return saida


def partidas(linhas):
    """Uma linha por id: a primeira (na dupla, as duas tem a mesma nota, o mesmo resultado e a mesma hora)."""
    vistas = {}
    for l in linhas:
        vistas.setdefault(l['id'], l)
    return list(vistas.values())


def media(xs):
    return statistics.mean(xs) if xs else None


def mediana(xs):
    return statistics.median(xs) if xs else None


def pct(a, b):
    return 100.0 * a / b if b else None


# ---- As contas ----

def por_materia(tent):
    """Uma linha por materia, dificuldade e modo (como tentativas_resumo.csv)."""
    grupos = defaultdict(list)
    for t in tent:
        grupos[(t['materia'], t['dificuldade'], t['modo'])].append(t)
    linhas = []
    for (materia, dif, modo), ts in sorted(grupos.items()):
        ps = partidas(ts)
        notas = [numero(t['nota_final']) for t in ps if t['resultado'] != 'abandonou']
        aprov = sum(1 for t in ps if t['aprovado'] == '1')
        aband = sum(1 for t in ps if t['resultado'] == 'abandonou')
        alunos = {t['matricula'] for t in ts if t['matricula'] != VISITANTE}
        linhas.append({
            'materia': materia, 'nome': ts[0].get('nome_da_materia', materia), 'dificuldade': dif,
            'modo': modo, 'partidas': len(ps), 'alunos': len(alunos), 'aprovacao': pct(aprov, len(ps) - aband),
            'abandono': pct(aband, len(ps)), 'nota_media': media(notas), 'nota_mediana': mediana(notas),
            'duracao': media([numero(t['duracao_s']) for t in ps]),
            'ate_aprovar': media(tentativas_ate_aprovar(ts)),
        })
    return linhas


def tentativas_ate_aprovar(ts):
    """Para cada aluno (sem visitantes): em que tentativa desta materia, dificuldade e modo ele passou."""
    por_aluno = defaultdict(list)
    for t in ts:
        if t['matricula'] != VISITANTE:
            por_aluno[t['matricula']].append(t)
    saida = []
    for lista in por_aluno.values():
        lista.sort(key=lambda t: t['id'])
        for k, t in enumerate(lista, 1):
            if t['aprovado'] == '1':
                saida.append(k)
                break
    return saida


def onde_a_nota_cai(fases, tent):
    """Cada fase de cada materia: a variacao da nota, os acertos, as limpas, e quantas acabaram nela."""
    grupos = defaultdict(list)
    for f in fases:
        grupos[(f['materia'], f['dificuldade'], f['fase'], f.get('anuncio', ''))].append(f)
    linhas = []
    for (materia, dif, fase, anuncio), fs in grupos.items():
        acabaram = sum(1 for f in fs if f.get('completa') == '0')
        linhas.append({
            'materia': materia, 'dificuldade': dif, 'fase': fase, 'anuncio': anuncio, 'vezes': len(fs),
            'variacao': media([numero(f['variacao']) for f in fs]),
            'acertos': media([numero(f['acertos_sofridos']) for f in fs]),
            'limpas': pct(sum(1 for f in fs if f.get('limpa') == '1'), len(fs)),
            'acabaram': pct(acabaram, len(fs)),
        })
    # As mais duras primeiro: a que mais tira nota.
    linhas.sort(key=lambda l: (l['variacao'] if l['variacao'] is not None else 0.0))
    return linhas


def curva_de_aprendizado(tent, ate=6):
    """A nota media (e a aprovacao) por numero da tentativa do aluno na materia (a coluna 'vez')."""
    grupos = defaultdict(list)
    for t in tent:
        vez = int(numero(t.get('vez'), 0))
        if vez <= 0 or t['resultado'] == 'abandonou':
            continue
        grupos[min(vez, ate)].append(t)
    linhas = []
    for vez in sorted(grupos):
        ts = grupos[vez]
        linhas.append({'vez': ('%d+' % vez) if vez == ate else str(vez), 'tentativas': len(ts),
                       'nota': media([numero(t['nota_final']) for t in ts]),
                       'aprovacao': pct(sum(1 for t in ts if t['aprovado'] == '1'), len(ts))})
    return linhas


MEDALHAS = [('B', 'aprovação'), ('P', 'nota 80'), ('O', 'nota 90'), ('100', 'nota cheia'), ('D', 'aprovado no Difícil'),
            ('2', 'aprovado em dupla'), ('Q', 'quietinho'), ('I', 'intocável'), ('A', 'prestando atenção'),
            ('F', 'sentar na frente'), ('100D', '100 no Difícil')]


def medalhas_da_tentativa(linhas_do_id):
    """As mencoes e os desafios que uma tentativa (as linhas de um id: uma no solo, duas na dupla) da.
    As regras sao as de Mencoes.h; o T (o orientador) depende do TCC e fica de fora."""
    t = linhas_do_id[0]
    if t['aprovado'] != '1':
        return set()
    nota = numero(t['nota_final'])
    dificil = t['dificuldade'] == 'dificil'
    dupla = t['modo'] == 'dupla'
    tiros = sum(numero(l['tiros']) for l in linhas_do_id)
    acertos = sum(numero(l['acertos_no_professor']) for l in linhas_do_id)
    de_perto = sum(numero(l.get('acertos_de_perto')) for l in linhas_do_id)
    sofridos = numero(t.get('acertos_sofridos_dupla', t['acertos_sofridos']))
    m = {'B'}
    if nota >= 80: m.add('P')
    if nota >= 90: m.add('O')
    if nota >= 100: m.add('100')
    if dificil: m.add('D')
    if dupla: m.add('2')
    if tiros == 0: m.add('Q')
    if sofridos == 0: m.add('I')
    if tiros >= 50 and acertos >= 0.7 * tiros: m.add('A')
    if acertos >= 50 and de_perto >= 0.5 * acertos: m.add('F')
    if dificil and nota >= 100: m.add('100D')
    return m


def medalhas(tent):
    """Por materia: quantos alunos (sem visitantes) ja tem cada medalha."""
    por_id = defaultdict(list)
    for t in tent:
        por_id[t['id']].append(t)
    tem = defaultdict(lambda: defaultdict(set))      # materia -> medalha -> alunos
    alunos = defaultdict(set)
    for linhas in por_id.values():
        ms = medalhas_da_tentativa(linhas)
        for l in linhas:
            if l['matricula'] == VISITANTE:
                continue
            alunos[l['materia']].add(l['matricula'])
            for m in ms:
                tem[l['materia']][m].add(l['matricula'])
    return [{'materia': mat, 'alunos': len(alunos[mat]), **{k: len(tem[mat][k]) for k, _ in MEDALHAS}}
            for mat in sorted(alunos)]


def uso(tent):
    ps = partidas(tent)
    dias = Counter(t.get('dia_da_semana', '') for t in ps)
    horas = Counter(int(numero(t.get('hora', '0')[:2])) for t in ps)
    return dias, horas


# ---- A pagina ----

def f1(x, sufixo=''):
    return '—' if x is None else ('%.1f%s' % (x, sufixo))


def barra(valor, maximo, cor='#5b8def'):
    if valor is None or valor <= 0 or not maximo:
        return ''
    w = max(0.0, min(100.0, 100.0 * valor / maximo))
    return '<span class="barra" style="width:%.0f%%;background:%s"></span>' % (w, cor)


def tabela(cabecalho, linhas):
    """O cabecalho e as celulas ja vem em HTML (quem chama escapa o que veio do CSV)."""
    h = ''.join('<th>%s</th>' % c for c in cabecalho)
    corpo = ''.join('<tr>%s</tr>' % ''.join('<td>%s</td>' % c for c in l) for l in linhas)
    return '<div class="rola"><table><thead><tr>%s</tr></thead><tbody>%s</tbody></table></div>' % (h, corpo)


def pagina(tent, fases, origem):
    alunos = {t['matricula'] for t in tent if t['matricula'] != VISITANTE}
    ps = partidas(tent)
    duplas = sum(1 for t in ps if t['modo'] == 'dupla')
    visitantes = sum(1 for t in tent if t['matricula'] == VISITANTE)
    aprov = sum(1 for t in ps if t['aprovado'] == '1')
    aband = sum(1 for t in ps if t['resultado'] == 'abandonou')
    horas = sum(numero(t['duracao_s']) for t in ps) / 3600.0
    datas = sorted(t['data'] for t in ps if t.get('data'))
    versoes = Counter(t.get('versao_do_jogo', '') for t in ps)
    partes = []
    partes.append('<h1>Relatório das tentativas</h1><p class="sub">%s</p>' % html.escape(origem))
    partes.append('<p class="numeros">%s</p>' % ' '.join(
        '<span><b>%s</b>%s</span>' % (v, html.escape(r)) for v, r in [
            (len(ps), 'partidas'), (duplas, 'em dupla'), (len(alunos), 'alunos'), (visitantes, 'jogadas de visitante'),
            (f1(pct(aprov, len(ps) - aband), '%'), 'aprovação'), (f1(pct(aband, len(ps)), '%'), 'abandono'),
            ('%.1f h' % horas, 'de luta')]))
    partes.append('<p class="sub">de %s a %s</p>' % (datas[0] if datas else '—', datas[-1] if datas else '—'))
    partes.append('<p class="sub">versões do jogo: %s</p>' % ', '.join('%s (%d)' % (html.escape(v or '?'), n)
                                                                        for v, n in versoes.most_common()))

    pm = por_materia(tent)
    partes.append('<h2>Por matéria</h2><p>A aprovação não conta os abandonos. “Até passar” é em que tentativa '
                  'o aluno passou pela primeira vez, nesta dificuldade e neste modo (a média dos que já passaram; '
                  'vazio enquanto ninguém identificado passou).</p>')
    partes.append(tabela(['Matéria', 'Dificuldade', 'Modo', 'Partidas', 'Alunos', 'Aprovação', '', 'Abandono', 'Nota média',
                          'Mediana', 'Duração', 'Até passar'],
                         [['%s <small>%s</small>' % (html.escape(l['materia']), html.escape(l['nome'])), l['dificuldade'],
                           l['modo'], l['partidas'], l['alunos'], f1(l['aprovacao'], '%'), barra(l['aprovacao'], 100, '#3fae6a'),
                           f1(l['abandono'], '%'), f1(l['nota_media']), f1(l['nota_mediana']), f1(l['duracao'], ' s'),
                           f1(l['ate_aprovar'])] for l in pm]))

    nc = onde_a_nota_cai(fases, tent)
    partes.append('<h2>Onde a nota cai</h2><p>Cada fase de cada matéria, as que mais tiram nota primeiro. '
                  '“Acabaram nela” é a fração das vezes em que a luta terminou no meio da fase (nota a zero ou '
                  'abandono). Cada fase é um ataque do professor: é o mais perto que os dados chegam de “qual '
                  'projétil acerta mais” (o jogo não grava qual projétil acertou). Com poucas vezes, o número ainda '
                  'não diz muito.</p>')
    pior = min((l['variacao'] for l in nc if l['variacao'] is not None), default=0.0)
    partes.append(tabela(['Matéria', 'Dificuldade', 'Fase', 'Vezes', 'Variação da nota', '', 'Acertos sofridos', 'Limpas',
                          'Acabaram nela'],
                         [[html.escape(l['materia']), l['dificuldade'], '%s <small>%s</small>' % (html.escape(l['fase']), html.escape(l['anuncio'])),
                           l['vezes'], f1(l['variacao']), barra(-l['variacao'] if l['variacao'] and l['variacao'] < 0 else 0, -pior or 1, '#d9534f'),
                           f1(l['acertos']), f1(l['limpas'], '%'), f1(l['acabaram'], '%')] for l in nc]))

    ca = curva_de_aprendizado(tent)
    partes.append('<h2>Curva de aprendizado</h2><p>A nota média na 1ª, 2ª, 3ª... tentativa de cada aluno numa matéria '
                  '(todas as matérias juntas, sem abandonos nem visitantes). Subindo devagar é o que se quer; um salto '
                  'grande da 1ª para a 2ª diz que a primeira vez ensina pouco.</p>')
    partes.append(tabela(['Tentativa', 'Tentativas', 'Nota média', '', 'Aprovação'],
                         [[l['vez'], l['tentativas'], f1(l['nota']), barra(l['nota'], 100), f1(l['aprovacao'], '%')] for l in ca]))

    md = medalhas(tent)
    partes.append('<h2>Medalhas</h2><p>Quantos alunos já têm cada menção e cada desafio, por matéria — contados das '
                  'tentativas, com as regras de <code>Mencoes.h</code> (o T, do orientador, fica de fora). Uma '
                  'que ninguém consegue talvez precise de ajuste.</p>')
    partes.append(tabela(['Matéria', 'Alunos'] + ['<span title="%s">%s</span>' % (html.escape(n), k) for k, n in MEDALHAS],
                         [[html.escape(l['materia']), l['alunos']] + [l[k] for k, _ in MEDALHAS] for l in md]))
    partes.append('<p class="sub">%s</p>' % ' · '.join('<b>%s</b> %s' % (k, html.escape(n)) for k, n in MEDALHAS))

    dias, hs = uso(tent)
    partes.append('<h2>Quando o gabinete é usado</h2>')
    md_dia = max(dias.values(), default=1)
    partes.append(tabela(['Dia', 'Partidas', ''], [[d, dias.get(d, 0), barra(dias.get(d, 0), md_dia)] for d in DIAS]))
    md_hora = max(hs.values(), default=1)
    partes.append(tabela(['Hora', 'Partidas', ''], [['%02dh' % h, hs[h], barra(hs[h], md_hora)] for h in sorted(hs)]))

    estilo = '''<style>
:root { --fundo: #ffffff; --texto: #1d2433; --fraco: #667085; --linha: #e4e7ec; --cabeca: #f2f4f7; }
@media (prefers-color-scheme: dark) { :root { --fundo: #12151c; --texto: #e6e8ee; --fraco: #98a2b3; --linha: #2b303b; --cabeca: #1b1f28; } }
body { background: var(--fundo); color: var(--texto); font: 15px/1.5 system-ui, sans-serif; max-width: 1100px; margin: 24px auto; padding: 0 16px; }
h1 { margin-bottom: 4px; } h2 { margin-top: 36px; } .sub, small { color: var(--fraco); } .sub { overflow-wrap: anywhere; }
.numeros { display: flex; flex-wrap: wrap; gap: 8px 24px; margin: 16px 0 4px; }
.numeros span { display: flex; flex-direction: column; color: var(--fraco); font-size: 13px; }
.numeros b { color: var(--texto); font-size: 24px; font-variant-numeric: tabular-nums; }
.rola { overflow-x: auto; margin: 8px 0 16px; }
table { border-collapse: collapse; width: 100%; font-variant-numeric: tabular-nums; }
th, td { border-bottom: 1px solid var(--linha); padding: 6px 8px; text-align: left; vertical-align: middle; }
th { background: var(--cabeca); font-weight: 600; } td:nth-child(n+3) { white-space: nowrap; }
.barra { display: inline-block; height: 10px; border-radius: 3px; min-width: 1px; vertical-align: middle; }
td:has(.barra) { width: 140px; }
</style>'''
    return ('<!doctype html><html lang="pt-BR"><head><meta charset="utf-8"><meta name="viewport" '
            'content="width=device-width, initial-scale=1"><title>Relatório das tentativas</title>%s</head><body>%s'
            '</body></html>') % (estilo, '\n'.join(partes))


def main():
    p = argparse.ArgumentParser(description='O relatorio das tentativas do gabinete.')
    p.add_argument('pasta', nargs='?', default='Saves', help='a pasta dos saves (padrao: Saves)')
    p.add_argument('-o', '--saida', default='relatorio-tentativas.html')
    p.add_argument('--desde', help='so as tentativas desta data em diante (AAAA-MM-DD)')
    p.add_argument('--versao', help='so as tentativas desta versao do jogo (o commit)')
    a = p.parse_args()
    tent = filtrar(ler_csv(os.path.join(a.pasta, 'tentativas.csv')), a.desde, a.versao)
    ids = {t['id'] for t in tent}
    fases = [f for f in ler_csv(os.path.join(a.pasta, 'tentativas_fases.csv')) if f['id'] in ids]
    if not tent:
        sys.exit('nenhuma tentativa em %s' % a.pasta)
    with open(a.saida, 'w', encoding='utf-8') as f:
        f.write(pagina(tent, fases, os.path.abspath(a.pasta)))
    resumo_no_terminal(tent, fases)
    print('\n%d partidas (%d linhas), %d fases -> %s' % (len(partidas(tent)), len(tent), len(fases), a.saida))


def resumo_no_terminal(tent, fases):
    """O essencial, para quem esta no gabinete sem navegador."""
    print('Aprovacao por materia (sem abandonos):')
    for l in por_materia(tent):
        print('  %-7s %-7s %-5s %3d partidas  aprovacao %6s  nota %5s  ate passar %4s' % (
            l['materia'], l['dificuldade'], l['modo'], l['partidas'], f1(l['aprovacao'], '%'), f1(l['nota_media']),
            f1(l['ate_aprovar'])))
    duras = [l for l in onde_a_nota_cai(fases, tent) if l['variacao'] is not None and l['variacao'] < 0][:5]
    if duras:
        print('As fases que mais tiram nota:')
        for l in duras:
            print('  %-7s %-10s %-34s %+6.1f  (%d vezes, %s acertos)' % (
                l['materia'], l['fase'], l['anuncio'][:34], l['variacao'], l['vezes'], f1(l['acertos'])))


if __name__ == '__main__':
    main()
