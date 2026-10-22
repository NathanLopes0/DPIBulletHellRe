//
// A fabrica do chefe da INF 331.
//

#include "VladimirFactory.h"

#include <SDL_log.h>

#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"
#include "../../../CaminhosArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Vladimir.h"

VladimirFactory::VladimirFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> VladimirFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<Vladimir>(scene);
}

void VladimirFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    // Arte PROVISORIA, gerada por Ferramentas/gerar_vladimir.py: um cartao com
    // um automato de dois estados. Trocar o PNG basta - o json de quadro
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

    // Os projeteis vem de Assets/Attacks/vladimir/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, "vladimir")) {
        SDL_Log("ERRO CRITICO: o chefe \"vladimir\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.");
    }
}

void VladimirFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Boss nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/vladimir/fases.json e as regras dos
    // projeteis no regras.json ao lado. Como no Ricardo, NAO HA CONFIGURACAO DE
    // RESERVA EM C++: se o arquivo faltar ou tiver uma transicao quebrada, este
    // chefe fica SEM ATAQUE NENHUM, e quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "vladimir")) {
        SDL_Log("ERRO CRITICO: o chefe \"vladimir\" nao pode ser montado a partir de "
                "Assets/Attacks/vladimir/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
