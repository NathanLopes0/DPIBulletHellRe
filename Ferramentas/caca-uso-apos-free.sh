#!/usr/bin/env bash
# RODA UMA BATALHA SOB AddressSanitizer, para pegar uso de memoria liberada.
#
# POR QUE ISTO EXISTE. A fase 3 do Vitor divide o chefe em dois corpos, e o
# segundo e destruido quando os dois se juntam. Cada Boss e DONO das fabricas
# de projetil dele, e todo projetil guarda um ponteiro cru para a fabrica que o
# criou - entao os tiros do segundo corpo sobreviviam ao dono e, ao sair da
# tela, chamavam Release() num pool ja liberado. O jogo FECHAVA ao passar da
# fase 3 para o exame, que e a unica saida em que a batalha continua depois da
# divisao.
#
# A suite normal nao alcanca isso: Battle precisa de SDL e nao entra em
# dpi_tests. Este arnes e a rede que sobrou.
#
#   bash Ferramentas/caca-uso-apos-free.sh            # monta o arnes
#   cd "${TMPDIR:-/tmp}/dpi-asan"
#   DPI_BATALHA=9 DPI_EXAME=1 DPI_QUADROS=4600 DPI_PARADO=1 DPI_ATIRA=1 \
#     DPI_PASSO=0.016 ASAN_OPTIONS=detect_leaks=0 \
#     SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy \
#     ../basan-dpi/DPIBulletHellRe
#
# DPI_BATALHA   o indice da materia em materias.json (9 = Vitor, o que divide)
# DPI_EXAME     forca o desfecho "vai para o exame" ao fim da fase 3
# DPI_QUADROS   teto de quadros - SEM ELE O ARNES NAO TERMINA (ver abaixo)
# DPI_PARADO    o jogador nao sai do lugar
# DPI_ATIRA     atira sempre que puder
# DPI_PASSO     passo de tempo fixo, para a corrida ser reproduzivel
#
# POR QUE FORCAR O DESFECHO em vez de mirar uma nota entre 40 e 59: o que esta
# sob teste e a TRANSICAO com o chefe dividido, nao a regra que escolhe o
# desfecho. Fazer o piloto terminar naquela faixa por acaso mediria a pontaria
# do piloto.
#
# POR QUE O TETO DE QUADROS: FinishBattle marca a batalha como encerrada, mas a
# maquina de estados continua trocando de fase e a tela de resultado espera uma
# tecla que nunca chega no driver offscreen. Sem o teto, o processo roda para
# sempre. Antes da correcao isso nao aparecia - o processo morria no crash.
#
# LEITURA DO RESULTADO: codigo de saida 0 e nenhuma linha "ERROR:
# AddressSanitizer" quer dizer limpo. Vale rodar tambem com DPI_EXAME ausente
# (o desfecho real) e em outras batalhas - um desfecho limpo nao prova os
# outros.

set -uo pipefail
S="$(cd "$(dirname "$0")/.." && pwd)"           # a raiz do repositorio
arvore="${1:-$S}"
alvo="${TMPDIR:-/tmp}/dpi-asan"
build="${TMPDIR:-/tmp}/basan-dpi"

rm -rf "$alvo" "$build" && cp -r "$arvore" "$alvo" && cd "$alvo"

python3 -I - <<'PY'
import io, sys

def edit(arq, de, para):
    cru = io.open(arq, encoding='utf-8', newline='', errors='surrogateescape').read()
    crlf = cru.count('\r\n') > cru.count('\n') - cru.count('\r\n')
    t = cru.replace('\r\n', '\n')
    if t.count(de) != 1:
        sys.exit("GANCHO %d em %s: %r" % (t.count(de), arq, de[:60]))
    t = t.replace(de, para, 1)
    if crlf:
        t = t.replace('\n', '\r\n')
    io.open(arq, 'w', encoding='utf-8', newline='', errors='surrogateescape').write(t)

edit("Source/Game.cpp", '#include "Game.h"',
     '#include "Game.h"\n#include <cstdio>\n#include <cstdlib>')
edit("Source/Game.cpp", "    Random::Init();", "    Random::Seed(20261014u);")
edit("Source/Game.cpp", "    ChangeScene(Scene::SceneType::MainMenu);",
"""    if (std::getenv("DPI_BATALHA")) {
        mSelectedStage = std::atoi(std::getenv("DPI_BATALHA"));
        ChangeScene(Scene::SceneType::Battle);
    } else
    ChangeScene(Scene::SceneType::MainMenu);""")
edit("Source/Game.cpp", "        while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}",
     '        if (!std::getenv("DPI_PASSO")) while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}')
edit("Source/Game.cpp", "        mTicksCount = SDL_GetTicks();",
"""        mTicksCount = SDL_GetTicks();
        if (const char* p = std::getenv("DPI_PASSO")) deltaTime = static_cast<float>(std::atof(p));
        if (const char* n = std::getenv("DPI_QUADROS")) {
            static int q = 0;
            if (++q >= std::atoi(n)) { std::puts("TETO"); std::fflush(stdout); mIsGameRunning = false; }
        }""")

edit("Source/Actors/Player/Player.cpp", "#include <vector>", "#include <vector>\n#include <cstdlib>")
edit("Source/Actors/Player/Player.cpp", "void Player::MoveInput(const Uint8 *keyState) {",
     "void Player::MoveInput(const Uint8 *keyState) {\n"
     "    if (std::getenv(\"DPI_PARADO\")) { GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0,0)); mMoving = false; return; }")
edit("Source/Actors/Player/Player.cpp", "void Player::ShootInput(const Uint8 *keyState) {",
     "void Player::ShootInput(const Uint8 *keyState) {\n"
     "    if (std::getenv(\"DPI_ATIRA\") && mOverloadTimer <= 0.0f) { Shoot(); return; }")

edit("Source/Actors/Teachers/BossAttackState.cpp", '#include "BossAttackState.h"',
     '#include "BossAttackState.h"\n#include <cstdlib>')
edit("Source/Actors/Teachers/BossAttackState.cpp",
     "            switch (Nota::AposTerceiraFase(battle->GetNotaAtual())) {",
     "            const auto desfecho = std::getenv(\"DPI_EXAME\")\n"
     "                ? Nota::Desfecho::VaiParaFinal\n"
     "                : Nota::AposTerceiraFase(battle->GetNotaAtual());\n"
     "            switch (desfecho) {")
print("  patches ok")
PY

cmake -S "$alvo" -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" >/dev/null 2>&1
cmake --build "$build" --target DPIBulletHellRe -j6 >"${TMPDIR:-/tmp}/dpi-asan.log" 2>&1 || {
    echo "!!! nao compilou - veja ${TMPDIR:-/tmp}/dpi-asan.log"
    grep -m5 "error:" "${TMPDIR:-/tmp}/dpi-asan.log"; exit 1; }
echo "arnes ASAN pronto em $alvo (binario em $build)"
