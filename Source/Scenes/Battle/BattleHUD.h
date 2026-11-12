//
// Created by gensh on 11/11/2025.
//

#pragma once
#include <cstddef>
#include <vector>

class Actor;
class Scene;
class DrawRelogioDaFaseComponent;

class BattleHUD {

public:
    BattleHUD(Scene* scene);

    /// 'relogioParado': o chefe esta congelado (o hlt), e a fase nao corre.
    void OnUpdate(float deltaTime, bool relogioParado = false);
    void SetTimeBar(Actor* barActor, DrawRelogioDaFaseComponent* barComp);

    /// Os segundos que faltam para a fase acabar.
    [[nodiscard]] float Restante() const { return mTotalStateTime - mCurrentStateTime; }

    void ResetTimeBar(float newTotalTime);

private:

    Scene* mOwner;
    Actor* mTimeBarActor = nullptr;
    DrawRelogioDaFaseComponent* mTimeBarComp = nullptr;

    float mCurrentStateTime;
    float mTotalStateTime;

    // ---- Sistema de orbes visuais ----
    void UpdateVisualOrbs(float deltaTime);

    /// Quantas orbes giram em volta de cada aluno. E o teto de Pontos Extras
    /// que da para ver - quem tem mais que isto nao ganha orbe nova.
    static constexpr size_t kOrbesPorJogador = 3;

    /// As orbes de cada jogador, um conjunto por posicao do painel.
    std::vector<std::vector<Actor*>> mOrbesPorJogador;

    float mPulseTimer = 0.0f;
    const float ORB_SMOOTH_SPEED = 20.0f;
    const float ORB_DISTANCE = 45.0f;

};
