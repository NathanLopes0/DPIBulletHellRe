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

IBossFactory::IBossFactory(Game* game) : mGame(game) {
}

std::unique_ptr<Boss> IBossFactory::CreateBoss(Scene *scene) {

    auto boss = InstantiateBoss(scene);
    if (!boss) {
        // SEM CHEFE NAO HA O QUE MONTAR. Seguir adiante passaria um nulo para
        // os passos, e o primeiro AddComponent derrubaria o jogo.
        SDL_Log("FABRICA: o chefe \"%s\" nao pode ser criado.", Pasta().c_str());
        return nullptr;
    }

    ConfigureComponents(boss.get());
    ConfigureAttacksAndFSM(boss.get());
    return boss;
}

void IBossFactory::ConfigureComponents(Boss* boss) {

    // O DESENHO: o cartao do professor, parado no primeiro quadro. A arte
    // provisoria de cada um sai de Ferramentas/gerar_<pasta>.py; trocar o PNG
    // basta, o json de quadros continua valendo.
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>(
        Caminhos::Asset(Retrato() + ".png"),
        Caminhos::Asset(Retrato() + ".json"));
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // O COLISOR, do tamanho do desenho.
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

    // As fases vivem em Assets/Attacks/<pasta>/fases.json e as regras dos
    // projeteis no regras.json ao lado. NAO HA CONFIGURACAO DE RESERVA EM C++:
    // quem impede um arquivo quebrado de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade.
    if (!ConfigurarFasesDeArquivo(boss, fsm, Pasta())) {
        SDL_Log("ERRO CRITICO: o chefe \"%s\" nao pode ser montado a partir de "
                "Assets/Attacks/%s/fases.json e ficara sem ataque. As linhas "
                "FASES: acima dizem o motivo.", Pasta().c_str(), Pasta().c_str());
    }

    boss->SetInitialState("StateOne");
}
