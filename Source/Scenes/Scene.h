//
// Created by nslop on 04/04/2024.
//

#ifndef DPIBULLETHELLRE_SCENE_H
#define DPIBULLETHELLRE_SCENE_H

#include <SDL_stdinc.h>
#include "../Math.h"

class Scene {

public:
    Scene(class Game* game);

    virtual void Load();
    virtual void ProcessInput(const Uint8* keyState);

    class Game* GetGame() { return mGame; }

    virtual const Vector2& GetCameraPos();

protected:
    class Game* mGame;
};


#endif //DPIBULLETHELLRE_SCENE_H
