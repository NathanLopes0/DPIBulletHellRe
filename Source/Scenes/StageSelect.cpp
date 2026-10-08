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
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"

#include "../CaminhosArquivo.h"

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect)
                                       , mStageSelectFont(std::make_unique<Font>())
                                       , mInputTimer(INPUT_DELAY)
{

    mStageSelectFont->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
    mButtonObservers.reserve(RESERVA_DE_BOTOES);

}

void StageSelect::Load() {

    CriarFundo();
    CreateStageButtons();
    CriarLigacoes();          // depois dos botoes: as linhas saem das posicoes deles
    CreateStaticUI();
    CriarIdentificacaoNaTela();

}

void StageSelect::CriarFundo() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura = static_cast<float>(mGame->GetWindowHeight());

    // O MESMO CORREDOR DO MENU. A tela era preta: os losangos flutuavam no
    // vazio e nada dizia que aquilo era a mesma sala do resto do jogo. Com a
    // foto, a selecao passa a acontecer EM ALGUM LUGAR.
    auto fundo = std::make_unique<Actor>(this);
    fundo->SetPosition(Vector2(largura / 2.0f, altura / 2.0f));
    const auto foto = fundo->AddComponent<DrawSpriteComponent>(
        Caminhos::Asset("MainMenuBackground.png"), kOrdemDoFundo);
    foto->SetColor(66, 70, 92);   // puxada para o azul e bem escurecida
    AddActor(std::move(fundo));

    // O VEU. A foto tem tijolo vermelho e janela clara; sem escurecer de novo
    // por cima, o nome de uma materia fechada cai em cima de um reflexo e some.
    auto veu = std::make_unique<Actor>(this);
    veu->SetPosition(Vector2(largura / 2.0f, altura / 2.0f));
    veu->AddComponent<DrawCaixaComponent>(
        static_cast<int>(largura), static_cast<int>(altura),
        SDL_Color{0, 0, 0, 0}, SDL_Color{6, 8, 18, 200}, 0, kOrdemDoVeu);
    AddActor(std::move(veu));
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

void StageSelect::Linha(const float x, const float y,
                        const float largura, const float altura) {

    // Uma caixa PREENCHIDA sem moldura: espessura zero deixa os quatro lados
    // com tamanho zero, entao sobra so o fundo - que e o segmento.
    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));
    ator->AddComponent<DrawCaixaComponent>(
        static_cast<int>(largura), static_cast<int>(altura),
        SDL_Color{0, 0, 0, 0}, kCorDaLigacao, 0, kOrdemDasLigacoes);
    AddActor(std::move(ator));
}

void StageSelect::CriarLigacoes() {

    // O CURSO COMO MAPA, e nao como losangos soltos. Entre duas colunas sai um
    // tronco vertical, e dele um ramo para cada materia dos dois lados - e o
    // desenho classico de grade curricular, e e a mesma forma para as duas
    // regras de desbloqueio que o curso usa.
    //
    // So desenha onde a ligacao E VERDADE: Materias::ColunaDependeDaAnterior
    // responde se toda materia da coluna da frente depende mesmo da de tras.
    // Uma materia que exigisse algo de duas colunas atras faria a linha mentir,
    // e ai a coluna fica sem tronco em vez de ganhar um errado.
    const Materias::Lista& materias = Materias::Carregadas();

    for (size_t c = 0; c + 1 < mGrade.size(); ++c) {

        if (mGrade[c].empty() || mGrade[c + 1].empty()) continue;
        if (!materias.ColunaDependeDaAnterior(static_cast<int>(c) + 1)) continue;

        const float xEsquerda = mButtonObservers[mGrade[c].front()]->GetPosition().x
                              + kMetadeDoLosango;
        const float xDireita = mButtonObservers[mGrade[c + 1].front()]->GetPosition().x
                             - kMetadeDoLosango;
        const float xTronco = (xEsquerda + xDireita) / 2.0f;

        float yMenor = mButtonObservers[mGrade[c].front()]->GetPosition().y;
        float yMaior = yMenor;
        for (const size_t lado : {c, c + 1}) {
            for (const size_t i : mGrade[lado]) {
                const float y = mButtonObservers[i]->GetPosition().y;
                yMenor = Math::Min(yMenor, y);
                yMaior = Math::Max(yMaior, y);
            }
        }

        if (yMaior > yMenor) {
            Linha(xTronco, (yMenor + yMaior) / 2.0f, kGrossuraDaLigacao, yMaior - yMenor);
        }

        for (const size_t i : mGrade[c]) {
            const float y = mButtonObservers[i]->GetPosition().y;
            Linha((xEsquerda + xTronco) / 2.0f, y, xTronco - xEsquerda, kGrossuraDaLigacao);
        }
        for (const size_t i : mGrade[c + 1]) {
            const float y = mButtonObservers[i]->GetPosition().y;
            Linha((xTronco + xDireita) / 2.0f, y, xDireita - xTronco, kGrossuraDaLigacao);
        }
    }
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

Actor* StageSelect::TextoDoCartao(const float y, const int tamanho, const Vector3& cor) {

    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(kCartaoX, y));
    const auto dc = ator->AddComponent<DrawTextComponent>(
        " ", mStageSelectFont.get(), static_cast<int>(kCartaoLargura) - 40,
        tamanho + 8, tamanho, 255);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(kCartaoLargura) - 40);
    dc->SetAjustarAoTexto(true);
    dc->SetColor(cor);

    Actor* observador = ator.get();
    AddActor(std::move(ator));
    return observador;
}

void StageSelect::CreateStaticUI() {

    const auto w = static_cast<float>(mGame->GetWindowWidth());

    // O TITULO no meio, e o avanco no curso na direita. A maior nota saiu deste
    // canto: ela fala da materia em foco, entao foi para o cartao com o resto
    // do que se sabe dela. No alto fica so o que vale para a tela inteira.
    auto tituloAtor = std::make_unique<Actor>(this);
    tituloAtor->SetPosition(Vector2(w / 2.0f, 52.f));
    const auto titulo = tituloAtor->AddComponent<DrawTextComponent>(
        "ESCOLHA A MATERIA", mStageSelectFont.get(), 360, 44, 30, 255);
    titulo->SetLarguraDeQuebra(360);
    titulo->SetAjustarAoTexto(true);
    AddActor(std::move(tituloAtor));

    const Materias::Lista& materias = Materias::Carregadas();
    int aprovadas = 0;
    for (int i = 0; i < materias.Quantas(); ++i) {
        if (mGame->Aprovado(i)) ++aprovadas;
    }

    auto avanco = std::make_unique<Actor>(this);
    avanco->SetPosition(Vector2(w - 190.f, 52.f));
    const auto avancoDc = avanco->AddComponent<DrawTextComponent>(
        "Aprovadas: " + std::to_string(aprovadas) + " de " + std::to_string(materias.Quantas()),
        mStageSelectFont.get(), 320, 38, 26, 255);
    avancoDc->SetLarguraDeQuebra(320);
    avancoDc->SetAjustarAoTexto(true);
    avancoDc->SetColor(Color::LightBlue);
    AddActor(std::move(avanco));

    // O CARTAO DA MATERIA EM FOCO, no canto que a grade deixa vazio. Tudo o que
    // se sabe da materia selecionada mora aqui dentro, em vez de ficar espalhado
    // em textos soltos pela tela.
    auto moldura = std::make_unique<Actor>(this);
    moldura->SetPosition(Vector2(kCartaoX, kCartaoY));
    moldura->AddComponent<DrawCaixaComponent>(
        static_cast<int>(kCartaoLargura), static_cast<int>(kCartaoAltura),
        kCorDaMolduraDoCartao, kCorDoFundoDoCartao, 2, kOrdemDoCartao);
    AddActor(std::move(moldura));

    mCodigoAtor    = TextoDoCartao(kCartaoY - 58.f, 30, Color::White);
    mNomeAtor      = TextoDoCartao(kCartaoY - 16.f, 22, Color::LightYellow);
    mProfessorAtor = TextoDoCartao(kCartaoY + 20.f, 20, Color::LightBlue);
    mEstadoAtor    = TextoDoCartao(kCartaoY + 58.f, 22, Color::White);

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

void StageSelect::EscreverNoCartao(Actor* ator, const std::string& texto, const Vector3& cor) {

    if (!ator) return;
    if (const auto dc = ator->GetComponent<DrawTextComponent>()) {
        // Espaco, e nao vazio: o componente precisa de alguma coisa para medir,
        // e texto vazio deixaria a textura anterior na tela.
        dc->SetText(texto.empty() ? std::string(" ") : texto);
        dc->SetColor(cor);
    }
}

void StageSelect::UpdateStageInfo() const {

    const Materias::Lista& materias = Materias::Carregadas();
    const Materias::Materia* m = materias.Por(mSelectedSubject);

    EscreverNoCartao(mCodigoAtor, m ? m->nome : std::string(), Color::White);
    EscreverNoCartao(mNomeAtor, m ? m->nomeCompleto : std::string(), Color::LightYellow);
    EscreverNoCartao(mProfessorAtor, (m && !m->professor.empty()) ? ("Prof. " + m->professor)
                                                                 : std::string(),
                     Color::LightBlue);

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
        // Seis das onze materias ainda nao tem batalha. Sem este aviso, apertar
        // o botao abre a batalha, ela nao acha a fabrica e volta sozinha - e o
        // aluno ve a tela piscar sem entender o que fez de errado.
        //
        // "Batalha", e nao "professor": desde que materias.json passou a trazer
        // o professor, a INF 330 TEM professor (o Salles) e mesmo assim nao tem
        // luta - sao duas coisas diferentes.
        frase = "Batalha ainda nao pronta";
        cor = Color::LightBlue;

    } else {
        const float nota = mGame->GetMelhorNota(mSelectedSubject);

        if (nota <= 0.0f) {
            frase = "Ainda nao jogou";
        } else {
            // Duas casas pelo mesmo motivo da barra de nota da batalha: o teto
            // por dano e 99,99, e com uma casa ele viraria "100.00" na tela.
            std::stringstream ss;
            ss << "Maior nota: " << std::fixed << std::setprecision(2) << nota;
            frase = ss.str();
        }

        // O DOURADO ENTRA AQUI TAMBEM, e nao so na batalha: esta e a tela onde
        // um aluno compara a propria nota com a dos outros, entao e onde a nota
        // cheia mais precisa se distinguir do 99,99 de quem levou dano.
        if (Nota::ECheia(nota))                 cor = Color::Gold;
        else if (nota >= Nota::kNotaAprovacao)  cor = Color::LightGreen;
        else if (nota > 0.0f)                   cor = Color::LightPink;
        else                                    cor = Color::White;
    }

    EscreverNoCartao(mEstadoAtor, frase, cor);
}
