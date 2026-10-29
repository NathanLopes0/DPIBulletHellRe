//
// A fabrica do chefe da INF 340.
//

#include "LuizinhoFactory.h"

#include <SDL_log.h>

#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"
#include "../../../Core/CaminhosArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Luizinho.h"

LuizinhoFactory::LuizinhoFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> LuizinhoFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Luizinho>(scene);
}

void LuizinhoFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    // Arte PROVISORIA, gerada por Ferramentas/gerar_luizinho.py: um cartao com
    // uma coluna e um pergaminho. Trocar o PNG basta - o json de quadro
    // continua valendo.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset(Retrato() + ".png"),
        Caminhos::Asset(Retrato() + ".json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/luizinho/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, "luizinho")) {
        SDL_Log("ERRO CRITICO: o chefe \"luizinho\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.");
    }
}

void LuizinhoFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Boss nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/luizinho/fases.json. Sem configuracao
    // de reserva em C++, como nos outros: quem impede um arquivo quebrado de
    // chegar ao jogo e tests/test_arquivos_de_dados.cpp.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "luizinho")) {
        SDL_Log("ERRO CRITICO: o chefe \"luizinho\" nao pode ser montado a partir de "
                "Assets/Attacks/luizinho/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
