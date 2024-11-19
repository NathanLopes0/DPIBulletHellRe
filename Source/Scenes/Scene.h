//
// Created by nslop on 04/04/2024.
//

#pragma once

#include <SDL_stdinc.h>
#include "../Math.h"

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

    virtual class Boss * GetBoss() const;

protected:
    class Game* mGame;
    SceneType mSceneType;
};
