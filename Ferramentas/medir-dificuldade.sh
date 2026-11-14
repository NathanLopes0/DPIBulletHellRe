#!/usr/bin/env bash
# MEDE UMA BATALHA: o jogo rodando sozinho, sem tela, ate a luta se decidir.
#
#   bash Ferramentas/medir-dificuldade.sh --batalha 0 --jogadores 2 --semente 1234 --piloto segue
#
# Os parametros vao direto para o jogo (o modo de simulacao, Source/Simulacao/Simulacao.h),
# e este script so acrescenta o que toda medida usa: --passo 0.016 --atira --relatorio
# --sair-no-fim, sem video e sem som. Sai a nota a cada 60 quadros de luta e o fim:
#
#   q00060 nota=40.0000 sofridos=0
#   FIM nota=88.7911 acertos_sofridos=10 acertos_dados=508 aprovado=1
#
# Antes, este script copiava o repositorio e remendava a copia com ganchos de variavel de
# ambiente; os ganchos agora moram no jogo, e o executavel que se mede e o que se joga.
#
# JOGO  o executavel (padrao: build/DPIBulletHellRe, ou cmake-build-debug/...)
#
# --batalha     o indice em materias.json (0 e INF110) ou o codigo ("INF213")
# --jogadores   1 ou 2
# --semente     o sorteio. Sem --variacao, e tambem a semente da batalha
#               (Battle::SementeDaBatalha), de onde cada fase tira o que varia: uma
#               corrida e uma funcao de um numero so
# --variacao    a semente da batalha, separada da do sorteio
# --dificil     o modo Dificil (Dificuldade.h). Sem ele, Normal - SEMPRE: a tela de
#               dificuldade nao aparece numa medida
# --orientador  o codigo da materia do orientador do TCC (INF213, ...): a batalha
#               que pede orientacao nasce com ele, como se tivesse sido escolhido
# --poder       no TCC, o orientador solta o poder sozinho quando ele serve
# --estado      a fase em que o chefe comeca (StateTwo, StateFinal, ...): mede uma
#               fase sozinha. Toda batalha comeca com a nota inicial (40), e a
#               StateThree a decide - a StateFinal sozinha mede quem chega a ela com
#               40, o pior caso de quem a ve
# --piloto      segue, parado, vaivem ou esquiva (abaixo)
#
# USE VARIAS SEMENTES. Com dois jogadores ha mais colisoes, e cada colisao consome
# numeros do mesmo sorteador global: a sequencia de ataques diverge da do solo depois
# do primeiro acerto. Uma corrida de cada mede o sorteio daquela corrida, e nao a
# dificuldade.
#
# OS PILOTOS, e o vies de cada um:
#
# segue   casa o x do aluno com o do chefe (no TCC, com o Hugo blindado, vai ao alvo do
#         objetivo e ao lugar que a etapa pede). Como o tiro sobe RETO, isso lhe da MIRA
#         PERFEITA: ele acerta quase todo tiro, a nota so sobe e o numero final nao diz
#         muito sobre a fase. E contra um ataque que desce em coluna sobre o chefe ele
#         caminha para dentro da coluna, fazendo a fase parecer mais dura do que e. E o
#         melhor para comparar os dois modos: os dois miram igual, e o que sobra para
#         medir e o que a regra 2.1 faz (ganho e perda divididos pelo numero de alunos).
# parado  o vies oposto: mira perfeita se o chefe passar por cima, e nenhuma se nao
#         passar. NAO SERVE PARA COMPARAR OS DOIS MODOS: o aluno solo nasce no meio,
#         embaixo do chefe, e acerta de graca; os dois da dupla nascem a 1/3 e 2/3 da
#         largura e nao acertam nada - a medida vira a posicao de nascimento. Serve para
#         o portao do parado-reprova.sh.
# vaivem  varre a largura jogavel ignorando o chefe. A mira dele e ruim de um jeito mais
#         parecido com o de gente, e ele nunca fica parado. Leia dele os ACERTOS
#         SOFRIDOS, e nao a nota: sem mira, a nota termina em zero em qualquer chefe.
# esquiva o perfil das optativas: quer o mesmo lugar do segue, mas a cada quadro olha os
#         tiros que machucam (meio segundo adiante, cada um em linha reta) e escolhe o rumo
#         mais seguro entre nove. E o teto de quem so desvia: ve tudo e reage na hora, e
#         ganha o bonus de fase limpa quase sempre - e por isso pontua acima de gente. Nao
#         entende as mecanicas (nao para de atirar na convolucao do Michel, nao se esconde
#         do detector): o que ele perde para o segue, perde por desviar; o que uma pessoa
#         perde para ele, perde por nao ver tudo ou por nao entender a fase.

set -uo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
JOGO="${JOGO:-}"
if [ -z "$JOGO" ]; then
    for c in "$RAIZ/build/DPIBulletHellRe" "$RAIZ/cmake-build-debug/DPIBulletHellRe" "$RAIZ/../build/DPIBulletHellRe"; do
        [ -x "$c" ] && { JOGO="$c"; break; }
    done
fi
[ -x "${JOGO:-}" ] || { echo "!!! nao achei o executavel; diga qual com JOGO=..."; exit 2; }

saida="$(mktemp)"
# Para arquivo, e so depois o filtro: se o timeout matar o jogo, nada se perde num pipe.
( cd "$RAIZ" && SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy timeout 900 "$JOGO" \
      --passo 0.016 --atira --relatorio --sair-no-fim "$@" > "$saida" 2>/dev/null )
grep -E "^q|^FIM" "$saida"
rm -f "$saida"
