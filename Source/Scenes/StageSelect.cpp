//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"

#include "../Matricula.h"
#include "../MateriasArquivo.h"
#include <sstream>
#include <iomanip>
#include "../Math.h"
#include "../Random.h"
#include "../Actors/Buttons/StageSelectButton.h"
#include "../Font.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

#include "../CaminhosArquivo.h"

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect)
                                       , mStageSelectFont(std::make_unique<Font>())
                                       , mInputTimer(INPUT_DELAY)
{

    mStageSelectFont->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
    mButtonObservers.reserve(NUM_STAGES);

}

void StageSelect::Load() {

    CreateStageButtons();
    CreateStaticUI();
    CriarIdentificacaoNaTela();

}

void StageSelect::CriarIdentificacaoNaTela() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    // Quem esta jogando, no canto de cima. Numa maquina compartilhada isto nao e
    // enfeite: e como o aluno confere que nao esta jogando na ficha do colega que
    // usou antes dele.
    auto aluno = std::make_unique<Actor>(this);
    aluno->SetPosition(Vector2(220.f, 50.f));
    const bool identificado = mGame->ProgressoEGravado();
    const std::string rotulo = identificado
        ? ("Matricula: " + Matricula::ParaExibir(mGame->MatriculaAtual()))
        : std::string("Visitante - nao salva");
    aluno->AddComponent<DrawTextComponent>(rotulo, mStageSelectFont.get(), 380, 60, 72, 255);
    mAlunoAtor = aluno.get();
    AddActor(std::move(aluno));

    auto trocar = std::make_unique<Actor>(this);
    trocar->SetPosition(Vector2(largura / 2.0f, static_cast<float>(mGame->GetWindowHeight()) - 40.f));
    trocar->AddComponent<DrawTextComponent>("T - trocar usuario", mStageSelectFont.get(),
                                            300, 40, 48, 255);
    mTrocarAtor = trocar.get();
    AddActor(std::move(trocar));
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

    // 3. Definindo os Limites Verticais (Âncoras Y) para as Colunas
    // Definir o topo e o fundo baseados na margem Y.
    const float startY = marginY + buttonHeight / 2.0f;
    const float endY = screenH - (marginY + buttonHeight / 2.0f);

    // A GRADE VEM DE materias.json, e nao de listas escritas aqui. Antes os
    // nomes dos botoes, as colunas e quem ficava em cada uma estavam repetidos
    // neste arquivo, ja declarados no JSON - e acrescentar uma materia exigia
    // acertar os dois, sem nada avisando se um ficasse para tras.
    const Materias::Lista& materias = Materias::Carregadas();
    const int quantasColunas = materias.QuantasColunas();

    // O passo horizontal supoe intervalos entre colunas, e nao colunas: com
    // quatro colunas sao tres intervalos. Com uma coluna so nao ha intervalo
    // nenhum, e dividir por zero poria os botoes no infinito.
    const float stepX = (quantasColunas > 1)
                            ? (endX - startX) / static_cast<float>(quantasColunas - 1)
                            : 0.0f;

    // 4. Cálculo do Passo Vertical (Step Y)
    // Para que as colunas fiquem alinhadas, precisamos saber qual tem mais itens.
    size_t maxRows = 0;
    for (int c = 0; c < quantasColunas; ++c) {
        maxRows = std::max(maxRows, materias.DaColuna(c).size());
    }

    // Evita divisão por zero se as listas estiverem vazias ou tiverem apenas 1 item
    float stepY = 0.0f;
    if (maxRows > 1) {
        stepY = (endY - startY) / static_cast<float>(maxRows - 1);
    }

    // ---------------------------------------------------------
    // CRIAÇÃO DOS BOTÕES
    // ---------------------------------------------------------

    for (int coluna = 0; coluna < quantasColunas; ++coluna) {

        const std::vector<int> daColuna = materias.DaColuna(coluna);
        if (daColuna.empty()) continue;

        const float xPos = startX + stepX * static_cast<float>(coluna);

        for (size_t i = 0; i < daColuna.size(); ++i) {

            // Coluna de uma materia so fica CENTRADA na vertical; com mais de
            // uma, elas se distribuem no mesmo passo de todas as colunas, para
            // as linhas ficarem alinhadas lado a lado. E o que fazia os botoes
            // solitarios do INF 213 e do TCC parecerem centrados de proposito.
            const float yPos = (daColuna.size() == 1)
                                   ? screenH / 2.0f
                                   : startY + stepY * static_cast<float>(i);

            const Materias::Materia* m = materias.Por(daColuna[i]);
            if (m == nullptr) continue;

            CreateButton(m->nome, (daColuna[i]),
                         Vector2(xPos, yPos));
        }
    }

    // 5. Inicialização da Seleção
    if (!mButtonObservers.empty()) {
        // Validação de segurança: sempre cheque ponteiros brutos em vetores
        if (mButtonObservers[0]) {
            mButtonObservers[0]->ChangeSelected();
            mSelectedSubject = mButtonObservers[0]->GetSubject();
        }
    }
}

void StageSelect::CreateButton(const std::string& text, int subject, const Vector2& position) {

    bool unlocked = mGame->IsStageUnlocked(subject);
    auto button = std::make_unique<StageSelectButton>(this, text, subject, Caminhos::Asset("Fonts/Zelda.ttf"), !unlocked);
    button->SetPosition(position);

    mButtonObservers.push_back(button.get());
    this->AddActor(std::move(button));
}

void StageSelect::CreateStaticUI() {
    auto infoActor = std::make_unique<Actor>(this);

    const auto w = static_cast<float>(mGame->GetWindowWidth());
    infoActor->SetPosition(Vector2(w - 200.f, 50.f));

    infoActor->AddComponent<DrawTextComponent>("Maior Nota: --", mStageSelectFont.get(), 200, 60, 72, 255);
    mScoreInfoActor = infoActor.get();
    AddActor(std::move(infoActor));

    UpdateStageInfo();
}
void StageSelect::OnProcessInput(const Uint8 *keyState) {

    // Trocar de usuario volta para a identificacao. Nao grava nada aqui: a ficha
    // do aluno que esta saindo ja foi para o disco ao fim de cada batalha, entao
    // nao ha o que perder - e sair sem ter jogado nao deveria criar arquivo.
    const bool trocar = keyState[SDL_SCANCODE_T];
    if (trocar && !mTrocarAnterior) {
        mTrocarAnterior = true;
        mGame->RequestSceneChange(SceneType::Identificacao);
        return;
    }
    mTrocarAnterior = trocar;

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
        UpdateStageInfo();
    }

    // Precisa ser uma batida NOVA do ENTER, nao o estado dele: ver mEntrarAnterior.
    const bool entrar = keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (entrar && !mEntrarAnterior && mGame->IsStageUnlocked(mSelectedSubject)) {
        mEntrarAnterior = true;
        mGame->SetSelectedStage(mSelectedSubject);
        mGame->RequestSceneChange(SceneType::Battle);
        return;
    }
    mEntrarAnterior = entrar;
}

size_t StageSelect::HandleSelectedChange(const Uint8 *keyState, size_t currSelected) {
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

int StageSelect::GetColumnFromIndex(size_t index) {
    // Coluna 0: Botão Esquerdo (Apenas índice 0)
    if (index == 0) return 0;

    // Constantes dinâmicas baseadas no tamanho dos vetores
    constexpr size_t col1Size = 4; // Ou mCol1Data.size() se eu quiser tornar membro
    constexpr size_t col2Size = 4; // Ou mCol2Data.size()

    // Coluna 1: Indices 1 até 4
    if (index <= col1Size) return 1;

    // Coluna 2: Indices 5 até 8
    if (index <= col1Size + col2Size) return 2;

    // Coluna 3: O resto (Botão Direito)
    return 3;
}

/* Qual índice do vetor começa a coluna colIndex?
 */
size_t StageSelect::GetColumnStartIndex(int colIndex) {
    constexpr size_t col1Size = 4;
    constexpr size_t col2Size = 4;

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
size_t StageSelect::GetColumnSize(const int colIndex) {
    constexpr size_t col1Size = 4;
    constexpr size_t col2Size = 4;

    switch (colIndex) {
        case 0: return 1;
        case 1: return col1Size;
        case 2: return col2Size;
        case 3: return 1;
        default: return 0;
    }
}

// --------------------------------------------------------------------------

size_t StageSelect::HandleUpInput(const size_t currSelected) {
    const int col = GetColumnFromIndex(currSelected);
    const size_t colStart = GetColumnStartIndex(col);
    const size_t colSize = GetColumnSize(col);

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

size_t StageSelect::HandleDownInput(const size_t currSelected) {
    const int col = GetColumnFromIndex(currSelected);
    const size_t colStart = GetColumnStartIndex(col);
    const size_t colSize = GetColumnSize(col);

    if (colSize <= 1) return currSelected;

    // Se for o último, volta para o primeiro
    if (const size_t relativeIndex = currSelected - colStart; relativeIndex == colSize - 1) {
        return colStart;
    }

    return currSelected + 1;
}

size_t StageSelect::HandleLeftInput(const size_t currSelected) {
    const int col = GetColumnFromIndex(currSelected);

    // Se já estamos na extrema esquerda, ir para a extrema direita
    // Posso bloquear tbm.. mas vou fazer ir pro outro lado para ficar fluido.
    if (col == 0) return NUM_STAGES - 1;

    const int targetCol = col - 1;
    const size_t targetStart = GetColumnStartIndex(targetCol);
    const size_t targetSize = GetColumnSize(targetCol);

    // Agora calculamos para qual ALTURA vamos.
    // Se estou saindo de uma lista grande para uma pequena (ex: Col 1 -> Col 0), vou para o meio.
    // Se estou saindo de uma lista igual para igual (Col 2 -> Col 1), mantenho a linha.

    const size_t currentStart = GetColumnStartIndex(col);
    const size_t currentRow = currSelected - currentStart; // Linha atual (0, 1, 2...)

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
    if (const size_t currentSize = GetColumnSize(col); currentSize == 1 && targetSize > 1) {
        targetRow = targetSize / 2 - 1; // Vai para o meio
    }

    return targetStart + targetRow;
}

size_t StageSelect::HandleRightInput(const size_t currSelected) {
    const int col = GetColumnFromIndex(currSelected);

    // Se estamos na última coluna, volta para a primeira (Wrap)
    if (col == 3) return 0;

    const int targetCol = col + 1;
    const size_t targetStart = GetColumnStartIndex(targetCol);
    const size_t targetSize = GetColumnSize(targetCol);

    const size_t currentStart = GetColumnStartIndex(col);
    const size_t currentRow = currSelected - currentStart;

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



bool StageSelect::IsInBorder(const size_t currSelected) {

    return currSelected == 0 || currSelected == NUM_STAGES - 1;
}

void StageSelect::OnUpdate(float deltaTime) {

    if(mInputTimer < INPUT_DELAY) mInputTimer += deltaTime;

}

void StageSelect::UpdateStageInfo() const {
    if (!mScoreInfoActor) return;

    const int subject = mSelectedSubject;
    // Agora e de fato o recorde: antes mostrava a ULTIMA nota, entao uma
    // tentativa ruim baixava o numero que se chamava highScore.
    const float highScore = mGame->GetMelhorNota(subject);

    std::stringstream ss;
    // Duas casas pelo mesmo motivo da barra de nota da batalha: o teto por dano
    // e 99,99, e com uma casa ele viraria "100.0" na tela do recorde.
    ss << "Maior Nota: " << std::fixed << std::setprecision(2) << highScore;

    if (auto dc = mScoreInfoActor->GetComponent<DrawTextComponent>()) {
        dc->SetText(ss.str());

        // TODO - mudar cor quando criar o SetColor em DrawTextComponent
        /*
        if (highScore >= 60.0f) {
            dc->SetColor(0, 255, 0, 255); // Verde (Aprovado)
        } else if (highScore > 0.0f) {
            dc->SetColor(255, 100, 100, 255); // Vermelho (Reprovado)
        } else {
            dc->SetColor(200, 200, 200, 255); // Cinza (Nunca jogou)
        }
        */
    }



}
