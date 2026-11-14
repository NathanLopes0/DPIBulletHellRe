#!/usr/bin/env bash
# A LINHA DE BASE: 79 batalhas que nao podem mudar sem querer.
#
# Roda o jogo sozinho (o modo de simulacao, Source/Simulacao/Simulacao.h) nas dezesseis
# materias, nos dois modos e nas duas dificuldades, e no TCC com cada um dos quinze
# orientadores - sempre com a mesma semente, o mesmo passo de tempo e o mesmo piloto -
# e compara a nota quadro a quadro com a gravada em Ferramentas/linha-de-base/.
#
#   bash Ferramentas/linha-de-base.sh                      # compara
#   bash Ferramentas/linha-de-base.sh --atualizar          # grava a de agora como a linha de base
#   JOGO=../build/DPIBulletHellRe bash Ferramentas/linha-de-base.sh
#
# IDENTICAS, BYTE A BYTE, e o que uma refatoracao tem de dar. Uma mudanca de
# balanceamento muda algumas de proposito: confira QUAIS mudaram e quanto (o diff
# mostra a linha FIM), e so entao grave com --atualizar, no mesmo commit da mudanca.
#
# JOGO     o executavel (padrao: build/DPIBulletHellRe, ou cmake-build-debug/...)
# PARALELO quantas batalhas ao mesmo tempo (padrao: 3; cada uma usa uns 2,5 GB)
# SAIDA    onde ficam as saidas desta rodada (padrao: uma pasta temporaria)
#
# Cada batalha: --semente B*7+3 (11 no TCC), --passo 0.016, o piloto que segue o
# chefe, atirando sempre e com o poder do orientador solto sozinho. O piloto e a
# semente estao explicados em build-e-testes.md.

set -uo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
BASE="$RAIZ/Ferramentas/linha-de-base"
atualizar=0; [ "${1:-}" = "--atualizar" ] && atualizar=1

JOGO="${JOGO:-}"
if [ -z "$JOGO" ]; then
    for c in "$RAIZ/build/DPIBulletHellRe" "$RAIZ/cmake-build-debug/DPIBulletHellRe" "$RAIZ/../build/DPIBulletHellRe"; do
        [ -x "$c" ] && { JOGO="$c"; break; }
    done
fi
[ -x "${JOGO:-}" ] || { echo "!!! nao achei o executavel; diga qual com JOGO=..."; exit 2; }
JOGO="$(cd "$(dirname "$JOGO")" && pwd)/$(basename "$JOGO")"

SAIDA="${SAIDA:-$(mktemp -d)}"; mkdir -p "$SAIDA"
PARALELO="${PARALELO:-3}"

# A lista: "nome parametros...", uma batalha por linha. NENHUMA LINHA PODE TERMINAR EM
# ESPACO: para o xargs -L, o espaco no fim emenda a linha com a seguinte, e as duas
# batalhas viram uma so (com os parametros da ultima).
lista() {
    for B in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do for M in N D; do for J in 1 2; do
        dificil=""; [ $M = D ] && dificil=" --dificil"
        echo "b${B}_${M}_${J} --batalha $B --jogadores $J --semente $((B*7+3)) --orientador INF213${dificil}"
    done; done; done
    for O in INF110 INF221 INF220 INF250 INF331 INF420 INF340 INF321 INF310 INF213 INF332 INF451 INF425 INF429 INF491; do
        echo "tcc_${O} --batalha 10 --jogadores 1 --semente 11 --orientador $O"
    done
}

rodar() {  # nome parametros...
    local nome=$1; shift
    ( cd "$RAIZ" && SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy timeout 900 "$JOGO" "$@" \
        --passo 0.016 --piloto segue --atira --poder --relatorio --sair-no-fim > "$SAIDA/$nome.log" 2>/dev/null )
    # Para arquivo, e so depois o filtro: se o timeout matar o jogo, nada se perde num pipe.
    grep -E "^q|^FIM" "$SAIDA/$nome.log" | tr -d '\r' > "$SAIDA/$nome.txt"; rm -f "$SAIDA/$nome.log"
}
export -f rodar; export JOGO SAIDA RAIZ

echo "rodando 79 batalhas com $JOGO ($PARALELO por vez) em $SAIDA..."
lista | xargs -P "$PARALELO" -L 1 bash -c 'rodar "$@"' _

sem_fim=0
for f in "$SAIDA"/*.txt; do grep -q '^FIM' "$f" || { echo "!!! sem FIM: $(basename "$f" .txt)"; sem_fim=$((sem_fim+1)); }; done

if [ $atualizar = 1 ]; then
    [ $sem_fim = 0 ] || { echo "!!! $sem_fim batalha(s) sem FIM: a linha de base nao foi gravada"; exit 1; }
    mkdir -p "$BASE" && cp "$SAIDA"/*.txt "$BASE/"
    echo "linha de base gravada em $BASE ($(ls "$BASE" | wc -l) batalhas)"
    exit 0
fi

# Sem o \r: o .gitattributes poe os .txt com CRLF no checkout, e o jogo escreve LF (no Linux).
iguais=0; diferentes=0
for f in "$BASE"/*.txt; do
    n=$(basename "$f")
    if cmp -s <(tr -d '\r' < "$f") "$SAIDA/$n"; then iguais=$((iguais+1))
    else
        diferentes=$((diferentes+1))
        echo "--- difere: ${n%.txt}"
        echo "    antes: $(grep '^FIM' "$f" | tr -d '\r')"
        echo "    agora: $(grep '^FIM' "$SAIDA/$n" 2>/dev/null)"
    fi
done
echo "identicas=$iguais diferentes=$diferentes (saidas em $SAIDA)"
[ $diferentes = 0 ] && [ $sem_fim = 0 ]
