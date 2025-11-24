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
// --------------------------------------------------------------------------
// LÓGICA DE NAVEGAÇÃO (GRID SYSTEM)
// --------------------------------------------------------------------------

int StageSelect::GetColumnFromIndex(size_t index) const {
    // Coluna 0: Botão Esquerdo (Apenas índice 0)
    if (index == 0) return 0;

    // Constantes dinâmicas baseadas no tamanho dos vetores
    const size_t col1Size = 4; // Ou mCol1Data.size() se eu quiser tornar membro
    const size_t col2Size = 4; // Ou mCol2Data.size()

    // Coluna 1: Indices 1 até 4
    if (index <= col1Size) return 1;

    // Coluna 2: Indices 5 até 8
    if (index <= col1Size + col2Size) return 2;

    // Coluna 3: O resto (Botão Direito)
    return 3;
}

/* Qual índice do vetor começa a coluna colIndex?
 */
size_t StageSelect::GetColumnStartIndex(int colIndex) const {
    const size_t col1Size = 4;
    const size_t col2Size = 4;

    switch (colIndex) {
        case 0: return 0;
        case 1: return 1;
        case 2: return 1 + col1Size;
        case 3: return 1 + col1Size + col2Size;
        default: return 0;
    }
}

/* Qual o tamanho da coluna colIndex?
 */
size_t StageSelect::GetColumnSize(int colIndex) const {
    const size_t col1Size = 4;
    const size_t col2Size = 4;

    switch (colIndex) {
        case 0: return 1;
        case 1: return col1Size;
        case 2: return col2Size;
        case 3: return 1;
        default: return 0;
    }
}

// --------------------------------------------------------------------------

size_t StageSelect::HandleUpInput(const size_t currSelected) const {
    int col = GetColumnFromIndex(currSelected);
    size_t colStart = GetColumnStartIndex(col);
    size_t colSize = GetColumnSize(col);

    // Se a coluna só tem 1 item (bordas), cima/baixo não faz nada
    if (colSize <= 1) return currSelected;

    // Lógica Cíclica:
    // Posição relativa dentro da coluna (0 a N-1)

    // Se for o primeiro (0), vai para o último (Size - 1)
    if (const size_t relativeIndex = currSelected - colStart; relativeIndex == 0) {
        return colStart + (colSize - 1);
    }

    return currSelected - 1;
}

size_t StageSelect::HandleDownInput(const size_t currSelected) const {
    int col = GetColumnFromIndex(currSelected);
    size_t colStart = GetColumnStartIndex(col);
    size_t colSize = GetColumnSize(col);

    if (colSize <= 1) return currSelected;

    // Se for o último, volta para o primeiro
    if (size_t relativeIndex = currSelected - colStart; relativeIndex == colSize - 1) {
        return colStart;
    }

    return currSelected + 1;
}

size_t StageSelect::HandleLeftInput(const size_t currSelected) const {
    int col = GetColumnFromIndex(currSelected);

    // Se já estamos na extrema esquerda, ir para a extrema direita
    // Posso bloquear tbm.. mas vou fazer ir pro outro lado para ficar fluido.
    if (col == 0) return NUM_STAGES - 1;

    int targetCol = col - 1;
    size_t targetStart = GetColumnStartIndex(targetCol);
    size_t targetSize = GetColumnSize(targetCol);

    // Agora calculamos para qual ALTURA vamos.
    // Se estou saindo de uma lista grande para uma pequena (ex: Col 1 -> Col 0), vou para o meio.
    // Se estou saindo de uma lista igual para igual (Col 2 -> Col 1), mantenho a linha.

    size_t currentStart = GetColumnStartIndex(col);
    size_t currentRow = currSelected - currentStart; // Linha atual (0, 1, 2...)

    if (targetSize == 1) {
        // Indo para um botão solitário (centro vertical)
        return targetStart;
    }

    // Indo para uma coluna com vários itens.
    // Tentamos manter o mesmo índice de linha (currentRow).
    // Mas se a coluna destino for menor, usamos clamp (Math::Min).
    size_t targetRow = std::min(currentRow, targetSize - 1);

    // Caso especial: Se viemos de um botão solitário (Col 3 -> Col 2),
    // queremos ir para o MEIO da lista, não para o topo.
    size_t currentSize = GetColumnSize(col);
    if (currentSize == 1 && targetSize > 1) {
        targetRow = targetSize / 2 - 1; // Vai para o meio
    }

    return targetStart + targetRow;
}

size_t StageSelect::HandleRightInput(const size_t currSelected) const {
    int col = GetColumnFromIndex(currSelected);

    // Se estamos na última coluna, volta para a primeira (Wrap)
    if (col == 3) return 0;

    int targetCol = col + 1;
    size_t targetStart = GetColumnStartIndex(targetCol);
    size_t targetSize = GetColumnSize(targetCol);

    size_t currentStart = GetColumnStartIndex(col);
    size_t currentRow = currSelected - currentStart;

    // Lógica simétrica ao LeftInput
    if (targetSize == 1) {
        return targetStart;
    }

    size_t targetRow = std::min(currentRow, targetSize - 1);

    // Se saímos do botão solitário da esquerda (Col 0) para a Col 1,
    // queremos cair no meio da lista.
    size_t currentSize = GetColumnSize(col);
    if (currentSize == 1 && targetSize > 1) {
        targetRow = targetSize / 2 - 1;
    }

    return targetStart + targetRow;
}



bool StageSelect::IsInBorder(size_t currSelected) const {

    return currSelected == 0 || currSelected == NUM_STAGES - 1;

}

void StageSelect::OnUpdate(float deltaTime) {

    if(mInputTimer < INPUT_DELAY) mInputTimer += deltaTime;

}
