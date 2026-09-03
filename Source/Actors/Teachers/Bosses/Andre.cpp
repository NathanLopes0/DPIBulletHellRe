//
// Created by gensh on 27/11/2025.
//

#include "Andre.h"
#include "../../../Random.h"
#include "../../ProjectileFactory.h"
#include "../../../Scenes/Battle/Battle.h"
#include "../../../Scenes/Battle/ProjectileManager.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"

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

void Andre::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {
    // Uma cor por RAJADA, nao por projetil: o sorteio fica fora do laco, igual
    // ao codigo original.
    static const std::vector<std::string> colors = { "Red", "Blue", "Yellow" };
    const int choose = Random::GetIntRange(0, static_cast<int>(colors.size()) - 1);

    for (auto& p : projectiles) {
        if (!p) continue;
        // Guarda de nulo que a versao antiga nao tinha: ela dereferenciava
        // GetComponent<DrawAnimatedComponent>() direto.
        if (auto drawComp = p->GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation(colors[choose]);
        }
    }
}

