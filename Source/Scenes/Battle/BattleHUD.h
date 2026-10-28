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

    /// 'relogioParado': o chefe esta congelado (o HLT), e a fase nao corre.
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

    // --- SISTEMA DE ORBES VISUAIS ---
    void UpdateVisualOrbs(float deltaTime);

    /// Quantas orbes giram em volta de cada aluno. E o teto de Pontos Extras
    /// que da para ver - quem tem mais que isto nao ganha orbe nova.
    static constexpr size_t kOrbesPorJogador = 3;

    /**
     * @brief As orbes de CADA jogador, um conjunto por posicao do painel.
     *
     * Era um vetor so, de tres, girando em volta do unico aluno. Com dois em
     * campo as orbes sao de cada um - o estoque de Pontos Extras e proprio
     * (requisitos-coop, D8) -, e um conjunto so faria as orbes do jogador 2
     * girarem em volta do jogador 1.
     *
     * Apenas ponteiros observadores: a Scene e dona dos Actors.
     */
    std::vector<std::vector<Actor*>> mOrbesPorJogador;

    float mPulseTimer = 0.0f;
    const float ORB_SMOOTH_SPEED = 20.0f;
    const float ORB_DISTANCE = 45.0f;

};
