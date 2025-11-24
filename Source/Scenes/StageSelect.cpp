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
    // 1. Definições de Tamanho e Bordas
    constexpr float buttonWidth = 128.0f;
    constexpr float buttonHeight = 64.0f;

    // Obtendo dimensões da tela uma única vez para clareza
    const auto screenW = static_cast<float>(mGame->GetWindowWidth());
    const auto screenH = static_cast<float>(mGame->GetWindowHeight());

    // Margens (1/12 da tela)
    const float marginX = screenW / 12.0f;
    const float marginY = screenH / 12.0f;

    // 2. Definindo os Limites Horizontais (Âncoras X)
    // Onde começa o layout (lado esquerdo) e onde termina (lado direito)
    const float startX = marginX + buttonWidth / 2.0f;
    const float endX = screenW - (marginX + buttonWidth / 2.0f);

    // Calculamos o passo horizontal (3 intervalos para 4 colunas visuais, por isso divide por 3)
    const float stepX = (endX - startX) / 3.0f;

    // 3. Definindo os Limites Verticais (Âncoras Y) para as Colunas
    // Definir o topo e o fundo baseados na margem Y.
    const float startY = marginY + buttonHeight / 2.0f;
    const float endY = screenH - (marginY + buttonHeight / 2.0f);

    // Dados das colunas
    const std::vector<std::pair<std::string, Game::GameSubject>> col1Data = {
        {"INF 250", Game::GameSubject::INF250},
        {"INF 220", Game::GameSubject::INF220},
        {"INF 330", Game::GameSubject::INF330},
        {"INF 332", Game::GameSubject::INF332}
    };

    const std::vector<std::pair<std::string, Game::GameSubject>> col2Data = {
        {"INF 420", Game::GameSubject::INF420},
        {"BIOINF", Game::GameSubject::BIOINF},
        {"INF394", Game::GameSubject::INF394},
        {"VISCPP", Game::GameSubject::VISCCP}
    };

    // 4. Cálculo do Passo Vertical (Step Y)
    // Para que as colunas fiquem alinhadas, precisamos saber qual tem mais itens.
    // (Programando defensivamente. Atualmente as duas tem 4 itens, mas vai que né)
    size_t maxRows = std::max(col1Data.size(), col2Data.size());

    // Evita divisão por zero se as listas estiverem vazias ou tiverem apenas 1 item
    float stepY = 0.0f;
    if (maxRows > 1) {
        stepY = (endY - startY) / static_cast<float>(maxRows - 1);
    }

    // ---------------------------------------------------------
    // CRIAÇÃO DOS BOTÕES
    // ---------------------------------------------------------

    // Botão Solitário da Esquerda (INF 213) - Centralizado verticalmente na tela
    CreateButton("INF 213", Game::INF213, Vector2(startX, screenH / 2.0f));

    // Função Lambda para criar colunas com a nova lógica vertical
    auto CreateColumn = [&](const auto& data, const float xPos) {
        for (size_t i = 0; i < data.size(); i++) {
            // Lerp vertical
            // Se só tiver 1 item, coloca no startY. Se tiver mais, distribui.
            const float yPos = (maxRows > 1) ? (startY + stepY * i) : startY;

            CreateButton(data[i].first, data[i].second, Vector2(xPos, yPos));
        }
    };

    // Criar Coluna 1 (Posição X: Início + 1 passo)
    CreateColumn(col1Data, startX + stepX);

    // Criar Coluna 2 (Posição X: Início + 2 passos)
    CreateColumn(col2Data, startX + (stepX * 2.0f));

    // Botão Solitário da Direita (TCC) - Centralizado verticalmente na tela
    CreateButton("TCC", Game::GameSubject::TCC, Vector2(endX, screenH / 2.0f));

    // 5. Inicialização da Seleção
    if (!mButtonObservers.empty()) {
        // Validação de segurança: sempre cheque ponteiros brutos em vetores
        if (mButtonObservers[0]) {
            mButtonObservers[0]->ChangeSelected();
            mSelectedSubject = mButtonObservers[0]->GetSubject();
        }
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
        mGame->RequestSceneChange(SceneType::Battle);
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

    return currSelected;
}
size_t StageSelect::HandleRightInput(const size_t currSelected) const {
    if (!IsInBorder(currSelected)) {
        return Math::Min<size_t>(currSelected + 4, NUM_STAGES - 1);
    }
    if (currSelected == 0) {
        return Random::GetIntRange(2, 3);
    }

    return currSelected;
}



bool StageSelect::IsInBorder(size_t currSelected) const {

    return currSelected == 0 || currSelected == NUM_STAGES - 1;

}

void StageSelect::OnUpdate(float deltaTime) {

    if(mInputTimer < INPUT_DELAY) mInputTimer += deltaTime;

}
