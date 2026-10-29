//
// A fabrica do chefe do TCC.
//

#include "HugoFactory.h"

#include <SDL_log.h>

#include "../../../Attacks/Engine/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/Engine/ProjeteisDeChefeArquivo.h"
#include "../../../Core/CaminhosArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Hugo.h"

HugoFactory::HugoFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> HugoFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Hugo>(scene);
}

void HugoFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    // Arte PROVISORIA, gerada por Ferramentas/gerar_hugo.py: o capelo da
    // formatura em cima de uma monografia. Trocar o PNG basta: o json de
    // quadro continua valendo.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset(Retrato() + ".png"),
        Caminhos::Asset(Retrato() + ".json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/hugo/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, "hugo")) {
        SDL_Log("ERRO CRITICO: o chefe \"hugo\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.");
    }
}

void HugoFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Boss nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/hugo/fases.json e as regras dos
    // projeteis no regras.json ao lado. NAO HA CONFIGURACAO DE RESERVA EM C++:
    // quem impede um arquivo quebrado de chegar ao jogo e
    // tests/Data/test_arquivos_de_dados.cpp, que le os Assets de verdade.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "hugo")) {
        SDL_Log("ERRO CRITICO: o chefe \"hugo\" nao pode ser montado a partir de "
                "Assets/Attacks/hugo/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
