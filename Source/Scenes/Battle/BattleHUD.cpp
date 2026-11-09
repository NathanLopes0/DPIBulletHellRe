//
// Created by gensh on 11/11/2025.
//

#include "BattleHUD.h"

#include "Battle.h"
#include "../../Attacks/Fase.h"
#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Player/Player.h"
#include "../../Components/DrawComponents/DrawRelogioDaFaseComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

#include "../../Motor/CaminhosArquivo.h"

BattleHUD::BattleHUD(Scene *scene)
    :mOwner(scene),
    mCurrentStateTime(0.0f),
    mTotalStateTime(Fase::kDuracao)
{
    // ---- Criação de objetos POWER_ICON que seguem o player ----- // ----
    const size_t quantos = scene->GetGame() ? scene->GetGame()->QuantosJogadores() : 1;

    for (size_t quem = 0; quem < quantos; ++quem) {

        std::vector<Actor*> doJogador;

        for (size_t i = 0; i < kOrbesPorJogador; ++i) {

            // Cria o Actor que vai segurar a sprite e a posição.
            auto orbActor = std::make_unique<Actor>(scene);

            // Configura o visual.
            orbActor->SetScale(1.5f);
            const auto dc = orbActor->AddComponent<DrawAnimatedComponent>(
                Caminhos::Asset("Icons/DPIBHPowerIcon.png"),
                Caminhos::Asset("Icons/DPIBHPowerIcon.json")
            );
            dc->AddAnimation("Idle", {0, 1, 2, 3, 4, 5, 6, 7, 8});
            dc->SetAnimation("Idle");
            dc->SetIsVisible(false); // Começa invisível

            // Guarda referência.
            doJogador.push_back(orbActor.get());

            // Passa a posse para a cena, pq a cena é dona de todos os Actors que estão nela.
            scene->AddActor(std::move(orbActor));
        }

        mOrbesPorJogador.push_back(std::move(doJogador));
    }
}

void BattleHUD::OnUpdate(const float deltaTime, const bool relogioParado) {
    if (!relogioParado) mCurrentStateTime += deltaTime;

    if (mTimeBarComp != nullptr) {
        mTimeBarComp->SetTempo(mTotalStateTime - mCurrentStateTime, mTotalStateTime);
    }

    UpdateVisualOrbs(deltaTime);
}

void BattleHUD::UpdateVisualOrbs(float deltaTime) {

    // Precisamos do Player para saber onde orbitar e quantos pontos tem.
    const auto battle = dynamic_cast<Battle*>(mOwner);
    if (!battle) return;

    // Aqui nao e Alvo().
    const auto& jogadores = battle->Jogadores();

    // A respiracao e uma so, e por isso o relogio avanca fora do laco dos jogadores.
    mPulseTimer += deltaTime;
    const float breathing = std::sin(mPulseTimer * 3.0f) * 5.0f;
    const float currentDist = ORB_DISTANCE + breathing;

    // Offsets (Direita, Baixo, Esquerda)
    const std::vector<Vector2> offsets = {
        Vector2(currentDist, 0.0f),
        Vector2(0.0f, currentDist),
        Vector2(-currentDist, 0.0f)
    };

    for (size_t quem = 0; quem < jogadores.size() && quem < mOrbesPorJogador.size(); ++quem) {

        const auto player = jogadores[quem];
        if (!player) continue;

        const std::vector<Actor*>& orbes = mOrbesPorJogador[quem];
        const int numPoints = player->PontosExtras();
        const Vector2 playerPos = player->GetPosition();

        for (size_t i = 0; i < orbes.size(); i++) {
            const auto dc = orbes[i]->GetComponent<DrawAnimatedComponent>();

            // Lógica de Visibilidade.
            if (static_cast<int>(i) < numPoints) {
                if (!dc->GetIsVisible()) {
                    dc->SetIsVisible(true);
                    // Teleporta para a posição inicial para não vir voando do além.
                    orbes[i]->SetPosition(playerPos + offsets[i]);
                }

                // ---- Lógica de movimento suave (lerp - Linear Interpolation) ----
                Vector2 targetPos = playerPos + offsets[i];
                Vector2 currentPos = orbes[i]->GetPosition();

                // O HUD puxa o orbe para perto do Player.
                Vector2 newPos = Vector2::Lerp(currentPos, targetPos, ORB_SMOOTH_SPEED * deltaTime);

                orbes[i]->SetPosition(newPos);
            }
            else {
                if (dc->GetIsVisible()) {
                    dc->SetIsVisible(false);
                }
            }
        }
    }
}

// ---- Metodo de injeção para Battle chamar ----- // ----
void BattleHUD::SetTimeBar(Actor *barActor, DrawRelogioDaFaseComponent *barComp) {
    mTimeBarActor = barActor;
    mTimeBarComp = barComp;
}

void BattleHUD::ResetTimeBar(const float newTotalTime) {
    mCurrentStateTime = 0.0f;

    if (newTotalTime > 0) {
        mTotalStateTime = newTotalTime;
    }
    else {
        mTotalStateTime = 1.0f;
    }

    // A capsula se enche de volta, num clarao (DrawRelogioDaFaseComponent).
    if (mTimeBarComp) {
        mTimeBarComp->SetTempo(mTotalStateTime, mTotalStateTime);
        mTimeBarComp->Recomecar();
    }
}

