//
// Fábrica do Júlio (INF 420)
//

#include "JulioFactory.h"

#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Julio.h"
#include "BossProjectileFactory/JulioProjectile1Factory.h"

#include "../../../CaminhosArquivo.h"

JulioFactory::JulioFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> JulioFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Julio>(scene);
}

void JulioFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Teachers/DPIBHJulio.png"),
                                                              Caminhos::Asset("Teachers/DPIBHJulio.json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // ----- FÁBRICA DE PROJÉTEIS ----- //
    // Um tipo só. Cada tipo registrado custa mais 300 instâncias no
    // pré-aquecimento, então só adicione outro se ele for mesmo diferente.
    boss->AddProjectileFactory("Dados", std::make_unique<JulioProjectile1Factory>());
}

void JulioFactory::ConfigureAttacksAndFSM(Boss* boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Julio nao tem FSMComponent");
        return;
    }

    // As fases deste chefe vivem em Assets/Attacks/fases.json, no conjunto
    // "julio", e as regras dos projeteis em Assets/Attacks/regras.json.
    //
    // O que continua em C++ e so o que NAO e dado: a mira de cada fase, em
    // Julio::CustomizeAttackParams, que depende da posicao e da velocidade do
    // jogador no instante do disparo, e por isso muda a cada tiro.
    //
    // NAO HA MAIS CONFIGURACAO DE RESERVA EM C++. Se o arquivo faltar, estiver
    // malformado ou tiver uma transicao quebrada, ConfigurarFasesDeArquivo
    // devolve false depois de explicar o motivo no log, e este chefe fica SEM
    // ATAQUE NENHUM. Quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade e quebra a
    // suite ao primeiro erro de digitacao.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "julio")) {
        SDL_Log("ERRO CRITICO: o chefe \"julio\" nao pode ser montado a partir de "
                "Assets/Attacks/fases.json e ficara sem ataque. As linhas FASES: acima "
                "dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
