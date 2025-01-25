//
// Created by nslop on 04/04/2024.
//

#pragma once

#include <SDL_stdinc.h>
#include "../Math.h"
#include "../Actors/Projectile.h"

class Scene {
public:
    enum class SceneType
    {
        MainMenu,
        StageSelect,
        Battle
    };

    explicit Scene(class Game* game, SceneType sceneType);

    virtual void Load();
    virtual void ProcessInput(const Uint8* keyState);
    virtual void Update(float deltaTime);
    void SetScene(SceneType sceneType);

    class Game* GetGame() { return mGame; }
    SceneType GetType() { return mSceneType; }

    virtual const Vector2& GetCameraPos();

    virtual class Boss* GetBoss() const;
    virtual const Vector2& GetPlayerPosition() { return Vector2::Zero; }




    // TAMBEM PHUNÇÕES QUE ERAM PRA SER SÓ DA BATTLE
    virtual std::vector<Projectile*> GetBossProjectiles() { return {}; }
    void AddToBossProjectiles(Projectile* p) { mBossAllProjectiles.push_back(p); }
    void SetBossProjectiles(std::vector<Projectile *> vector1);
    void EraseBossProjectiles() { mBossAllProjectiles.clear(); }
    float GetGrade() const { return mGrade; };
    void AddGrade(float addValue) { mGrade = Math::Clamp(mGrade + addValue, 0.f, 100.f);}

protected:
    class Game* mGame;
    SceneType mSceneType;


    //COISAS DA CENA DE BATALHA, QUERO TIRAR DAQUI, MAS N SEI POLIMORPHAR DIREITO COM TEMPLATE
    std::vector<Projectile*> mBossAllProjectiles;
    float mGrade;


};
