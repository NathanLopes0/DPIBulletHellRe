//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "../Actors/Buttons/StageSelectButton.h"
#include "../Font.h"

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect),
    mNumMaterias(10)
{
    mStageSelectFont = new Font();
    mStageSelectFont->Load("../Assets/Fonts/Zelda.ttf");

    mButtons.reserve(mNumMaterias);
    SDL_Log("Stage Select Created");

}

void StageSelect::Load() {

    CreateButtons();
    DrawUI();

}

void StageSelect::CreateButtons() {

    float espacamentoX = 162; //espaçamento horizontal entre botões
    float espacamentoY = 140; //espaçamento vertical entre botões

    float widthBorder = (float)mGame->GetWindowWidth() / 12;    //margem horizontal
    float heightBorder = (float)mGame->GetWindowHeight() / 12;  //margem vertical

    //imagem do botão & variavel para guardar texto que será escrito dentro do botão
    std::string buttonSpritePath = "../Assets/Icons/DPIBHSelectSubjectButtonPrototype.png";
    std::string subjectCode = "";



}

void StageSelect::DrawUI() {

}

void StageSelect::ProcessInput(const Uint8 *keyState) {

}
