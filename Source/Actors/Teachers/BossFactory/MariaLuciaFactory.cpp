//
// A fabrica da chefe da INF 221.
//

#include "MariaLuciaFactory.h"

#include <SDL_log.h>

#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"
#include "../../../CaminhosArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/MariaLucia.h"

MariaLuciaFactory::MariaLuciaFactory(Game* game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> MariaLuciaFactory::InstantiateBoss(Scene* scene) {
    return std::make_unique<MariaLucia>(scene);
}

void MariaLuciaFactory::ConfigureComponents(Boss* boss) {

    // ----- DRAW COMPONENT ----- //
    // Arte PROVISORIA, gerada por Ferramentas/gerar_maria_lucia.py: um cartao
    // com uma classe em UML - a caixa de tres reparticoes e a seta de heranca
    // saindo por cima. Trocar o PNG basta: o json de quadro continua valendo.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset("Teachers/DPIBHMariaLucia.png"),
        Caminhos::Asset("Teachers/DPIBHMariaLucia.json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/marialucia/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, "marialucia")) {
        SDL_Log("ERRO CRITICO: a chefe \"marialucia\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.");
    }
}

void MariaLuciaFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: Boss nao tem FSMComponent");
        return;
    }

    // As fases vivem em Assets/Attacks/marialucia/fases.json e as regras dos
    // projeteis no regras.json ao lado. Como no Ricardo, NAO HA CONFIGURACAO DE
    // RESERVA EM C++: se o arquivo faltar ou tiver uma transicao quebrada, esta
    // chefe fica SEM ATAQUE NENHUM, e quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "marialucia")) {
        SDL_Log("ERRO CRITICO: a chefe \"marialucia\" nao pode ser montada a partir de "
                "Assets/Attacks/marialucia/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
