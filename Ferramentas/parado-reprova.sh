#!/usr/bin/env bash
# "FICAR PARADO NAO DEVERIA APROVAR EM CHEFE NENHUM."
#
# Roda os dez chefes com um piloto que NAO SE MEXE e atira sem parar, e
# reprova o chefe que o deixar passar.
#
#   bash Ferramentas/parado-reprova.sh            (JOGO=... para outro executavel)
#
# POR QUE ISTO E UM PORTAO E NAO UMA CURIOSIDADE. Um jogador parado no centro
# de baixo nao esta jogando: nao desvia, nao procura abertura, nao decide nada.
# Se a nota dele chega a 60, a batalha tem um buraco - ou existe um ponto do
# campo que os ataques nao alcancam em dezessete segundos, ou o chefe fica
# tempo demais na coluna de quem atira para cima. Nos dois casos o chefe
# ensina a coisa errada.
#
# O QUE CADA NUMERO QUER DIZER, se um chefe cair:
#
#   acertos_sofridos baixo   o campo tem um bolso seguro. Ache onde os ataques
#                            nao chegam - costuma ser a FAIXA DE BAIXO, porque
#                            aneis e leques nascem no chefe, que voa em cima.
#   acertos_dados alto       o chefe mora na coluna do meio. Quem atira para
#                            cima sem se mexer acerta de graca a luta inteira,
#                            e a nota sobe sem o jogador fazer nada.
#
# Foi assim que o Vitor caiu: o anel da exclusao mutua tinha raio de uns 240
# pixels e o centro dele voava 440 acima do jogador, entao a fase 2 inteira era
# CENARIO para quem ficasse embaixo - um acerto em dezessete segundos, e nota
# subindo 33 pontos no caminho.
#
# NAO E UM TESTE DA SUITE porque Battle precisa de SDL e nao entra em
# dpi_tests. E uma ferramenta de rodar a mao quando se mexe no balanceamento de
# qualquer chefe.
#
# ESTADO NA ULTIMA MEDICAO (commit 170, o Vitor refeito): os dez reprovam o
# piloto parado. O que chegou mais perto foi o Lucas (5,5, com 18 acertos
# sofridos e 135 dados), e os outros nove ficaram com zero. O Vitor, que era o
# mais perto (26,1 no 164), zera ainda na fase 2: os semaforos poem uma
# coluna em cima de quem nao sai do lugar e a acordam entre as tres
# primeiras, e o roteador da fase 2 mira nele. O Luizinho leva 18 acertos: o
# alocador da fase 3 poe bloco atras de bloco em cima de quem nao sai do lugar. O Salles passava por parar na coluna do meio em tres fases,
# e deixou de passar quando cada fase dele ganhou um gesto - a mesma cura da
# primeira versao do Vladimir. O Andre passou num estado intermediario (commit
# 155: a fase 2 nova, a fase 3 e o exame ainda os antigos, com GoToCenter);
# desde o 156 reprova o parado nas cinco sementes medidas.

set -uo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
JOGO="${JOGO:-}"
if [ -z "$JOGO" ]; then
    for c in "$RAIZ/build/DPIBulletHellRe" "$RAIZ/cmake-build-debug/DPIBulletHellRe" "$RAIZ/../build/DPIBulletHellRe"; do
        [ -x "$c" ] && { JOGO="$c"; break; }
    done
fi
[ -x "${JOGO:-}" ] || { echo "!!! nao achei o executavel; diga qual com JOGO=..."; exit 2; }
JOGO="$(cd "$(dirname "$JOGO")" && pwd)/$(basename "$JOGO")"

saidas="$(mktemp -d)"
PARALELO="${PARALELO:-3}"   # cada batalha usa uns 2,5 GB

# indice:nome - os indices sao a ordem das materias em Assets/materias.json, e
# e assim que Game::mSelectedStage as enxerga. Materia sem chefe fica de fora.
CHEFES="0:Andre 1:Salles 2:Ricardo 3:Thiago 4:Luizinho 5:Vladimir 6:Julio 7:MariaLucia 8:Lucas 9:Vitor"

rodar() {  # indice nome
    ( cd "$RAIZ" && SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy timeout 900 "$JOGO" \
          --batalha "$1" --jogadores 1 --semente 20261014 --passo 0.016 --piloto parado --atira \
          --relatorio --sair-no-fim > "$saidas/$2.log" 2>/dev/null )
    grep -E '^FIM' "$saidas/$2.log" > "$saidas/$2.txt"
}
export -f rodar; export JOGO RAIZ saidas
for par in $CHEFES; do echo "${par%%:*} ${par##*:}"; done | xargs -P "$PARALELO" -L 1 bash -c 'rodar "$@"' _

falhas=0
echo
printf '%-3s %-11s %s\n' '' CHEFE 'PILOTO PARADO'
for par in $CHEFES; do
  nome="${par##*:}"
  linha="$(cat "$saidas/${nome}.txt" 2>/dev/null)"
  marca='   '
  case "$linha" in *"aprovado=1"*) marca='!! '; falhas=$((falhas+1));; esac
  printf '%s %-11s %s\n' "$marca" "$nome" "${linha:-(nao terminou)}"
done
rm -rf "$saidas"

echo
if [ "$falhas" -eq 0 ]; then
  echo "OK: o piloto parado reprova nos dez."
  exit 0
fi
echo "$falhas chefe(s) deixam o jogador parado aprovar - marcados com !!"
exit 1
