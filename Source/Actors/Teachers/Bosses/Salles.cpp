//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"
#include "../../../Random.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"


Salles::Salles(Scene *scene) : Boss(scene)
{

}

void Salles::OnUpdate(float deltaTime) {
    Boss::OnUpdate(deltaTime);
}

void Salles::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // Mesmo conserto de Andre::CustomizeAttackParams: Math::Acos(dir.x) so
    // devolve [0, 180] e perdia o sinal de y, fazendo o boss mirar para baixo
    // quando o player estava acima. Calculado uma vez, em vez de tres.
    const Vector2 dirToPlayer = GetDirectionToPlayer();
    const float angleToPlayer = Math::ToDegrees(Math::Atan2(dirToPlayer.y, dirToPlayer.x));

    if (stateName == "StateOne") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }

    }

    if (stateName == "StateTwo") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {
            params.centralAngle = angleToPlayer;
        }
        else {
            params.centralAngle = Random::GetFloatRange(45.f, 135.f);
        }
    }

    if (stateName == "StateThree") {
        if (const auto chance = Random::GetFloatRange(0.0f, 1.0f); chance < 0.3f) {

            // A BUSCA DESCE POR UM RAMO, E NAO POR CIMA DA RAIZ.
            //
            // Uma arvore binaria tem numero PAR de folhas - 16, aqui - e elas
            // se abrem simetricamente em torno do eixo do tiro. Logo existe
            // sempre um corredor vazio EM CIMA do eixo, da largura de um
            // espacamento. Mirar o eixo no jogador punha esse corredor
            // exatamente nele: a arvore se abria em volta e passava sem
            // encostar, por mais que ele ficasse parado. Da para ver numa foto
            // da fase - quatro nos longe a esquerda, quatro longe a direita,
            // nada no meio.
            //
            // O desvio poe o jogador SOBRE um ramo em vez de sobre a raiz, que
            // e o que uma busca faz de verdade: na raiz ela vai para a esquerda
            // ou para a direita, nunca fica em cima. O sinal alterna para o
            // desvio nao virar um vies fixo, que o jogador aprenderia a
            // compensar ficando um pouco de lado.
            mLadoDaBusca = -mLadoDaBusca;
            params.centralAngle = angleToPlayer + mLadoDaBusca * kDesvioDaBusca;
        }
        else {
            params.centralAngle = Random::GetFloatRange(0.f, 180.f);
        }
    }

    if (stateName == "StateFinal") {
        params.centralAngle = Random::GetFloatRange(0.f, 180.f);
    }
}




