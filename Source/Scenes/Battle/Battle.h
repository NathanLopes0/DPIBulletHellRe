//
// Created by nslop on 16/09/2024.
//

#ifndef DPIBULLETHELLRE_BATTLE_H
#define DPIBULLETHELLRE_BATTLE_H

#include <memory>
#include "../Scene.h"
#include "../../Game.h"
#include "../../Actors/Teachers/BossFactory/IBossFactory.h"

#define GRADE_CHANGE_UP 0.56f
#define GRADE_CHANGE_DOWN 6.f

class Font;
class Player;
class ProjectileManager;
class BattleHUD;
class ExtraPointItem;

class Battle : public Scene {

public:

    explicit Battle(Game* game, Game::GameSubject selectedStage);
    ~Battle() override;
    void Load() override;
    void CheckCollisions();
    void ResetHUDTimer(float newDuration);

    [[nodiscard]] Boss* GetBoss() const { return mBoss; }
    [[nodiscard]] Player* GetPlayer() const { return mPlayer; }
    [[nodiscard]] ProjectileManager* GetProjectileManager() const { return mProjectileManager.get(); }

    SDL_FRect GetPlayfieldBounds() const override;
    void OnPlayerUsedExtraPoint();
    void SpawnExtraPoint(Vector2 position);



protected:
    void TimeBarUpdate() const;

    void GradeTextUpdate();
    void GradeBarUpdate();

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;

private:
    // --- Funções Auxiliares de Lógica Interna ---
    void LoadHUD();
    void LoadPlayer();
    void LoadBoss();

    void LoadTimeClock();

    void LoadTimeBar();
    void LoadGradeBar();


    // --- Membros de Estado da Batalha ---
    float mGrade;
    Game::GameSubject mStage{};

    // --- Ponteiros observadores do Boss e do Player, que vão estar dentro
    // do vetor mActors da própria cena Battle. Ideal para, mesmo num vetor de Actors,
    // chamar funções específicas de Player e de Boss, e também para acesso rápido sem percorrer o vetor.
    Boss* mBoss{};
    Player* mPlayer{};

    // O GERENTE DE PROJÉTEIS: Um sistema especializado, possuído pela cena
    std::unique_ptr<ProjectileManager> mProjectileManager;

    // O HUD
    std::unique_ptr<BattleHUD> mHUD;
    std::unique_ptr<Font> mGradeBarFont;
    Actor* mHUDTimeBarActor = nullptr;
    Actor* mHUDClockActor = nullptr;

    Actor* mGradeBarActor = nullptr;
    Actor* mGradeTextActor = nullptr;

    void GradeUp(float amount = GRADE_CHANGE_UP);
    void GradeDown();

    std::vector<ExtraPointItem*> mExtraPoints;

};


#endif //DPIBULLETHELLRE_BATTLE_H
