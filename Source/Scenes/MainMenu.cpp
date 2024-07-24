//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Font.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

MainMenu::MainMenu(class Game *game)
        : Scene(game),
        mMainMenuFont(nullptr) {
    mMainMenuFont = new Font();
    mMainMenuFont->Load("../Assets/Fonts/Zelda.ttf");
}

void MainMenu::Load() {

    //Instanciar o background geral

    //Instanciar o Título do Jogo

    //Instanciar "Press Space"
    auto pressSpace = new Actor(this);
    pressSpace->SetPosition(Vector2(600, 700));
    new DrawTextComponent(pressSpace, "Jogar", mMainMenuFont,
                          500, 64, 72);
}

void MainMenu::ProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);

    if(keyState[SDL_SCANCODE_SPACE])
        mGame->SetScene(Game::GameScene::StageSelect);
}

