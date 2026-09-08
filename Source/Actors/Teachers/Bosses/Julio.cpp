//
// Júlio — professor de Inteligência Artificial (INF 420)
//

#include "Julio.h"

#include <SDL_log.h>

#include "../../../Random.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"

Julio::Julio(Scene* scene) : Boss(scene)
{
}

void Julio::OnUpdate(const float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Julio::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Ângulo até o jogador na convenção de centralAngle
    // (0 = direita, 90 = baixo, 180 = esquerda).
    const Vector2 dirToPlayer = GetDirectionToPlayer();
    const float angleToPlayer = Math::ToDegrees(Math::Atan2(dirToPlayer.y, dirToPlayer.x));

    if (stateName == "StateOne") {
        // EXPLORAÇÃO: o "modelo" ainda não sabe mirar. A direção central recebe
        // um desvio aleatório grande, então a rajada raramente aponta para o
        // jogador. É de propósito: esta fase deve parecer busca cega.
        params.centralAngle = angleToPlayer + static_cast<float>(Random::GetIntRange(-70, 70));
    }
    else if (stateName == "StateTwo") {
        // DESCIDA DO GRADIENTE: mira exata. O desafio da fase vem da curva que
        // os projéteis fazem depois (GradientDescentBehavior), não da mira.
        params.centralAngle = angleToPlayer;
    }
    else if (stateName == "StateThree") {
        // OVERFITTING: mira exata, mas com um erro pequeno e sistemático — o
        // modelo "decorou" uma posição levemente errada. Combinado com o leque
        // largo e a oscilação, cobre bastante área.
        params.centralAngle = angleToPlayer + static_cast<float>(Random::GetIntRange(-8, 8));
    }
    else if (stateName == "StateFinal") {
        // REGULARIZAÇÃO: volta a atirar para baixo, previsível e justo.
        params.centralAngle = 90.0f;
    }
}

void Julio::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                 const std::string& stateName) {

    // Cor por FASE, não por projétil: o jogador aprende a associar a cor ao
    // comportamento que vem a seguir.
    std::string animation = "Ruido";
    if (stateName == "StateTwo")        animation = "Gradiente";
    else if (stateName == "StateThree") animation = "Overfit";

    for (auto& p : projectiles) {
        if (!p) continue;
        if (auto drawComp = p->GetComponent<DrawAnimatedComponent>()) {
            drawComp->SetAnimation(animation);
        }
    }
}
