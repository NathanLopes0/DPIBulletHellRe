//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Font.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

MainMenu::MainMenu(class Game *game): Scene(game, SceneType::MainMenu), mMainMenuFont(nullptr)

{
    mMainMenuFont = new Font();
    mMainMenuFont->Load("../Assets/Fonts/Zelda.ttf");

}

void MainMenu::Load() {

    //Instanciar o background geral
    auto background = new Actor(this);
    new DrawSpriteComponent(background, "../Assets/DPIUfV.png",1600, 1188, 50);
    background->SetPosition(Vector2(mGame->GetWindowWidth() / 2, mGame->GetWindowHeight() / 2));

    //Instanciar o Título do Jogo
    auto title = new Actor(this);
    new DrawSpriteComponent(title, "../Assets/DPIBHTitleMainMenu.png",1200, 800, 75);
    title->SetPosition(Vector2(mGame->GetWindowWidth()/2, mGame->GetWindowHeight()/2));

    //Instanciar "Press Space"
    auto pressSpace = new Actor(this);
    pressSpace->SetPosition(Vector2(600, 700));
    new DrawTextComponent(pressSpace, "Jogar", mMainMenuFont,
                          500, 64, 72);
}

void MainMenu::ProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);

    if(keyState[SDL_SCANCODE_SPACE])
        mGame->SetScene(SceneType::StageSelect);
}

