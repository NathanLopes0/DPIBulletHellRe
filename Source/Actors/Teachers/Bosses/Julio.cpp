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

// Fracao do tempo de voo usada como antecipacao na fase 3.
//
// 1.0 seria interceptacao perfeita: quem anda em linha reta e SEMPRE atingido.
// 0.85 deixa uma margem pequena, entao correr reto quase sempre custa caro, mas
// nao e sentenca automatica. AJUSTE AQUI se a fase 3 ficar injusta (menor) ou
// facil demais (maior, ate 1.0).
//
// Derivar do tempo de voo, em vez de usar um numero fixo de segundos, importa:
// com antecipacao fixa de 0.5s e projetil a 265 px/s cruzando ~400 px, o tiro
// antecipava 150 px enquanto o jogador andava 450 px no mesmo intervalo - ou
// seja, correr reto era SEGURO, exatamente o oposto da intencao da fase.
static constexpr float kLeadFraction = 0.85f;

void Julio::CustomizeAttackParams(AttackParams& params, const std::string& stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Convencao de centralAngle: 0 = direita, 90 = baixo, 180 = esquerda.
    auto anguloDe = [](const Vector2& dir) {
        return Math::ToDegrees(Math::Atan2(dir.y, dir.x));
    };

    if (stateName == "StateOne") {
        // AINDA NAO TE CONHECE. Mira jogada fora com desvio grande: o jogador
        // le como "ele esta atirando a esmo".
        // Desvio menor que antes (era +-70): a varredura da fase 1 ja e larga
        // o bastante para nao parecer mirada, e um centro muito deslocado
        // jogaria metade do leque para fora da tela.
        params.centralAngle = anguloDe(GetDirectionToPlayer())
                            + static_cast<float>(Random::GetIntRange(-40, 40));
    }
    else if (stateName == "StateTwo") {
        // APRENDEU ONDE VOCE ESTA. Mira exata na posicao ATUAL. O desafio vem
        // da curva que os projeteis fazem depois (TrackingBehavior).
        params.centralAngle = anguloDe(GetDirectionToPlayer());
    }
    else if (stateName == "StateThree") {
        // APRENDEU COMO VOCE SE MOVE. Mira em onde o jogador ESTARA, nao onde
        // ele esta.
        //
        // Sem ruido aleatorio de proposito: se o tiro errasse tambem por acaso,
        // o jogador nao conseguiria distinguir "ele me previu" de "ele errou
        // sozinho", e a fase perderia a leitura inteira. Todo erro aqui tem de
        // ser causado pelo jogador ter mudado de direcao.
        // Tempo que o projetil leva para chegar onde o jogador esta agora.
        const float distancia = GetDistanceToPlayer();
        const float tempoDeVoo = (params.projectileSpeed > 1.0f)
                                     ? distancia / params.projectileSpeed
                                     : 0.0f;

        params.centralAngle = anguloDe(GetPredictedPlayerDirection(tempoDeVoo * kLeadFraction));
    }
    else if (stateName == "StateFinal") {
        // RECOMECOU DO ZERO. Volta a atirar para baixo, previsivel e justo.
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
