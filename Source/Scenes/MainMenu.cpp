//
// Created by nslop on 23/07/2024.
//

#include "MainMenu.h"
#include "../Game.h"
#include "../Actors/Actor.h"
#include "../Font.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

MainMenu::MainMenu(Game* game)
    : Scene(game, SceneType::MainMenu)
    , mMainMenuFont(std::make_unique<Font>())
{
    mMainMenuFont->Load("../Assets/Fonts/Zelda.ttf");
}

void MainMenu::Load() {

    LoadBackground();
    LoadTitle();
    LoadPlayButton();

}

void MainMenu::OnProcessInput(const Uint8 *keyState) {

    if(keyState[SDL_SCANCODE_SPACE])
        mGame->RequestSceneChange(SceneType::StageSelect);
}

void MainMenu::OnUpdate(float deltaTime) {

}

void MainMenu::LoadBackground() {
    auto background = std::make_unique<Actor>(this);
    background->SetPosition(Vector2(mGame->GetWindowWidth() / 2.0f, mGame->GetWindowHeight() / 2.0f));
    background->AddComponent<DrawSpriteComponent>("../Assets/MainMenuBackground.png", 50);

    mBackgroundActor = background.get(); //Guarda o ponteiro observador
    AddActor(std::move(background));
}
void MainMenu::LoadTitle() {
    auto title = std::make_unique<Actor>(this);
    title->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f,
                                    static_cast<float>(mGame->GetWindowHeight()) / 2.0f));

    title->AddComponent<DrawSpriteComponent>("../Assets/DPIBHTitleMainMenu.png", 75);

    mTitleActor = title.get();
    AddActor(std::move(title));
}
void MainMenu::LoadPlayButton() {
    auto playButton = std::make_unique<Actor>(this);
    playButton->SetPosition(Vector2(600.0f, 700.0f));
    // Passamos a fonte para o componente usando .get() para obter o ponteiro bruto
    playButton->AddComponent<DrawTextComponent>("Jogar", mMainMenuFont.get(), 500, 64, 72);

    mPlayButtonActor = playButton.get();
    AddActor(std::move(playButton));
}
