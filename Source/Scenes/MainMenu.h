//
// Created by nslop on 23/07/2024.
//

#pragma once

#include "Scene.h"
#include <memory> // Necessário para std::unique_ptr

// Forward declarations para manter o header limpo
class Font;
class Actor;

class MainMenu : public Scene {
public:
    explicit MainMenu(Game* game);

    void LoadPlayButton();

    void LoadBackground();

    void LoadTitle();

    ~MainMenu() override = default;

    void Load() override;
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

private:
    // A cena é DONA da fonte que ela carrega para o menu, então uso unique_ptr.
    std::unique_ptr<Font> mMainMenuFont;

    // A cena OBSERVA os atores que ela cria (dono = Game), então uso ponteiros brutos
    // para poder interagir com eles se necessário.

    Actor* mBackgroundActor{}; // Ator que segura a imagem de Background
    Actor* mTitleActor{}; // Ator que segura o título
    Actor* mPlayButtonActor{}; // Ator que segura o botão de start
};