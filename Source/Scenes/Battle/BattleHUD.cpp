//
// Created by gensh on 11/11/2025.
//

#include "BattleHUD.h"

#include "Battle.h"
#include "../../Attacks/Engine/Fase.h"
#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Actors/Player/Player.h"
#include "../../Components/DrawComponents/DrawRelogioDaFaseComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

#include "../../Core/CaminhosArquivo.h"




BattleHUD::BattleHUD(Scene *scene)
    :mOwner(scene),
    mCurrentStateTime(0.0f),
    mTotalStateTime(Fase::kDuracao)
{
    // ----- CRIAÇÃO DE OBJETOS POWER_ICON QUE SEGUEM O PLAYER ----- //
    // Pertencem à Cena (Scene), mas o HUD guarda referência para movê-los.
    //
    // UM CONJUNTO POR JOGADOR. O numero vem do Game, e nao da batalha: o HUD e
    // construido antes de os alunos existirem em campo, e quantos jogam ja foi
    // decidido no menu.
    const size_t quantos = scene->GetGame() ? scene->GetGame()->QuantosJogadores() : 1;

    for (size_t quem = 0; quem < quantos; ++quem) {

        std::vector<Actor*> doJogador;

        for (size_t i = 0; i < kOrbesPorJogador; ++i) {

            // Cria o Actor que vai segurar a sprite e a posição
            auto orbActor = std::make_unique<Actor>(scene);

            // Configura o visual
            orbActor->SetScale(1.5f);
            const auto dc = orbActor->AddComponent<DrawAnimatedComponent>(
                Caminhos::Asset("Icons/DPIBHPowerIcon.png"),
                Caminhos::Asset("Icons/DPIBHPowerIcon.json")
            );
            dc->AddAnimation("Idle", {0, 1, 2, 3, 4, 5, 6, 7, 8});
            dc->SetAnimation("Idle");
            dc->SetIsVisible(false); // Começa invisível

            // Guarda referência
            doJogador.push_back(orbActor.get());

            // Passa a posse para a cena, pq a cena é dona de todos os Actors que estão nela
            scene->AddActor(std::move(orbActor));
        }

        mOrbesPorJogador.push_back(std::move(doJogador));
    }
}

void BattleHUD::OnUpdate(const float deltaTime, const bool relogioParado) {
    // NO HLT A FASE PARA: a capsula do tempo para junto com a maquina de
    // estados do chefe, ou chegaria a zero antes da fase acabar.
    if (!relogioParado) mCurrentStateTime += deltaTime;

    if (mTimeBarComp != nullptr) {
        mTimeBarComp->SetTempo(mTotalStateTime - mCurrentStateTime, mTotalStateTime);
    }

    UpdateVisualOrbs(deltaTime);
}

void BattleHUD::UpdateVisualOrbs(float deltaTime) {

    // Precisamos do Player para saber onde orbitar e quantos pontos tem
    const auto battle = dynamic_cast<Battle*>(mOwner);
    if (!battle) return;

    // AQUI NAO E Alvo(). As orbes mostram o estoque de Pontos Extras de UM
    // jogador - que e proprio, nao da dupla (requisitos-coop, D8) - e precisam
    // da posicao DELE para orbitar. Alvo() responde outra pergunta: onde o
    // chefe mira. Eram a mesma chamada, e com dois jogadores deixariam de ser:
    // as orbes de um ficariam girando em volta do outro.
    const auto& jogadores = battle->Jogadores();

    // A RESPIRACAO E UMA SO, e por isso o relogio avanca FORA do laco dos
    // jogadores. Dentro, ele andaria duas vezes por quadro e os dois conjuntos
    // pulsariam no dobro da velocidade - um defeito que so apareceria no co-op.
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
        const int numPoints = player->GetExtraPoints();
        const Vector2 playerPos = player->GetPosition();

        for (size_t i = 0; i < orbes.size(); i++) {
            const auto dc = orbes[i]->GetComponent<DrawAnimatedComponent>();

            // Lógica de Visibilidade
            if (static_cast<int>(i) < numPoints) {
                if (!dc->GetIsVisible()) {
                    dc->SetIsVisible(true);
                    // Teleporta para a posição inicial para não vir voando do além
                    orbes[i]->SetPosition(playerPos + offsets[i]);
                }

                // --- LÓGICA DE MOVIMENTO SUAVE (LERP - Linear Interpolation) ---
                Vector2 targetPos = playerPos + offsets[i];
                Vector2 currentPos = orbes[i]->GetPosition();

                // O HUD puxa o orbe para perto do Player
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



// ----- Metodo de injeção para Battle chamar ----- //
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


    // A CAPSULA SE ENCHE DE VOLTA, num clarao (DrawRelogioDaFaseComponent).
    if (mTimeBarComp) {
        mTimeBarComp->SetTempo(mTotalStateTime, mTotalStateTime);
        mTimeBarComp->Recomecar();
    }
}


