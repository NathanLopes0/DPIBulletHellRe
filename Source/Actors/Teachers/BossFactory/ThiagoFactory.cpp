//
// Fabrica do Thiago (INF 220)
//

#include "ThiagoFactory.h"

#include "../../../Attacks/Engine/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/Engine/ProjeteisDeChefeArquivo.h"
#include "../../../Core/CaminhosArquivo.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Thiago.h"

ThiagoFactory::ThiagoFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> ThiagoFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Thiago>(scene);
}

void ThiagoFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(Caminhos::Asset(Retrato() + ".png"),
                                                              Caminhos::Asset(Retrato() + ".json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // ----- FABRICA DE PROJETEIS ----- //
    // Os projeteis vem de Assets/Attacks/projeteis.json, conjunto "thiago".
    if (!RegistrarProjeteisDeArquivo(boss, "thiago")) {
        SDL_Log("ERRO CRITICO: o chefe \"thiago\" ficou sem projetil. As linhas PROJETEIS: "
                "acima dizem o motivo.");
    }
}

void ThiagoFactory::ConfigureAttacksAndFSM(Boss* boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Thiago nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/fases.json, conjunto "thiago". O que fica
    // em C++ e so o que nao e dado: QUAL faixa cada consulta varre, em
    // Thiago::CustomizeAttackParams, porque depende de onde o jogador esta no
    // instante do disparo.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "thiago")) {
        SDL_Log("ERRO CRITICO: o chefe \"thiago\" nao pode ser montado a partir de "
                "Assets/Attacks/fases.json e ficara sem ataque. As linhas FASES: acima "
                "dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
