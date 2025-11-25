//
// Created by gensh on 11/11/2025.
//

#include "BattleHUD.h"

#include "Battle.h"
#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Player/Player.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"




BattleHUD::BattleHUD(Scene *scene)
    :mOwner(scene),
    mCurrentStateTime(0.0f),
    mTotalStateTime(17.f)
{
    // Eles pertencem à Cena (Scene), mas o HUD guarda referência para movê-los.
    for (int i = 0; i < 3; i++) {
        // Cria um ator genérico (não precisa de classe PlayerOrb especial)
        auto orbActor = std::make_unique<Actor>(scene);

        // Configura o visual
        orbActor->SetScale(1.5f);
        auto dc = orbActor->AddComponent<DrawAnimatedComponent>(
            "../Assets/Icons/DPIBHPowerIcon.png",
            "../Assets/Icons/DPIBHPowerIcon.json"
        );
        dc->AddAnimation("Idle", {0, 1, 2, 3, 4, 5, 6, 7, 8});
        dc->SetAnimation("Idle");
        dc->SetIsVisible(false); // Começa invisível

        // Guarda referência
        mVisualOrbs.push_back(orbActor.get());

        // Passa a posse para a cena
        scene->AddActor(std::move(orbActor));
    }
}

void BattleHUD::OnUpdate(float deltaTime) {
    mCurrentStateTime += deltaTime;
    float progress = 1.0f - (mCurrentStateTime / mTotalStateTime);

    if (mTimeBarComp != nullptr) {
        mTimeBarComp->SetFillPercent(progress);
    }

    UpdateVisualOrbs(deltaTime);
}

void BattleHUD::UpdateVisualOrbs(float deltaTime) {

    // Precisamos do Player para saber onde orbitar e quantos pontos tem
    auto battle = dynamic_cast<Battle*>(mOwner);
    if (!battle) return;

    auto player = battle->GetPlayer();
    if (!player) return;

    int numPoints = player->GetExtraPoints();
    Vector2 playerPos = player->GetPosition();

    // Animação de respiração
    mPulseTimer += deltaTime;
    float breathing = std::sin(mPulseTimer * 3.0f) * 5.0f;
    float currentDist = ORB_DISTANCE + breathing;

    // Offsets (Direita, Baixo, Esquerda)
    std::vector<Vector2> offsets = {
        Vector2(currentDist, 0.0f),
        Vector2(0.0f, currentDist),
        Vector2(-currentDist, 0.0f)
    };

    for (int i = 0; i < 3; i++) {
        auto dc = mVisualOrbs[i]->GetComponent<DrawAnimatedComponent>();

        // Lógica de Visibilidade
        if (i < numPoints) {
            if (!dc->GetIsVisible()) {
                dc->SetIsVisible(true);
                // Teleporta para a posição inicial para não vir voando do além
                mVisualOrbs[i]->SetPosition(playerPos + offsets[i]);
            }

            // --- LÓGICA DE MOVIMENTO SUAVE (LERP) ---
            Vector2 targetPos = playerPos + offsets[i];
            Vector2 currentPos = mVisualOrbs[i]->GetPosition();

            // A mágica acontece aqui: O HUD puxa o orbe para perto do Player
            Vector2 newPos = Vector2::Lerp(currentPos, targetPos, ORB_SMOOTH_SPEED * deltaTime);

            mVisualOrbs[i]->SetPosition(newPos);
        }
        else {
            if (dc->GetIsVisible()) {
                dc->SetIsVisible(false);
            }
        }
    }
}



// ----- Metodo de injeção para Battle chamar ----- //
void BattleHUD::SetTimeBar(Actor *barActor, ProgressBarComponent *barComp) {
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


    if (mTimeBarActor) {
        // Enche a barra novamente
        mTimeBarActor->GetComponent<ProgressBarComponent>()->SetFillPercent(1.0f);
    }
}


