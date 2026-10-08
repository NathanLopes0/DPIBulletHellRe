//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "../Painel.h"

#include "../Matricula.h"
#include "../MateriasArquivo.h"
#include "../Nota.h"
#include "../Navegacao.h"
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
    mButtonObservers.reserve(RESERVA_DE_BOTOES);

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
    // O RODAPE SAI DOS BOTOES, e nao de texto solto: ver Painel.h. Era aqui
    // que estava escrito "T trocar usuario", numa maquina sem tecla T.
    // A QUEBRA PRECISA ACOMPANHAR A LARGURA. O padrao de 500 px quebrava esta
    // linha em duas, e o ajuste ao texto ainda as encolhia para caber na altura
    // de uma - ficava "trocar usuario" solto embaixo e um "T" perdido na ponta.
    const auto rodape = trocar->AddComponent<DrawTextComponent>(
        Painel::Rodape({{Painel::Botao::Um,   "jogar"},
                        {Painel::Botao::Dois, "trocar aluno"},
                        {Painel::Botao::Tres, "ranking da materia"}}),
        mStageSelectFont.get(), 1100, 34, 22, 255);
    rodape->SetLarguraDeQuebra(1100);
    rodape->SetAjustarAoTexto(true);
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

    // A grade da navegacao e montada AQUI, no mesmo laco que posiciona os botoes,
    // e nao numa descricao a parte: assim nao existe uma segunda ideia do layout
    // que possa discordar da primeira. Ver o comentario em Navegacao.h.
    mGrade.assign(static_cast<size_t>(quantasColunas), {});

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

            // O indice do botao que CreateButton vai criar e a posicao em que ele
            // entra em mButtonObservers - por isso a grade e anotada antes.
            mGrade[static_cast<size_t>(coluna)].push_back(mButtonObservers.size());

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
    // O ESC continua valendo para quem desenvolve, mas nao aparece escrito:
    // o gabinete nao tem ESC, e o rodape so anuncia o que existe no painel.
    const bool trocar = Painel::Apertado(keyState, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
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

    // O RANKING DA MATERIA EM FOCO. Abre direto na materia certa: perguntar
    // "como fui nesta?" acontece olhando para ela, e nao no menu principal.
    const bool verRanking = Painel::Apertado(keyState, Painel::Botao::Tres);
    if (verRanking && !mRankingAnterior) {
        mRankingAnterior = true;
        mGame->PedirRanking(Game::PedidoDeRanking{false, mSelectedSubject,
                                                  SceneType::StageSelect});
        mGame->RequestSceneChange(SceneType::Ranking);
        return;
    }
    mRankingAnterior = verRanking;

    // Precisa ser uma batida NOVA, e nao o estado: ver mEntrarAnterior.
    const bool entrar = Painel::Apertado(keyState, Painel::Botao::Um)
                     || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (entrar && !mEntrarAnterior && mGame->IsStageUnlocked(mSelectedSubject)) {
        mEntrarAnterior = true;
        mGame->SetSelectedStage(mSelectedSubject);
        mGame->RequestSceneChange(SceneType::Battle);
        return;
    }
    mEntrarAnterior = entrar;
}

// --------------------------------------------------------------------------
// NAVEGACAO
//
// A REGRA MORA EM Navegacao, E A FORMA DA GRADE VEM DE QUEM DESENHOU OS BOTOES.
// Aqui havia tres funcoes que descreviam a grade de novo, a mao - "a coluna 1
// tem 4 itens, a 2 comeca no indice 5" - mais um NUM_STAGES fixo em 10. Era uma
// segunda descricao do mesmo layout, e quando o INF 110 entrou e os botoes
// passaram a 11 em cinco colunas, so os botoes acompanharam: a seta andava por
// uma tela que nao existia mais.
// --------------------------------------------------------------------------

size_t StageSelect::HandleSelectedChange(const Uint8 *keyState, const size_t currSelected) const {

    if (keyState[SDL_SCANCODE_UP])          return Navegacao::Cima(mGrade, currSelected);
    if (keyState[SDL_SCANCODE_DOWN])        return Navegacao::Baixo(mGrade, currSelected);
    if (keyState[SDL_SCANCODE_LEFT])        return Navegacao::Esquerda(mGrade, currSelected);
    if (keyState[SDL_SCANCODE_RIGHT])       return Navegacao::Direita(mGrade, currSelected);

    return currSelected;
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

        // O plano que estava aqui como TODO, agora que DrawTextComponent tem cor.
        // Ficava comentado esperando por isso, e com uma assinatura de quatro
        // Uint8 que nunca chegou a existir - descomentar nao compilaria.
        //
        // O DOURADO ENTRA AQUI TAMBEM, e nao so na batalha: esta e a tela onde um
        // aluno compara a propria nota com a dos outros, entao e onde a nota cheia
        // mais precisa se distinguir do 99,99 de quem levou dano.
        if (Nota::ECheia(highScore))   dc->SetColor(Color::Gold);
        else if (highScore >= 60.0f)   dc->SetColor(Color::LightGreen);
        else if (highScore > 0.0f)     dc->SetColor(Color::LightPink);
        else                           dc->SetColor(Color::White);
    }



}
