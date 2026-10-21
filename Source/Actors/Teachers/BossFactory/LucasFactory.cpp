//
// A fabrica do chefe da INF 321.
//

#include "LucasFactory.h"

#include <SDL_log.h>

#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"
#include "../../../CaminhosArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Lucas.h"

LucasFactory::LucasFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> LucasFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Lucas>(scene);
}

void LucasFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    // Arte PROVISORIA, gerada por Ferramentas/gerar_lucas.py: um cartao com
    // uma janela de navegador. Trocar o PNG basta - o json de quadro continua
    // valendo.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset("Teachers/DPIBHLucas.png"),
        Caminhos::Asset("Teachers/DPIBHLucas.json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/lucas/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, "lucas")) {
        SDL_Log("ERRO CRITICO: o chefe \"lucas\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.");
    }
}

void LucasFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Boss nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/lucas/fases.json. Sem configuracao de
    // reserva em C++, como nos outros: quem impede um arquivo quebrado de
    // chegar ao jogo e tests/test_arquivos_de_dados.cpp.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "lucas")) {
        SDL_Log("ERRO CRITICO: o chefe \"lucas\" nao pode ser montado a partir de "
                "Assets/Attacks/lucas/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
