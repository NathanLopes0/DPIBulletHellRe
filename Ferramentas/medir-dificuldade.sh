#!/usr/bin/env bash
# MEDE SE O CO-OP E "LEVEMENTE MAIS FACIL" (requisitos-coop.md, secao 8).
#
# Monta uma COPIA da arvore com tudo preso - semente escolhida, passo de tempo
# fixo, tiro automatico e um piloto no lugar do jogador - e roda a batalha ate o
# fim, imprimindo a nota, os acertos dados no chefe e os sofridos.
#
#   bash Ferramentas/medir-dificuldade.sh            # monta o arnes
#   cd "${TMPDIR:-/tmp}/dpi-medir"
#   DPI_SEMENTE=1234 DPI_BATALHA=0 DPI_JOGADORES=2 DPI_PASSO=0.016 \
#     DPI_SEGUE=1 DPI_ATIRA=1 SDL_VIDEODRIVER=offscreen SDL_AUDIODRIVER=dummy \
#     ../bdpi-medir/DPIBulletHellRe | grep -E "^q|^FIM"
#
# DPI_JOGADORES  1 ou 2        DPI_SEGUE   o jogador persegue o X do chefe
# DPI_SEMENTE    o sorteio     DPI_PARADO  o jogador nao sai do lugar
# DPI_BATALHA    a fase        DPI_VAIVEM  o jogador varre a largura, sem mirar
#                              DPI_ATIRA   atira sempre que puder
# DPI_ORIENTADOR o codigo da materia do orientador do TCC (INF213, ...): a
#                batalha que pede orientacao nasce com ele, como se tivesse
#                sido escolhido na tela de escolha
# DPI_PODER      no TCC, o orientador solta o poder sozinho quando ele enche
# DPI_DIFICIL    a batalha no modo Dificil (Dificuldade.h). Sem ela, Normal -
#                SEMPRE (a tela de dificuldade nao aparece numa medida)
# DPI_VARIACAO   a semente da batalha (Battle::SementeDaBatalha), de onde cada
#                fase tira o que varia de uma luta para a outra. Sem ela, e a
#                DPI_SEMENTE: uma corrida continua sendo uma funcao de um numero
#                so, e duas sementes diferentes variam as duas coisas juntas
# DPI_ESTADO     a fase em que o chefe comeca (StateTwo, StateFinal, ...): mede
#                ou fotografa uma fase sozinha. Toda batalha comeca com a nota
#                inicial (40), e StateThree a decide - a StateFinal sozinha mede
#                quem chega a ela com 40, o pior caso de quem a ve
#
# USE VARIAS SEMENTES. Com dois jogadores ha mais colisoes, e cada colisao
# consome numeros do mesmo sorteador global: a sequencia de ataques diverge da
# do solo depois do primeiro acerto. Uma corrida de cada mede o sorteio daquela
# corrida, e nao a dificuldade.
#
# E PREFIRA DPI_SEGUE A DPI_PARADO para comparar os dois modos. Ver a secao 8.1:
# parado, o jogador solo nasce embaixo do chefe e acerta de graca, e os dois da
# dupla nascem nos lados e nao acertam nada - a medida vira a posicao de
# nascimento.

set -uo pipefail
S="$(cd "$(dirname "$0")/.." && pwd)"           # a raiz do repositorio
arvore="${1:-$S}"; fase="${2:-0}"
alvo="${TMPDIR:-/tmp}/dpi-medir"
rm -rf "$alvo" && cp -r "$arvore" "$alvo" && cd "$alvo"

python3 -I - <<'PY'
import io, sys
def edit(arq, de, para, vezes=1):
    cru = io.open(arq, encoding='utf-8', newline='', errors='surrogateescape').read()
    crlf = cru.count('\r\n') > cru.count('\n') - cru.count('\r\n')
    t = cru.replace('\r\n', '\n')
    if t.count(de) != vezes: sys.exit("GANCHO %d em %s: %r" % (t.count(de), arq, de[:70]))
    t = t.replace(de, para, vezes)
    if crlf: t = t.replace('\n', '\r\n')
    io.open(arq, 'w', encoding='utf-8', newline='', errors='surrogateescape').write(t)

edit("Source/Game.cpp", '#include "Game.h"',
     '#include "Game.h"\n#include <cstdio>\n#include <cstdlib>')

# ---- SEMENTE FIXA. Sem isto cada corrida sorteia outros ataques e a
# ---- comparacao entre duas versoes nao quer dizer nada.
edit("Source/Game.cpp", "    Random::Init();",
"""    Random::Seed(std::getenv("DPI_SEMENTE")
                ? static_cast<unsigned>(std::atoi(std::getenv("DPI_SEMENTE"))) : 20261014u);""")

# ---- entra direto na batalha
edit("Source/Game.cpp", "    ChangeScene(Scene::SceneType::MainMenu);",
"""    if (std::getenv("DPI_BATALHA")) {
        mSelectedStage = std::atoi(std::getenv("DPI_BATALHA"));

        // AS SESSOES SAO REFEITAS pelo mesmo caminho do jogo. Mexer em
        // mSessoes na mao deixaria mModoDeJogo dizendo outra coisa, e a
        // batalha pergunta a Game::QuantosJogadores quantos Player criar.
        if (const char* n = std::getenv("DPI_JOGADORES")) {
            DefinirModoDeJogo(std::atoi(n) >= 2 ? Sessao::Modo::Dois : Sessao::Modo::Um);
        }
        if (const char* o = std::getenv("DPI_ORIENTADOR")) {
            DefinirOrientador(Materias::Carregadas().IndiceDe(o));
        }
        // O MODO E SEMPRE ESCRITO: a batalha medida nao passa pela tela de
        // dificuldade, e sem isto valeria o Normal de toda partida nova.
        DefinirDificuldade(std::getenv("DPI_DIFICIL") ? Dificuldade::Nivel::Dificil
                                                       : Dificuldade::Nivel::Normal);
        ChangeScene(Scene::SceneType::Battle);
    } else
    ChangeScene(Scene::SceneType::MainMenu);""")

# ---- DPI_ESTADO: o chefe comeca na fase pedida. A ancora e a mesma linha nas
# ---- onze fabricas; uma fabrica nova que a escreva de outro jeito derruba o
# ---- arnes aqui, em vez de comecar calada na fase 1.
import glob, os
for arq in sorted(glob.glob("Source/Actors/Teachers/BossFactory/*Factory.cpp")):
    fab = os.path.basename(arq)[:-4]
    if fab == "IBossFactory": continue
    edit(arq, 'boss->SetInitialState("StateOne");',
         'boss->SetInitialState(std::getenv("DPI_ESTADO") ? std::getenv("DPI_ESTADO") : "StateOne");')
    edit(arq, '#include "' + fab + '.h"', '#include "' + fab + '.h"\n#include <cstdlib>')

# ---- A SEMENTE DA BATALHA, presa: no jogo ela vem do random_device.
edit("Source/Scenes/Battle/Battle.cpp", "    mSemente = static_cast<std::uint32_t>(std::random_device{}());",
"""    mSemente = static_cast<std::uint32_t>(std::strtoul(
        std::getenv("DPI_VARIACAO") ? std::getenv("DPI_VARIACAO")
      : std::getenv("DPI_SEMENTE") ? std::getenv("DPI_SEMENTE") : "20261014", nullptr, 10));""")

# ---- passo de tempo fixo
edit("Source/Game.cpp", "        while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}",
     '        if (!std::getenv("DPI_PASSO")) while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16)) {}')
edit("Source/Game.cpp", "        mTicksCount = SDL_GetTicks();",
"""        mTicksCount = SDL_GetTicks();
        if (const char* p = std::getenv("DPI_PASSO")) deltaTime = static_cast<float>(std::atof(p));""")

# ---- O JOGADOR ATIRA SEMPRE E NAO ANDA. Parado, a corrida e identica entre
# ---- versoes; andando, qualquer diferenca de ordem viraria outra trajetoria e
# ---- a comparacao perderia o sentido.
# ---- O PILOTO: cada jogador persegue o X do chefe.
#
# PARADO NAO SERVE PARA COMPARAR OS DOIS MODOS. O tiro do jogador sobe RETO
# (Player::Shoot: velocidade (0, -800)), o chefe nasce no meio da tela, e o
# jogador solo nasce no meio tambem - entao parado ele acerta tudo. Os dois da
# dupla nascem a 1/3 e 2/3 da largura, e parados nao acertam NADA: medido, a
# nota da dupla ficou em 40,00 cravados por oito segundos enquanto a do solo
# subia a 48,96. Isso media a posicao de nascimento, e nao a dificuldade.
#
# Com o piloto, os dois modos miram igual e o que sobra para medir e o que a
# regra 2.1 faz: ganho e perda divididos pelo numero de jogadores.
# O TERCEIRO PILOTO: VAI E VEM. Existe por causa de um vies dos outros dois que
# so apareceu medindo a Maria Lucia.
#
# DPI_SEGUE casa o x do jogador com o x do chefe. Como o tiro sobe RETO, isso
# lhe da MIRA PERFEITA: ele acerta quase todo tiro, a nota so sobe e o numero
# final nao diz nada sobre a fase. E contra um ataque que desce em coluna sobre
# o chefe ele caminha para dentro da coluna, fazendo a fase parecer muito mais
# dura do que e.
#
# DPI_PARADO tem o vies oposto: mira perfeita se o chefe passar por cima, e
# nenhuma se nao passar.
#
# Este varre a largura jogavel ignorando o chefe. A mira dele e ruim de um jeito
# mais parecido com o de gente, e ele nunca fica parado - a unica coisa que um
# jogador de verdade com certeza nao faz. Leia dele os ACERTOS SOFRIDOS, e nao
# a nota: sem mira, a nota dele termina em zero em qualquer chefe.
edit("Source/Actors/Player/Player.cpp", "void Player::MoveInput(const Uint8 *keyState) {",
"""void Player::MoveInput(const Uint8 *keyState) {
    if (std::getenv("DPI_VAIVEM")) {
        static float dir = 1.0f;
        const float x = GetPosition().x;
        if (x > 1060.0f) dir = -1.0f;
        if (x <  140.0f) dir =  1.0f;
        GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(dir * VelocidadeDeAndar(), 0));
        mMoving = true;
        return;
    }
    if (std::getenv("DPI_SEGUE")) {
        // NO TCC, O OBJETIVO ANTES DO CHEFE: com o Hugo blindado, persegue o
        // alvo do objetivo (Hugo::ObjetivoAtual), e vai ate onde a etapa pede
        // que ele ande (Hugo::LugarDeIr) - em x e em y.
        static float casa = -1.0f;
        if (casa < 0.0f) casa = GetPosition().y;
        float alvo = GetPosition().x, alvoY = casa;
        if (const auto b = dynamic_cast<Battle*>(GetScene())) {
            if (const auto chefe = b->GetBoss()) alvo = chefe->GetPosition().x;
            if (auto* hugo = dynamic_cast<Hugo*>(b->GetBoss())) {
                if (auto o = hugo->ObjetivoAtual()) alvo = o->x;
                if (auto o = hugo->LugarDeIr()) { alvo = o->x; alvoY = o->y; }
            }
        }
        const float d = alvo - GetPosition().x, dy = alvoY - GetPosition().y;
        const float v = (d > 2.0f) ? VelocidadeDeAndar() : (d < -2.0f) ? -VelocidadeDeAndar() : 0.0f;
        const float vy = (dy > 2.0f) ? VelocidadeDeAndar() : (dy < -2.0f) ? -VelocidadeDeAndar() : 0.0f;
        GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(v, vy));
        mMoving = v != 0.0f;
        return;
    }
    if (std::getenv("DPI_PARADO")) { GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0,0)); mMoving = false; return; }""")
edit("Source/Actors/Player/Player.cpp", '#include "../../Scenes/Battle/Battle.h"',
     '#include "../../Scenes/Battle/Battle.h"\n#include "../Teachers/Boss.h"\n#include "../Teachers/Bosses/Hugo.h"')

# ---- DPI_PODER: o orientador solta o poder sozinho, assim que ele enche. Os
# ---- que atacam (Poder::Ataca) esperam o escudo do Hugo abrir; a bala de prata,
# ---- ao contrario, e para quando ele esta fechado.
edit("Source/Actors/Orientador/Orientador.cpp", "    Guiar(*batalha, deltaTime);\n",
"""    Guiar(*batalha, deltaTime);
    if (std::getenv("DPI_PODER") && PoderPronto() && batalha->GetBoss()) {
        const bool aberto = batalha->GetBoss()->AceitaDano();
        const bool bala = mPoder.tipo == Poder::Tipo::BalaDePrata;
        if (bala ? !aberto : (!Poder::Ataca(mPoder.tipo) || aberto)) SoltarPoder();
    }
""")
edit("Source/Actors/Orientador/Orientador.cpp", "#include <unordered_map>",
     "#include <unordered_map>\n#include <cstdlib>")
edit("Source/Actors/Player/Player.cpp", "#include <vector>",
     "#include <vector>\n#include <cstdlib>")
# O GANCHO E A ASSINATURA DA FUNCAO, e nao a condicao do if: a condicao ja
# mudou duas vezes (entrou o Painel, depois a posicao do jogador) e cada vez
# derrubou o arnes. A assinatura e o que menos muda.
edit("Source/Actors/Player/Player.cpp",
     "void Player::ShootInput(const Uint8 *keyState) {",
     "void Player::ShootInput(const Uint8 *keyState) {\n    if (std::getenv(\"DPI_ATIRA\") && mOverloadTimer <= 0.0f) { Shoot(); return; }")

# ---- o despejo: a nota a cada 60 quadros, e no fim
edit("Source/Scenes/Battle/Battle.cpp", "void Battle::FinishBattle(bool approved) {",
"""void Battle::FinishBattle(bool approved) {
    std::printf("FIM nota=%.4f acertos_sofridos=%d acertos_dados=%d aprovado=%d\\n",
                static_cast<double>(mGrade), mAcertosSofridos, gAcertosDados, approved ? 1 : 0);
    std::fflush(stdout);
    // SAI AQUI, antes de gravar: a medida e so da batalha, e deixar o jogo
    // seguir para a tela de resultado gastaria minutos por corrida e ainda
    // escreveria fichas de teste na pasta de saves.
    std::exit(0);""")
edit("Source/Scenes/Battle/Battle.cpp", '#include "Battle.h"',
     '#include "Battle.h"\n#include <cstdio>\n#include <cstdlib>\nstatic int gAcertosDados = 0;')

# ---- QUANTOS TIROS CHEGARAM NO CHEFE. E o numero que explica tudo o mais:
# ---- o ganho e dividido pelo numero de jogadores, entao a dupla so empata com
# ---- o solo se acertar o DOBRO - e acertar o dobro depende de os dois
# ---- conseguirem mirar, que e o que a posicao decide.
edit("Source/Scenes/Battle/Battle.cpp",
     "                GradeUp(PorJogador(Nota::kGanhoPorAcerto));",
     "                ++gAcertosDados;\n                GradeUp(PorJogador(Nota::kGanhoPorAcerto));")
edit("Source/Scenes/Battle/Battle.cpp", "void Battle::CheckCollisions() {",
"""void Battle::CheckCollisions() {
    static int quadro = 0;
    if (++quadro % 60 == 0) {
        std::printf("q%05d nota=%.4f sofridos=%d\\n", quadro,
                    static_cast<double>(mGrade), mAcertosSofridos);
        std::fflush(stdout);
    }""")
print("patches ok")
PY

cd "$(dirname "$alvo")" && cmake -S "$alvo" -B "b$(basename "$alvo")" -G Ninja -DCMAKE_BUILD_TYPE=Debug >/dev/null 2>&1
cmake --build "b$(basename "$alvo")" --target DPIBulletHellRe -j8 >/tmp/pig.log 2>&1 || {
    echo "!!! nao compilou"; grep -m8 "error:" /tmp/pig.log; exit 1; }

echo "arnes pronto"
