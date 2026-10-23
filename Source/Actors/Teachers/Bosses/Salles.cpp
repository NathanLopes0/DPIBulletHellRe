//
// Created by nslop on 25/09/2024.
//

#include "Salles.h"

#include <limits>

#include "../../../Game.h"
#include "../../../Random.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Components/ColliderComponents/CircleColliderComponent.h"
#include "../../../Components/DrawComponents/DrawAnimatedComponent.h"
#include "BossesProjectiles/BossProjectile.h"


Salles::Salles(Scene *scene) : Boss(scene)
{

}

void Salles::OnUpdate(float deltaTime) {
    Boss::OnUpdate(deltaTime);
    AtualizarCapivara(deltaTime);
}

void Salles::AtualizarCapivara(const float deltaTime) {
    const auto* fsm = GetComponent<FSMComponent>();
    if (!fsm || EhClone()) return;
    const std::string& fase = fsm->GetStateName();
    if (fase.empty()) return;

    // O AVISO NO PLACAR SAI SOZINHO, no tempo dele - na fase em que foi dado.
    if (mAvisoDaCapivara > 0.0f) {
        mAvisoDaCapivara -= deltaTime;
        if (mAvisoDaCapivara <= 0.0f) RegistrarAnuncio(mFaseDaCapivara, mAnuncioAntesDaCapivara);
    }

    // UMA FASE NOVA, UM SORTEIO NOVO: se ela vem nesta, quando e de que lado.
    if (fase != mFaseDaCapivara) {
        if (mAvisoDaCapivara > 0.0f) RegistrarAnuncio(mFaseDaCapivara, mAnuncioAntesDaCapivara);
        mAvisoDaCapivara = 0.0f;
        mFaseDaCapivara = fase;
        const auto sorteio = static_cast<std::uint32_t>(Random::GetIntRange(0, std::numeric_limits<int>::max()));
        mVisita = Capivara::Sortear(sorteio);
        mVeio = false;
    }
    if (!mVisita.vem || mVeio || fsm->GetStateTime() < mVisita.quando) return;

    // ELA APARECE, e o placar avisa.
    mVeio = true;
    mDisparandoCapivara = true;
    Disparar(Capivara::kGatilho);
    mDisparandoCapivara = false;
    mAnuncioAntesDaCapivara = AnuncioDaFase(fase);
    RegistrarAnuncio(fase, Capivara::kAnuncio);
    mAvisoDaCapivara = Capivara::kAnuncioNaTela;
}

void Salles::OnProjectilesCreated(std::vector<std::unique_ptr<Projectile>>& projectiles,
                                  const std::string& stateName) {
    Boss::OnProjectilesCreated(projectiles, stateName);
    if (!mDisparandoCapivara) return;

    // ELA ANDA OLHANDO PARA ONDE VAI.
    for (auto& p : projectiles) {
        if (auto* anim = p->GetComponent<DrawAnimatedComponent>()) {
            anim->SetAnimation(mVisita.daEsquerda ? Capivara::kAnimacaoDireita : Capivara::kAnimacaoEsquerda);
        }
    }
}

void Salles::CustomizeAttackParams(AttackParams &params, const std::string &stateName) {
    Boss::CustomizeAttackParams(params, stateName);

    // A CAPIVARA ENTRA POR UMA BORDA E ATRAVESSA ATE A OUTRA, reta, na altura
    // dela - e nao sai do professor nem no angulo da fase.
    if (mDisparandoCapivara) {
        const float largura = (mScene && mScene->GetGame())
                                  ? static_cast<float>(mScene->GetGame()->GetWindowWidth()) : 1200.0f;
        params.firePosition = Vector2(mVisita.daEsquerda ? -Capivara::kForaDaTela : largura + Capivara::kForaDaTela,
                                      Capivara::kAltura);
        params.centralAngle = mVisita.daEsquerda ? 0.0f : 180.0f;
        return;
    }

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




