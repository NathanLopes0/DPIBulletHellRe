#!/usr/bin/env bash
# "FICAR PARADO NAO DEVERIA APROVAR EM CHEFE NENHUM."
#
# Roda os nove chefes com um piloto que NAO SE MEXE e atira sem parar, e
# reprova o chefe que o deixar passar.
#
#   bash Ferramentas/parado-reprova.sh
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
# ESTADO NA ULTIMA MEDICAO: oito dos nove reprovam o piloto parado. O Salles
# ainda passa (nota 80,70, com 15 acertos sofridos e 309 dados), e a causa esta
# identificada: tres das quatro fases dele usam GoToCenter, entao ele para na
# coluna do meio e leva os tiros de graca. E o mesmo defeito que a primeira
# versao do Vladimir tinha, e a cura tambem e a mesma - dar um gesto a cada
# fase. O gesto candidato de cada uma esta no cabecalho de
# Assets/Attacks/salles/fases.json.

set -uo pipefail
S="$(cd "$(dirname "$0")/.." && pwd)"
arvore="${1:-$S}"

echo "montando o arnes..."
bash "$S/Ferramentas/medir-dificuldade.sh" "$arvore" >/dev/null 2>&1 || {
    echo "!!! o arnes de medicao nao compilou - rode medir-dificuldade.sh sozinho para ver o erro"
    exit 1; }

alvo="${TMPDIR:-/tmp}/dpi-medir"
bin="${TMPDIR:-/tmp}/bdpi-medir/DPIBulletHellRe"
[ -x "$bin" ] || { echo "!!! binario nao encontrado em $bin"; exit 1; }

saidas="${TMPDIR:-/tmp}/dpi-parado"
rm -rf "$saidas" && mkdir -p "$saidas"

# indice:nome - os indices sao a ordem das materias em Assets/materias.json, e
# e assim que Game::mSelectedStage as enxerga. Materia sem chefe fica de fora.
CHEFES="0:Andre 1:Salles 2:Ricardo 3:Thiago 5:Vladimir 6:Julio 7:MariaLucia 8:Lucas 9:Vitor"

for par in $CHEFES; do
  b="${par%%:*}"; nome="${par##*:}"
  ( cd "$alvo" && env DPI_SEMENTE=20261014 DPI_BATALHA=$b DPI_JOGADORES=1 \
        DPI_PASSO=0.016 DPI_PARADO=1 DPI_ATIRA=1 \
        SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy \
        timeout 900 "$bin" 2>/dev/null | grep -E '^FIM' > "$saidas/${nome}.txt" ) &
done
wait

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

echo
if [ "$falhas" -eq 0 ]; then
  echo "OK: o piloto parado reprova nos nove."
  exit 0
fi
echo "$falhas chefe(s) deixam o jogador parado aprovar - marcados com !!"
exit 1
