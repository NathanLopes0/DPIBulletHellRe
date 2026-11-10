//
// Created by gensh on 19/10/2025.
//

#include "IBossFactory.h"

#include <SDL_log.h>

#include "../Boss.h"
#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "../../../Attacks/ProjeteisDeChefeArquivo.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../../../Motor/CaminhosArquivo.h"
#include "../../../Motor/Game.h"

IBossFactory::IBossFactory(Game* game) : mGame(game) {
}

std::unique_ptr<Boss> IBossFactory::CreateBoss(Scene *scene) {

    auto boss = InstantiateBoss(scene);
    if (!boss) {
        // Sem chefe nao ha o que montar.
        SDL_Log("FABRICA: o chefe \"%s\" nao pode ser criado.", Pasta().c_str());
        return nullptr;
    }

    ConfigureComponents(boss.get());
    ConfigureAttacksAndFSM(boss.get());
    return boss;
}

void IBossFactory::ConfigureComponents(Boss* boss) {

    // O desenho: o cartao do professor, parado no primeiro quadro.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset(Retrato() + ".png"),
        Caminhos::Asset(Retrato() + ".json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // O colisor, do tamanho do desenho.
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    const auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    // Os projeteis vem de Assets/Attacks/<pasta>/projeteis.json.
    if (!RegistrarProjeteisDeArquivo(boss, Pasta())) {
        SDL_Log("ERRO CRITICO: o chefe \"%s\" ficou sem projetil. As linhas "
                "PROJETEIS: acima dizem o motivo.", Pasta().c_str());
    }
}

void IBossFactory::ConfigureAttacksAndFSM(Boss* boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) {
        SDL_Log("ERRO CRITICO: o chefe \"%s\" nao tem FSMComponent.", Pasta().c_str());
        return;
    }

    // Nao ha configuracao de reserva em c++.
    if (!ConfigurarFasesDeArquivo(boss, fsm, Pasta())) {
        SDL_Log("ERRO CRITICO: o chefe \"%s\" nao pode ser montado a partir de "
                "Assets/Attacks/%s/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.", Pasta().c_str(), Pasta().c_str());
    }

    // A simulacao pode comecar numa fase dada (--estado): mede ou fotografa uma fase sozinha.
    const Game* game = boss->GetScene() ? boss->GetScene()->GetGame() : nullptr;
    const std::string& estado = game ? game->ConfiguracaoDaSimulacao().estado : std::string();
    boss->SetInitialState(estado.empty() ? "StateOne" : estado);
}
