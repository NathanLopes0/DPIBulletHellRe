//
// Created by nslop on 23/07/2024.
//

#ifndef DPIBULLETHELLRE_MAINMENU_H
#define DPIBULLETHELLRE_MAINMENU_H

#include "Scene.h"

class MainMenu : public Scene {

public:
    explicit MainMenu(class Game* game);

    void Load() override;
    void ProcessInput(const Uint8* keyState) override;

private:
    class Font* mMainMenuFont;
};


#endif //DPIBULLETHELLRE_MAINMENU_H
