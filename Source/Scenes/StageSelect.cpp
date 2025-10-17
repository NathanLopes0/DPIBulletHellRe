//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "../Math.h"
#include "../Random.h"
#include "../Actors/Buttons/StageSelectButton.h"
#include "../Font.h"

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect)
, mStageSelectFont(std::make_unique<Font>())
, mInputTimer(INPUT_DELAY)
{

    mStageSelectFont->Load("../Assets/Fonts/Zelda.ttf");
    mButtonObservers.reserve(NUM_STAGES);

}

void StageSelect::Load() {

    CreateStageButtons();
    CreateStaticUI();

}

void StageSelect::CreateStageButtons() {

    float espacamentoX = 162; //espaçamento horizontal entre botões
    float espacamentoY = 140; //espaçamento vertical entre botões

    float buttonWidth = 128;
    float buttonHeight = 64;

    const float widthBorder = static_cast<float>(mGame->GetWindowWidth()) / 12;    //margem horizontal
    const float heightBorder = static_cast<float>(mGame->GetWindowHeight()) / 12;  //margem vertical

    // -------- COMEÇAR REFATORAÇÃO AQUI --------- //

    CreateButton("INF 213",Game::INF213,
        Vector2(widthBorder + buttonWidth / 2.f,
        mGame->GetWindowHeight() / 2.f));

    const std::vector<std::pair<std::string, Game::GameSubject>> col1 = {
        {"INF 250", Game::GameSubject::INF250},
        {"INF 220", Game::GameSubject::INF220},
        {"INF 330", Game::GameSubject::INF330},
        {"INF 332", Game::GameSubject::INF332}
    };

    const std::vector<std::pair<std::string, Game::GameSubject>> col2 = {
        {"INF 420", Game::GameSubject::INF420},
        {"BIOINF", Game::GameSubject::BIOINF},
        {"INF394", Game::GameSubject::INF394},
        {"VISCPP", Game::GameSubject::VISCCP}
    };


    // --- PRIMEIRA COLUNA DE BOTÕES --- //
    float area2x = widthBorder + buttonWidth / 2.0f + espacamentoX;
    for (size_t i = 0; i < col1.size(); i++) {
        const float area2y = heightBorder + buttonHeight/2.0f + i * (espacamentoY + buttonHeight);
        CreateButton(col1[i].first, col1[i].second, Vector2(area2x, area2y));
    }

    area2x += espacamentoX;
    for (size_t i = 0; i < col2.size(); i++) {
        const float area2y = heightBorder + buttonHeight/2.0f + i * (espacamentoY + buttonHeight);
        CreateButton(col2[i].first, col2[i].second, Vector2(area2x, area2y));
    }

    CreateButton("TCC", Game::GameSubject::TCC, Vector2(
        mGame->GetWindowWidth() - (widthBorder + buttonWidth / 2.0f),
        mGame->GetWindowHeight() / 2.f));



    if (!mButtonObservers.empty()) {
        mButtonObservers[0]->ChangeSelected();
        mSelectedSubject = mButtonObservers[0]->GetSubject(); // Usa o enum, não um int!
    }


}

void StageSelect::CreateButton(const std::string& text, Game::GameSubject subject, const Vector2& position) {
    auto button = std::make_unique<StageSelectButton>(this, text, subject, "../Assets/Fonts/Zelda.ttf");
    button->SetPosition(position);

    mButtonObservers.push_back(button.get());
    this->AddActor(std::move(button));
}

void StageSelect::CreateStaticUI() {

}
void StageSelect::OnProcessInput(const Uint8 *keyState) {

    keyState = SDL_GetKeyboardState(nullptr);
    HandleSelectionInput(keyState);

}

void StageSelect::HandleSelectionInput(const Uint8 *keyState) {

    size_t currSelected = mSelectedIndex;
    if (mInputTimer >= INPUT_DELAY) {
        currSelected = HandleSelectedChange(keyState, currSelected);
    }

    if (currSelected != mSelectedIndex) {
        // Se houve mudança, atualiza a UI e o estado
        mButtonObservers[mSelectedIndex]->ChangeSelected(); // Desseleciona o antigo
        mButtonObservers[currSelected]->ChangeSelected(); // Seleciona o novo

        mSelectedIndex = currSelected; // ATUALIZA O ESTADO REAL
        mSelectedSubject = mButtonObservers[mSelectedIndex]->GetSubject();
        mInputTimer = 0.0f;
    }

    if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_SPACE]) {
        mGame->SetSelectedStage(mSelectedSubject);
        mGame->ChangeScene(SceneType::Battle);
    }
}

size_t StageSelect::HandleSelectedChange(const Uint8 *keyState, size_t currSelected) const {
    if (keyState[SDL_SCANCODE_UP])
        currSelected = HandleUpInput(currSelected);
    else if (keyState[SDL_SCANCODE_DOWN])
        currSelected = HandleDownInput(currSelected);
    else if (keyState[SDL_SCANCODE_LEFT])
        currSelected = HandleLeftInput(currSelected);
    else if (keyState[SDL_SCANCODE_RIGHT])
        currSelected = HandleRightInput(currSelected);

    return currSelected;
}
size_t StageSelect::HandleUpInput(const size_t currSelected) const {

    /*
     * Caso estejamos em alguma borda superior e o jogador apertar pra subir,
     * jogar a seleção lá pra baixo.
     */

    if (currSelected == 1) {
        return 4;
    }
    if (currSelected == 5) {
        return 8;
    }
    if (!IsInBorder(currSelected)) {
        return currSelected - 1;
    }

    SDL_Log("HandleUpInput em StageSelect.cpp: Nao esta retornando valores desejados");
    return currSelected;
}
size_t StageSelect::HandleDownInput(const size_t currSelected) const {
    /*
     * Caso estejamos na borda inferior (4 e 8) e o jogador apertar pra baixo,
     * joga a seleção lá pra cima.
     */

    if (currSelected == 4) {
        return 1;
    }
    if (currSelected == 8) {
        return 5;
    }
    if (!IsInBorder(currSelected)) {
        return currSelected + 1;
    }

    SDL_Log("HandleDownInput em StageSelect.cpp: Nao esta retornando valores desejados");
    return currSelected;
}
size_t StageSelect::HandleLeftInput(const size_t currSelected) const {
    if (!IsInBorder(currSelected)) {
        if (currSelected < 5) return 0;
        return currSelected - 4;
    }

    if (currSelected == NUM_STAGES - 1) {
        return Random::GetIntRange(6,7);
    }

    SDL_Log("Handle Left Input em StageSelect.cpp: Nao esta retornando valores desejados");
    return currSelected;
}
size_t StageSelect::HandleRightInput(const size_t currSelected) const {
    if (!IsInBorder(currSelected)) {
        SDL_Log("Não estamos na borda");
        return Math::Min<size_t>(currSelected + 4, NUM_STAGES - 1);
    }
    if (currSelected == 0) {
        SDL_Log("Estamos na borda direita, matéria 0");
        return Random::GetIntRange(2, 3);
    }

    SDL_Log("HandleRightInput em StageSelect.cpp: Nao esta retornando valores desejados");
    return currSelected;
}



bool StageSelect::IsInBorder(size_t currSelected) const {

    return currSelected == 0 || currSelected == NUM_STAGES - 1;

}

void StageSelect::OnUpdate(float deltaTime) {

    if(mInputTimer < INPUT_DELAY) mInputTimer += deltaTime;

}
