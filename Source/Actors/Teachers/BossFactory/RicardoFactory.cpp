//
// Created by gensh on 26/11/2025.
//

#include "RicardoFactory.h"

#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Bosses/Ricardo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"

#include "../../../CaminhosArquivo.h"

RicardoFactory::RicardoFactory(Game *game)
    : IBossFactory(game)
{
}

std::unique_ptr<Boss> RicardoFactory::InstantiateBoss(Scene *scene) {
    return std::make_unique<Ricardo>(scene);
}

void RicardoFactory::ConfigureComponents(Boss *boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Teachers/DPIBHRicardo.png"),
        Caminhos::Asset("Teachers/DPIBHRicardo.json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/projeteis.json, conjunto "ricardo".
    // Acrescentar um tipo novo a este chefe e uma entrada naquele arquivo - nao
    // ha mais uma classe de fabrica por projetil.
    if (!RegistrarProjeteisDeArquivo(boss, "ricardo")) {
        SDL_Log("ERRO CRITICO: o chefe \"ricardo\" ficou sem projetil. As linhas PROJETEIS: "
                "acima dizem o motivo.");
    }
}

void RicardoFactory::ConfigureAttacksAndFSM(Boss *boss) {
    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent");
        return;
    }


    // As fases deste chefe vivem em Assets/Attacks/fases.json, no conjunto
    // "ricardo", e as regras dos projeteis em Assets/Attacks/regras.json.
    //
    // Ricardo nao sobrescreve nada em CustomizeAttackParams, entao TODO o
    // balanceamento dele esta naqueles dois arquivos.
    //
    // NAO HA MAIS CONFIGURACAO DE RESERVA EM C++. Se o arquivo faltar, estiver
    // malformado ou tiver uma transicao quebrada, ConfigurarFasesDeArquivo
    // devolve false depois de explicar o motivo no log, e este chefe fica SEM
    // ATAQUE NENHUM. Quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade e quebra a
    // suite ao primeiro erro de digitacao.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "ricardo")) {
        SDL_Log("ERRO CRITICO: o chefe \"ricardo\" nao pode ser montado a partir de "
                "Assets/Attacks/fases.json e ficara sem ataque. As linhas FASES: acima "
                "dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
