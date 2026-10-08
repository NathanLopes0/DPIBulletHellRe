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
    aluno->SetPosition(Vector2(230.f, 52.f));
    const bool identificado = mGame->ProgressoEGravado();
    const std::string rotulo = identificado
        ? ("Matricula: " + Matricula::ParaExibir(mGame->MatriculaAtual()))
        : std::string("Visitante - nao salva");
    // AJUSTADO AO TEXTO, como o titulo e a maior nota. Sem isso o rotulo era
    // ESTICADO ate preencher a caixa, entao a letra mudava de tamanho conforme
    // o texto: "Visitante - nao salva" saia espremido e a matricula, gigante.
    const auto rotuloDc = aluno->AddComponent<DrawTextComponent>(
        rotulo, mStageSelectFont.get(), 400, 44, 30, 255);
    rotuloDc->SetLarguraDeQuebra(400);
    rotuloDc->SetAjustarAoTexto(true);
    mAlunoAtor = aluno.get();
    AddActor(std::move(aluno));

    // QUANTO DO CURSO JA FOI, logo abaixo de quem esta jogando. E a unica coisa
    // nesta tela que fala do conjunto: o resto fala sempre da materia em foco.
    const Materias::Lista& materias = Materias::Carregadas();
    int aprovadas = 0;
    for (int i = 0; i < materias.Quantas(); ++i) {
        if (mGame->Aprovado(i)) ++aprovadas;
    }

    auto avanco = std::make_unique<Actor>(this);
    avanco->SetPosition(Vector2(230.f, 96.f));
    const auto avancoDc = avanco->AddComponent<DrawTextComponent>(
        "Aprovadas: " + std::to_string(aprovadas) + " de " + std::to_string(materias.Quantas()),
        mStageSelectFont.get(), 300, 30, 22, 255);
    avancoDc->SetLarguraDeQuebra(300);
    avancoDc->SetAjustarAoTexto(true);
    avancoDc->SetColor(Color::LightBlue);
    AddActor(std::move(avanco));

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
                        {Painel::Botao::Tres, "ranking"}}, mGame->JeitoDoRodape()),
        mStageSelectFont.get(), 1100, 34, 22, 255);
    rodape->SetLarguraDeQuebra(1100);
    rodape->SetAjustarAoTexto(true);
    mTrocarAtor = trocar.get();
    AddActor(std::move(trocar));
}

void StageSelect::CreateStageButtons() {
    // 1. Definições de Tamanho e Bordas
    constexpr float buttonWidth = 128.0f;

    // Obtendo dimensões da tela uma única vez para clareza
    const auto screenW = static_cast<float>(mGame->GetWindowWidth());
    const auto screenH = static_cast<float>(mGame->GetWindowHeight());

    // Margens (1/12 da tela)
    const float marginX = screenW / 12.0f;

    // 2. Definindo os Limites Horizontais (Âncoras X)
    // Onde começa o layout (lado esquerdo) e onde termina (lado direito)
    const float startX = marginX + buttonWidth / 2.0f;
    const float endX = screenW - (marginX + buttonWidth / 2.0f);

    // 3. Definindo os Limites Verticais (Âncoras Y) para as Colunas
    //
    // A GRADE NAO OCUPA MAIS A TELA INTEIRA. Com a margem de 1/12 ela ia de 100
    // a 700 e encostava nas duas pontas: o titulo nao tinha onde ficar em cima e
    // a linha de estado batia nos losangos de baixo. Agora o topo e o pe da tela
    // sao de quem escreve, e a grade vive entre os dois.
    const float startY = screenH * kTopoDaGrade;
    const float endY = screenH * kFundoDaGrade;

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
                                   ? (startY + endY) / 2.0f
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
    button->SetAprovado(mGame->Aprovado(subject));

    if (!unlocked) {
        // O NOME DA MATERIA FECHADA, EMBAIXO DO LOSANGO. Dentro dele nao cabe:
        // o quadro de materia fechada tem correntes e um cadeado desenhados no
        // centro, bem onde o texto cairia. Sem nome nenhum, que era como estava,
        // a grade virava uma fileira de cadeados iguais e o aluno nao via para
        // onde o curso ia - nem que materia tinha acabado de selecionar.
        auto rotulo = std::make_unique<Actor>(this);
        rotulo->SetPosition(position + Vector2(0.0f, kNomeAbaixoDoCadeado));
        const auto dc = rotulo->AddComponent<DrawTextComponent>(
            text, mStageSelectFont.get(), 150, 26, 18, 255);
        dc->SetLarguraDeQuebra(150);
        dc->SetAjustarAoTexto(true);

        // Quem acende o rotulo ao ganhar o foco e o proprio botao, que e quem
        // sabe da selecao. Ele so observa; o ator e da cena, como todos.
        button->SetRotuloFechado(rotulo.get());
        AddActor(std::move(rotulo));
    }

    mButtonObservers.push_back(button.get());
    this->AddActor(std::move(button));
}

void StageSelect::CreateStaticUI() {

    const auto w = static_cast<float>(mGame->GetWindowWidth());
    const auto h = static_cast<float>(mGame->GetWindowHeight());

    // O TITULO fica ENTRE a matricula (que ocupa 30..410) e a maior nota (que
    // ocupa 900..1100), na faixa que a grade desocupou. A caixa de 360 e o que
    // cabe entre as duas sem encostar em nenhuma; o ajuste ao texto encolhe a
    // letra se ela passar disso, em vez de invadir os cantos.
    auto tituloAtor = std::make_unique<Actor>(this);
    tituloAtor->SetPosition(Vector2(w / 2.0f, 52.f));
    const auto titulo = tituloAtor->AddComponent<DrawTextComponent>(
        "ESCOLHA A MATERIA", mStageSelectFont.get(), 360, 44, 30, 255);
    titulo->SetLarguraDeQuebra(360);
    titulo->SetAjustarAoTexto(true);
    AddActor(std::move(tituloAtor));

    auto infoActor = std::make_unique<Actor>(this);
    infoActor->SetPosition(Vector2(w - 190.f, 52.f));

    // Tambem ajustada ao texto: na caixa esticada, "Maior Nota: 0.00" e "Maior
    // Nota: 100.00" saiam com letras de tamanhos diferentes, e as vezes a
    // segunda quebrava em duas linhas - o numero dancava a cada seta apertada.
    const auto notaDc = infoActor->AddComponent<DrawTextComponent>(
        "Maior Nota: --", mStageSelectFont.get(), 320, 44, 30, 255);
    notaDc->SetLarguraDeQuebra(320);
    notaDc->SetAjustarAoTexto(true);
    mScoreInfoActor = infoActor.get();
    AddActor(std::move(infoActor));

    // A LINHA DE ESTADO, entre a grade e o rodape. E o que a tela nao dizia:
    // por que a materia esta fechada, e que seis delas ainda nao tem professor.
    auto estadoAtor = std::make_unique<Actor>(this);
    estadoAtor->SetPosition(Vector2(w / 2.0f, h * kLinhaDeEstado));
    const auto estado = estadoAtor->AddComponent<DrawTextComponent>(
        " ", mStageSelectFont.get(), 900, 32, 24, 255);
    estado->SetLarguraDeQuebra(900);   // ver o rodape: o padrao e 500
    estado->SetAjustarAoTexto(true);
    mEstadoAtor = estadoAtor.get();
    AddActor(std::move(estadoAtor));

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
        if (Nota::ECheia(highScore))                 dc->SetColor(Color::Gold);
        else if (highScore >= Nota::kNotaAprovacao)  dc->SetColor(Color::LightGreen);
        else if (highScore > 0.0f)                   dc->SetColor(Color::LightPink);
        else                                         dc->SetColor(Color::White);
    }

    AtualizarLinhaDeEstado();
}

void StageSelect::AtualizarLinhaDeEstado() const {

    if (!mEstadoAtor) return;
    const auto dc = mEstadoAtor->GetComponent<DrawTextComponent>();
    if (!dc) return;

    const Materias::Lista& materias = Materias::Carregadas();
    const Materias::Materia* m = materias.Por(mSelectedSubject);

    std::string frase;
    Vector3 cor = Color::White;

    if (!mGame->IsStageUnlocked(mSelectedSubject)) {
        // A FRASE VEM DA MESMA REGRA QUE FECHOU A MATERIA (Materias::ExigenciaDe),
        // e nao de um texto escrito aqui: se viesse daqui, mudar o desbloqueio no
        // JSON deixaria a tela explicando a regra antiga.
        frase = materias.ExigenciaDe(mSelectedSubject);
        if (frase.empty()) frase = "Ainda fechada";
        cor = Color::LightPink;

    } else if (m && m->chefe.empty()) {
        // Seis das onze materias ainda nao tem chefe. Sem este aviso, apertar o
        // botao abre a batalha, ela nao acha a fabrica e volta sozinha - e o
        // aluno ve a tela piscar sem entender o que fez de errado.
        frase = "Ainda sem professor";
        cor = Color::LightBlue;

    } else {
        const float nota = mGame->GetMelhorNota(mSelectedSubject);
        if (Nota::ECheia(nota))                 { frase = "Aprovado com nota cheia"; cor = Color::Gold; }
        else if (nota >= Nota::kNotaAprovacao)  { frase = "Aprovado";                cor = Color::LightGreen; }
        else if (nota > 0.0f)                   { frase = "Ainda nao passou";        cor = Color::LightPink; }
        else                                    { frase = "Ainda nao jogou";         cor = Color::White; }
    }

    dc->SetText(frase);
    dc->SetColor(cor);
}
