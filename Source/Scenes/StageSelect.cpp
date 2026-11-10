//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "TextoDeCena.h"
#include "../Entrada/Painel.h"
#include "FundoDeCena.h"

#include "../Aluno/Matricula.h"
#include "../Aluno/ProvaDaSemanaArquivo.h"
#include "../Motor/Relogio.h"
#include "../Curso/MateriasArquivo.h"
#include "../Regras/Nota.h"
#include "../Entrada/Navegacao.h"
#include <sstream>
#include <iomanip>
#include "../Motor/Math.h"
#include "../Actors/Buttons/StageSelectButton.h"
#include "../Motor/Font.h"
#include "../Components/DrawComponents/DrawTextComponent.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"

#include "../Motor/CaminhosArquivo.h"
#include "../Motor/Fontes.h"

StageSelect::StageSelect(Game *game) : Scene(game, SceneType::StageSelect)
                                       , mStageSelectFont(std::make_unique<Font>())
                                       , mInputTimer(INPUT_DELAY)
{

    mStageSelectFont->Load(Caminhos::Asset(Fontes::kJogo));
    mButtonObservers.reserve(RESERVA_DE_BOTOES);

}

void StageSelect::Load() {

    CriarFundoDoCorredor(*this);
    CreateStageButtons();
    CriarLigacoes();          // depois dos botoes: as linhas saem das posicoes deles
    CreateStaticUI();
    CriarIdentificacaoNaTela();

}

void StageSelect::CriarIdentificacaoNaTela() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    // Quem esta jogando, no canto de cima.
    const size_t quantos = mGame->QuantosJogadores();

    for (size_t quem = 0; quem < quantos; ++quem) {

        auto aluno = std::make_unique<Actor>(this);
        aluno->SetPosition(Vector2(230.f, 52.f + static_cast<float>(quem) * kEntreOsAlunos));

        const std::string quemE = quantos > 1 ? ("J" + std::to_string(quem + 1) + "  ") : "";
        const std::string rotulo = mGame->ProgressoEGravado(quem)
            ? (quemE + "Matricula: " + Matricula::ParaExibir(mGame->MatriculaDe(quem)))
            : (quemE + "Visitante - nao salva");
        // Ajustado ao texto, como o titulo e a maior nota.
        const auto rotuloDc = aluno->AddComponent<DrawTextComponent>(
            rotulo, mStageSelectFont.get(), 400, 44, 30, 255);
        rotuloDc->SetLarguraDeQuebra(400);
        rotuloDc->SetAjustarAoTexto(true);

        // O observador guarda a primeira linha, que e a unica que alguem reescreve.
        if (quem == 0) mAlunoAtor = aluno.get();
        AddActor(std::move(aluno));
    }

    auto trocar = std::make_unique<Actor>(this);
    trocar->SetPosition(Vector2(largura / 2.0f, static_cast<float>(mGame->GetWindowHeight()) - 40.f));
    // O rodape sai dos botoes, e nao de texto solto: ver Painel.h.
    const auto rodape = trocar->AddComponent<DrawTextComponent>(
        Painel::Rodape({{Painel::Jogador::Um, Painel::Botao::Um,   "jogar"},
                        {Painel::Jogador::Um, Painel::Botao::Dois, "dificuldade"},
                        {Painel::Jogador::Um, Painel::Botao::Tres, "boletim"}},
                       mGame->JeitoDoRodape()),
        mStageSelectFont.get(), 1100, 34, 22, 255);
    rodape->SetLarguraDeQuebra(1100);
    rodape->SetAjustarAoTexto(true);
    mTrocarAtor = trocar.get();
    AddActor(std::move(trocar));
}

void StageSelect::Linha(const float x, const float y,
                        const float largura, const float altura) {

    // Uma caixa preenchida sem moldura.
    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(x, y));
    ator->AddComponent<DrawCaixaComponent>(
        static_cast<int>(largura), static_cast<int>(altura),
        SDL_Color{0, 0, 0, 0}, kCorDaLigacao, 0, kOrdemDasLigacoes);
    AddActor(std::move(ator));
}

void StageSelect::CriarLigacoes() {

    // O curso como mapa, e nao como losangos soltos.
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
    // 1.
    constexpr float buttonWidth = 128.0f;

    // Obtendo dimensões da tela uma única vez para clareza.
    const auto screenW = static_cast<float>(mGame->GetWindowWidth());
    const auto screenH = static_cast<float>(mGame->GetWindowHeight());

    // Margens (1/12 da tela)
    const float marginX = screenW / 12.0f;

    // 2.
    const float startX = marginX + buttonWidth / 2.0f;
    const float endX = screenW - (marginX + buttonWidth / 2.0f);

    // 3.
    const float startY = screenH * kTopoDaGrade;
    const float endY = screenH * kFundoDaGrade;

    // A grade vem de materias.json, e nao de listas escritas aqui.
    const Materias::Lista& materias = Materias::Carregadas();
    const int quantasColunas = materias.QuantasColunas();

    // O passo horizontal supoe intervalos entre colunas, e nao colunas.
    const float stepX = (quantasColunas > 1)
                            ? (endX - startX) / static_cast<float>(quantasColunas - 1)
                            : 0.0f;

    // 4.
    size_t maxRows = 0;
    for (int c = 0; c < quantasColunas; ++c) {
        maxRows = std::max(maxRows, materias.DaColuna(c).size());
    }

    // Evita divisão por zero se as listas estiverem vazias ou tiverem apenas 1 item.
    float stepY = 0.0f;
    if (maxRows > 1) {
        stepY = (endY - startY) / static_cast<float>(maxRows - 1);
    }

    // Criação dos botões.

    mGrade.assign(static_cast<size_t>(quantasColunas), {});

    for (int coluna = 0; coluna < quantasColunas; ++coluna) {

        const std::vector<int> daColuna = materias.DaColuna(coluna);
        if (daColuna.empty()) continue;

        const float xPos = startX + stepX * static_cast<float>(coluna);

        for (size_t i = 0; i < daColuna.size(); ++i) {

            // Coluna de uma materia so fica centrada na vertical.
            const float yPos = (daColuna.size() == 1)
                                   ? (startY + endY) / 2.0f
                                   : startY + stepY * static_cast<float>(i);

            const Materias::Materia* m = materias.Por(daColuna[i]);
            if (m == nullptr) continue;

            mGrade[static_cast<size_t>(coluna)].push_back(mButtonObservers.size());

            CreateButton(m->nome, (daColuna[i]),
                         Vector2(xPos, yPos));
        }
    }

    // 5.
    if (!mButtonObservers.empty()) {
        mSelectedIndex = 0;
        for (size_t i = 0; i < mButtonObservers.size(); ++i) {
            if (mButtonObservers[i] && mButtonObservers[i]->GetSubject() == mGame->MateriaEmFoco()) {
                mSelectedIndex = i;
                break;
            }
        }
        // Validação de segurança: sempre cheque ponteiros brutos em vetores.
        if (mButtonObservers[mSelectedIndex]) {
            mButtonObservers[mSelectedIndex]->ChangeSelected();
            mSelectedSubject = mButtonObservers[mSelectedIndex]->GetSubject();
            mGame->FocarMateria(mSelectedSubject);
        }
    }
}

void StageSelect::CreateButton(const std::string& text, int subject, const Vector2& position) {

    bool unlocked = mGame->IsStageUnlocked(subject);
    auto button = std::make_unique<StageSelectButton>(this, text, subject, Caminhos::Asset(Fontes::kJogo), !unlocked);
    button->SetPosition(position);
    button->SetAprovado(mGame->Aprovado(subject));

    if (!unlocked) {
        // O nome da materia fechada, embaixo do losango.
        auto rotulo = std::make_unique<Actor>(this);
        rotulo->SetPosition(position + Vector2(0.0f, kNomeAbaixoDoCadeado));
        const auto dc = rotulo->AddComponent<DrawTextComponent>(
            text, mStageSelectFont.get(), 150, 26, 18, 255);
        dc->SetLarguraDeQuebra(150);
        dc->SetAjustarAoTexto(true);

        // Quem acende o rotulo ao ganhar o foco e o proprio botao, que e quem sabe da selecao.
        button->SetRotuloFechado(rotulo.get());
        AddActor(std::move(rotulo));
    }

    mButtonObservers.push_back(button.get());
    this->AddActor(std::move(button));
}

Actor* StageSelect::TextoDoCartao(const float y, const int tamanho, const Vector3& cor) {
    const int largura = static_cast<int>(kCartaoLargura) - 40;
    auto* dc = CriarTexto(*this, mStageSelectFont.get(), " ", Vector2(kCartaoX, y), tamanho, largura, tamanho + 8);
    dc->SetColor(cor);
    return dc->GetOwner();
}

void StageSelect::CreateStaticUI() {

    const auto w = static_cast<float>(mGame->GetWindowWidth());

    // O titulo no meio, e o avanco no curso na direita.
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

    // E as mencoes do curso, embaixo: passar abre a materia seguinte, e elas sao o que sobra em cada uma.
    const Mencoes::Total mencoes = Mencoes::DoCurso(mGame->Sessoes().front().progresso, materias);
    auto* mencoesDc = CriarTexto(*this, mStageSelectFont.get(),
                                 "Men\xc3\xa7\xc3\xb5" "es: " + std::to_string(mencoes.ganhas) + " de " +
                                     std::to_string(mencoes.possiveis),
                                 Vector2(w - 190.f, 86.f), 20, 320, 28);
    mencoesDc->SetColor(Color::LightYellow);

    // O cartao da materia em foco, no canto que a grade deixa vazio.
    auto moldura = std::make_unique<Actor>(this);
    moldura->SetPosition(Vector2(kCartaoX, kCartaoY));
    moldura->AddComponent<DrawCaixaComponent>(
        static_cast<int>(kCartaoLargura), static_cast<int>(kCartaoAltura),
        kCorDaMolduraDoCartao, kCorDoFundoDoCartao, 2, kOrdemDoCartao);
    AddActor(std::move(moldura));

    mCodigoAtor    = TextoDoCartao(kCartaoY - 74.f, 30, Color::White);
    mNomeAtor      = TextoDoCartao(kCartaoY - 34.f, 22, Color::LightYellow);
    mProfessorAtor = TextoDoCartao(kCartaoY + 0.f, 20, Color::LightBlue);
    mEstadoAtor    = TextoDoCartao(kCartaoY + 36.f, 22, Color::White);
    mMencoes = std::make_unique<SelosDeMencao>(
        *this, mStageSelectFont.get(), Vector2(kCartaoX - SelosDeMencao::kLarguraDaFileira / 2.0f, kCartaoY + 78.f));

    UpdateStageInfo();
    CriarProvaDaSemana();
}

void StageSelect::CriarProvaDaSemana() {

    const Materias::Lista& materias = Materias::Carregadas();
    const ProvaDaSemana::Semana semana = ProvaDaSemana::Da(Relogio::Agora());
    const int materia = ProvaDaSemana::MateriaDa(semana, materias);
    const Materias::Materia* m = materias.Por(materia);
    if (!m) return;

    auto moldura = std::make_unique<Actor>(this);
    moldura->SetPosition(Vector2(kProvaX, kCartaoY));
    moldura->AddComponent<DrawCaixaComponent>(static_cast<int>(kProvaLargura), static_cast<int>(kCartaoAltura),
                                              SDL_Color{255, 214, 72, 200}, kCorDoFundoDoCartao, 2, kOrdemDoCartao);
    AddActor(std::move(moldura));

    const int largura = static_cast<int>(kProvaLargura) - 24;
    const auto linha = [&](const std::string& texto, const float dy, const int tamanho, const Vector3& cor) {
        CriarTexto(*this, mStageSelectFont.get(), texto.empty() ? " " : texto, Vector2(kProvaX, kCartaoY + dy), tamanho,
                   largura, tamanho + 8)->SetColor(cor);
    };

    // \xc3\xba e o u com acento, \xc2\xb7 o ponto do meio, \xc2\xba o "o" de ordinal, \xc3\xad o i com acento.
    linha("PROVA DA SEMANA", -86.0f, 22, Color::Gold);
    const std::string prazo = semana.faltam <= 1 ? std::string("\xc3\xbaltimo dia")
                                                 : "faltam " + std::to_string(semana.faltam) + " dias";
    linha(m->nome + "  \xc2\xb7  " + prazo, -58.0f, 18, Color::LightYellow);

    // O ranking do modo e da dificuldade desta partida.
    const bool dupla = mGame->QuantosJogadores() > 1;
    const bool dificil = mGame->NivelDeDificuldade() == Dificuldade::Nivel::Dificil;
    const auto ranking = ProvaDaSemana::Ranking(ProvaDaSemana::Carregar(), semana, dupla, dificil);
    for (int i = 0; i < ProvaDaSemana::kPodio; ++i) {
        std::string texto = std::to_string(i + 1) + "\xc2\xba   -";
        if (static_cast<size_t>(i) < ranking.size()) {
            const ProvaDaSemana::Colocado& c = ranking[static_cast<size_t>(i)];
            std::stringstream ss;
            ss << c.posicao << "\xc2\xba   " << c.quem << "   " << std::fixed << std::setprecision(2) << c.nota;
            texto = ss.str();
        }
        linha(texto, -26.0f + 24.0f * static_cast<float>(i), 18, i == 0 ? Color::White : Color::LightBlue);
    }

    // O lugar de quem esta jogando.
    ProvaDaSemana::Entrada eu;
    eu.dupla = dupla;
    eu.matricula = mGame->MatriculaDe(0);
    if (dupla) eu.parceiro = Sessao::ParceiroDe(mGame->Sessoes(), 0);
    const bool visitante = !mGame->ProgressoEGravado(0);
    const int lugar = visitante ? 0 : ProvaDaSemana::PosicaoDe(ranking, ProvaDaSemana::Quem(eu));
    std::string meu;
    if (visitante) meu = "visitante n\xc3\xa3o entra no ranking";
    else if (lugar > 0) meu = (dupla ? "a dupla: " : "voc\xc3\xaa: ") + std::to_string(lugar) + "\xc2\xba de " + std::to_string(ranking.size());
    else meu = dupla ? "a dupla ainda n\xc3\xa3o fez" : "voc\xc3\xaa ainda n\xc3\xa3o fez";
    linha(meu, 58.0f, 18, lugar > 0 ? Color::LightGreen : Color::LightYellow);
    linha(std::string(dificil ? "Dif\xc3\xad" "cil" : "Normal") + (dupla ? ", em dupla" : ", sozinho") +
              "  \xc2\xb7  a mesma prova para todos",
          86.0f, 14, Color::LightBlue);

    // A marca em cima do botao da materia da semana.
    for (const StageSelectButton* botao : mButtonObservers) {
        if (!botao || botao->GetSubject() != materia) continue;
        CriarTexto(*this, mStageSelectFont.get(), "PROVA DA SEMANA", botao->GetPosition() + Vector2(0.0f, -44.0f), 13,
                   160, 20)->SetColor(Color::Gold);
    }
}

void StageSelect::OnProcessInput(const Uint8 *keyState) {

    // Voltar e voltar um passo: a escolha da dificuldade, e de la a troca de aluno (a identificacao).
    const bool trocar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Dois)
                     || keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaTrocar.Bateu(trocar)) {
        mGame->RequestSceneChange(SceneType::EscolhaDaDificuldade);
        return;
    }

    HandleSelectionInput(keyState);
}
void StageSelect::HandleSelectionInput(const Uint8 *keyState) {

    size_t currSelected = mSelectedIndex;
    if (mInputTimer >= INPUT_DELAY) {
        currSelected = HandleSelectedChange(keyState, currSelected);
    }

    if (currSelected != mSelectedIndex) {
        // Se houve mudança, atualiza a UI e o estado.
        mButtonObservers[mSelectedIndex]->ChangeSelected();
        mButtonObservers[currSelected]->ChangeSelected(); // Seleciona o novo

        mSelectedIndex = currSelected; // Atualiza o estado real.
        mSelectedSubject = mButtonObservers[mSelectedIndex]->GetSubject();
        mGame->FocarMateria(mSelectedSubject);
        mInputTimer = 0.0f;
        UpdateStageInfo();
    }

    // O boletim; dele, o Botao 3 de novo leva ao ranking da materia em foco.
    const bool verBoletim = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Tres);
    if (mBordaRanking.Bateu(verBoletim)) {
        mGame->RequestSceneChange(SceneType::Boletim);
        return;
    }

    // Precisa ser uma batida nova, e nao o estado.
    const bool entrar = Painel::Apertado(keyState, Painel::Jogador::Um, Painel::Botao::Um)
                     || keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (mBordaEntrar.Bateu(entrar) && mGame->IsStageUnlocked(mSelectedSubject)) {
        mGame->SetSelectedStage(mSelectedSubject);

        // A materia que pede orientacao passa pela escolha antes da batalha.
        mGame->DefinirOrientador(-1);
        const Materias::Materia* m = Materias::Carregadas().Por(mSelectedSubject);
        mGame->RequestSceneChange((m && m->orientacao) ? SceneType::EscolhaDoOrientador : SceneType::Battle);
        return;
    }
}

// Navegacao. A regra mora em Navegacao, E a forma da grade vem de quem desenhou os botoes.

size_t StageSelect::HandleSelectedChange(const Uint8 *keyState, const size_t currSelected) const {

    // A selecao e sempre do jogador 1, tambem no co-op: a materia que ele abriu e a que a dupla joga.
    constexpr Painel::Jogador kQuem = Painel::Jogador::Um;

    if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Cima))     return Navegacao::Cima(mGrade, currSelected);
    if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Baixo))    return Navegacao::Baixo(mGrade, currSelected);
    if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Esquerda)) return Navegacao::Esquerda(mGrade, currSelected);
    if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Direita))  return Navegacao::Direita(mGrade, currSelected);

    return currSelected;
}

void StageSelect::OnUpdate(float deltaTime) {

    if(mInputTimer < INPUT_DELAY) mInputTimer += deltaTime;

}

void StageSelect::EscreverNoCartao(Actor* ator, const std::string& texto, const Vector3& cor) {

    if (!ator) return;
    if (const auto dc = ator->GetComponent<DrawTextComponent>()) {
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
        frase = materias.ExigenciaDe(mSelectedSubject);
        if (frase.empty()) frase = "Ainda fechada";
        cor = Color::LightPink;

    } else if (m && m->chefe.empty()) {
        // Seis das onze materias ainda nao tem batalha.
        frase = "Batalha ainda nao pronta";
        cor = Color::LightBlue;

    } else {
        const float nota = mGame->GetMelhorNota(mSelectedSubject);

        if (nota <= 0.0f) {
            frase = "Ainda nao jogou";
        } else {
            // Duas casas pelo mesmo motivo da barra de nota da batalha.
            std::stringstream ss;
            ss << "Maior nota: " << std::fixed << std::setprecision(2) << nota;
            frase = ss.str();
        }

        // O dourado entra aqui tambem, e nao so na batalha.
        if (Nota::ECheia(nota))                 cor = Color::Gold;
        else if (nota >= Nota::kNotaAprovacao)  cor = Color::LightGreen;
        else if (nota > 0.0f)                   cor = Color::LightPink;
        else                                    cor = Color::White;
    }

    EscreverNoCartao(mEstadoAtor, frase, cor);

    // A fileira so aparece na materia que tem batalha; centrada, com as mencoes que ela tem.
    if (mMencoes) mMencoes->Mostrar(Mencoes::Da(mGame->Sessoes().front().progresso, materias, mSelectedSubject));
}
