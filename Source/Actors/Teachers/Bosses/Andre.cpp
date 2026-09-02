//
// Created by gensh on 27/11/2025.
//

#include "Andre.h"
#include "../../../Random.h"
#include "../../ProjectileFactory.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"

Andre::Andre(Scene *scene) : Boss(scene)
{

}

void Andre::OnUpdate(float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Andre::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {

    // Angulo, em GRAUS, da direcao ate o player, na convencao de
    // AttackParams::centralAngle (0 = direita, 90 = baixo, 180 = esquerda).
    // Antes usava Math::Acos(dir.x), que so devolve [0, 180] e portanto PERDE
    // o sinal de y: (0,1) e (0,-1) davam ambos 90 graus. Na pratica, com o
    // player ACIMA do boss, ele mirava para baixo.
    // Atan2(y, x) preserva o quadrante e devolve o angulo correto (negativo
    // para cima, o que o cos/sin das strategies interpreta sem problema).
    const Vector2 dirToPlayer = GetDirectionToPlayer();
    const auto directionToPlayerAngle = Math::ToDegrees(Math::Atan2(dirToPlayer.y, dirToPlayer.x));

    if (stateName == "StateOne" || stateName == "StateTwo") {
        params.centralAngle = Random::GetFloatRange(directionToPlayerAngle - 20.f, directionToPlayerAngle + 20.f);
    }
    else if (stateName == "StateThree") {
        params.projectileSpeed = 500.f;

        //Se o player tiver acima do Andre, atirar nele
        if (dirToPlayer.y < 0) {
            params.projectileSpeed = 600.f;
            params.numProjectiles = 36;
            params.centralAngle = directionToPlayerAngle;
        }
        else {
            params.numProjectiles = 18;
            params.centralAngle = Random::GetFloatRange(75.f, 105.f);
        }
    }
}

void Andre::ExecuteAttack(AttackDefinition &attackDef, const std::string &stateName) {
    // Atualiza a posição de tiro pra posição do boss.
    attackDef.params->firePosition = GetPosition();

    // Chama customização do boss específico
    CustomizeAttackParams(*(attackDef.params), stateName);

    auto battleScene = dynamic_cast<Battle*>(mScene);
    if (!battleScene) { return; }

    auto projManager = battleScene->GetProjectileManager();
    if (!projManager) { return; }

    // ---------- EXECUTA O ATAQUE ---------- //
    auto projectiles = attackDef.strategy->Execute(*(attackDef.params));
    // ---------- ------- - ------ ---------- //

    // Depois de executar, aplicar a personalização lambda, se existir
    if (attackDef.configurator) {
        for (int i = 0; i < projectiles.size(); ++i) {
            attackDef.configurator(projectiles[i].get(), i);
        }
    }

    // --- DIFERENÇA DO EXECUTEATTACK DO BOSS PRO DO ANDRE --- //
    const int choose = Random::GetIntRange(0,2);
    const std::vector<std::string> colors = {
        "Red",
        "Blue",
        "Yellow"
    };
    for (auto& p : projectiles) {
        p->GetComponent<DrawAnimatedComponent>()->SetAnimation(colors[choose]);
    }
    // --- --------- -- ------------- -- ---- --- -- ----- --- //

    // Converte pra BossProjectile e envia para o Manager
    std::vector<std::unique_ptr<BossProjectile>> bossProjVector;
    bossProjVector.reserve(projectiles.size());

    for (auto& p : projectiles) {
        if (dynamic_cast<BossProjectile*>(p.get())) {
            bossProjVector.emplace_back(std::unique_ptr<BossProjectile>(dynamic_cast<BossProjectile*>(p.release())));
        }
    }

    if (!bossProjVector.empty()) {
        projManager->AddBossProjectiles(std::move(bossProjVector));
    }

    // -- LIDANDO COM O AUDIO DO ATAQUE -- //
    std::string attackAudio;
    {
        if (auto choose = Random::GetIntRange(0,1)) attackAudio = "tan00.wav";
        else attackAudio = "tan01.wav";
    }
    auto handler = mScene->GetGame()->GetAudio()->PlaySound(attackAudio, false);
    mScene->GetGame()->GetAudio()->SetSoundVolume(handler, 10);
    // ----------------------------- //
}

