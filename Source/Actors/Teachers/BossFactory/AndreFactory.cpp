//
// Created by gensh on 27/11/2025.
//

#include "AndreFactory.h"


#include "../../Teachers/Bosses/Andre.h"
#include "../../../Attacks/FasesDeAtaqueArquivo.h"
#include "BossProjectileFactory/AndreBaloonProjectileFactory.h"

AndreFactory::AndreFactory(Game *game)
    : IBossFactory(game)
{

}

std::unique_ptr<Boss> AndreFactory::InstantiateBoss(Scene* scene) {
    auto boss = std::make_unique<Andre>(scene);
    return boss;
}

void AndreFactory::ConfigureComponents(Boss *boss) {

    // ----- DRAW COMPONENT ----- //
    auto drawComp = boss->AddComponent<DrawAnimatedComponent>("../Assets/Teachers/DPIBHAndre.png",
                                                                               "../Assets/Teachers/DPIBHAndre.json");
    drawComp->AddAnimation("Idle", {0});
    drawComp->SetAnimation("Idle");

    // ----- COLLIDER COMPONENT ----- //
    const float colliderRadius = static_cast<float>(drawComp->GetSpriteWidth()) / 2.2f;
    auto collider = boss->AddComponent<CircleColliderComponent>(colliderRadius);
    collider->SetTag(ColliderTag::Boss);

    boss->AddProjectileFactory("Baloes", std::make_unique<AndreBaloonProjectileFactory>());

}

void AndreFactory::ConfigureAttacksAndFSM(Boss *boss) {

    auto fsm = boss->GetComponent<FSMComponent>();
    if (!fsm) { SDL_Log("ERRO CRÍTICO: Boss não tem FSMComponent!"); return; }

    // As fases deste chefe vivem em Assets/Attacks/fases.json, no conjunto
    // "andre".
    //
    // ATENCAO: parte do balanceamento NAO esta la. Andre::CustomizeAttackParams
    // sobrescreve numProjectiles, projectileSpeed e anguloCentral da fase 3 a
    // cada disparo, com valores diferentes conforme o jogador esteja acima ou
    // abaixo. Mexer nesses tres no arquivo nao tem efeito; para mudar a fase 3,
    // mexa em Andre.cpp.
    //
    // NAO HA MAIS CONFIGURACAO DE RESERVA EM C++. Se o arquivo faltar, estiver
    // malformado ou tiver uma transicao quebrada, ConfigurarFasesDeArquivo
    // devolve false depois de explicar o motivo no log, e este chefe fica SEM
    // ATAQUE NENHUM. Quem impede isso de chegar ao jogo e
    // tests/test_arquivos_de_dados.cpp, que le os Assets de verdade e quebra a
    // suite ao primeiro erro de digitacao.
    if (!ConfigurarFasesDeArquivo(boss, fsm, "andre")) {
        SDL_Log("ERRO CRITICO: o chefe \"andre\" nao pode ser montado a partir de "
                "Assets/Attacks/fases.json e ficara sem ataque. As linhas FASES: acima "
                "dizem o motivo.");
    }

    boss->SetInitialState("StateOne");
}
