//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <memory>
#include "../Scene.h"
#include "../../Game.h"
#include "../../Actors/Teacher/BossFactory/IBossFactory.h"

class Player;
class ProjectileManager;

class Battle : public Scene {

public:

    explicit Battle(Game* game, Game::GameSubject selectedStage);
    ~Battle() override;
    void Load() override;
    void CheckCollisions();
    [[nodiscard]] Boss* GetBoss() const { return mBoss; }
    [[nodiscard]] Player* GetPlayer() const { return mPlayer; }

protected:
    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;

private:
    // --- Funções Auxiliares de Lógica Interna ---
    void LoadHUD();
    void LoadPlayer();
    void LoadBoss();


    // --- Membros de Estado da Batalha ---
    float mGrade = 40.f;
    Game::GameSubject mStage{};

    // --- Ponteiros observadores do Boss e do Player, que vão estar dentro
    // do vetor mActors da própria cena Battle. Ideal para, mesmo num vetor de Actors,
    // chamar funções específicas de Player e de Boss, e também para acesso rápido sem percorrer o vetor.
    Boss* mBoss{};
    Player* mPlayer{};

    // O GERENTE DE PROJÉTEIS: Um sistema especializado, possuído pela cena
    std::unique_ptr<ProjectileManager> mProjectileManager;

};


#endif //DPIBULLETHELLRE_BATTLE_H
