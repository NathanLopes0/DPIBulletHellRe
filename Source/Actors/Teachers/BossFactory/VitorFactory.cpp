//
// A fabrica do chefe da INF 310.
//

#include "VitorFactory.h"

#include <SDL_log.h>

#include "../../../Attacks/Engine/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/Engine/ProjeteisDeChefeArquivo.h"
#include "../../../Core/CaminhosArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Vitor.h"

VitorFactory::VitorFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> VitorFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Vitor>(scene);
}

void VitorFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    // Arte PROVISORIA, gerada por Ferramentas/gerar_vitor.py: um cartao com
    // o diagrama de fork/join - um fluxo que se abre em dois e volta a se
    // juntar, que e o que a fase 3 dele faz de verdade. Trocar o PNG basta: o
    // json de quadro continua valendo.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset(Retrato() + ".png"),
        Caminhos::Asset(Retrato() + ".json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/vitor/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, "vitor")) {
        SDL_Log("ERRO CRITICO: o chefe \"vitor\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.");
    }
}

void VitorFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Boss nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/vitor/fases.json e as regras dos
    // projeteis no regras.json ao lado. Como no Ricardo, NAO HA CONFIGURACAO DE
    // RESERVA EM C++: se o arquivo faltar ou tiver uma transicao quebrada, este
    // chefe fica SEM ATAQUE NENHUM, e quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "vitor")) {
        SDL_Log("ERRO CRITICO: o chefe \"vitor\" nao pode ser montado a partir de "
                "Assets/Attacks/vitor/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
