//
// Created by nslop on 21/08/2024.
//

#include "StageSelect.h"
#include "TextoDeCena.h"
#include "../Entrada/Painel.h"
#include "FundoDeCena.h"

#include "../Aluno/Matricula.h"
#include "../Aluno/FichaArquivo.h"
#include "../Aluno/Rival.h"
#include "../Curso/MapaDaGrade.h"
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
    mFichas = FichaArquivo::TodasAsFichas(Materias::Carregadas());
    CreateStageButtons();
    CriarLigacoes();          // depois dos botoes: as linhas saem das posicoes deles
    CreateStaticUI();
    CriarIdentificacaoNaTela();
    SeguirOFoco(true);

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
    NoMapa(AddActor(std::move(ator)));
}

void StageSelect::CriarLigacoes() {

    // O curso como mapa, e nao como losangos soltos.
    const Materias::Lista& materias = Materias::Carregadas();

    for (size_t c = 0; c + 1 < mGrade.size(); ++c) {

        if (mGrade[c].empty() || mGrade[c + 1].empty()) continue;
        if (!materias.ColunaDependeDaAnterior(static_cast<int>(c) + 1)) continue;
        // De uma pagina para a outra nao ha linha: o corredor muda de ala.
        if (mColunas[c].pagina != mColunas[c + 1].pagina) continue;

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
    const auto screenW = static_cast<float>(mGame->GetWindowWidth());
    const auto screenH = static_cast<float>(mGame->GetWindowHeight());
    const float startY = screenH * kTopoDaGrade;
    const float endY = screenH * kFundoDaGrade;

    // A grade vem de materias.json, e nao de listas escritas aqui; o x de cada coluna, e a pagina dela,
    // do mapa: as obrigatorias na primeira pagina, as optativas nas seguintes.
    const Materias::Lista& materias = Materias::Carregadas();
    const int quantasColunas = materias.QuantasColunas();
    mColunas = MapaDaGrade::Colunas(materias.QuantasColunasObrigatorias(), quantasColunas, screenW);
    mPaginas = MapaDaGrade::Paginas(mColunas);

    // O passo vertical de cada pagina: a coluna mais cheia dela divide a altura.
    std::vector<size_t> maxRows(static_cast<size_t>(mPaginas), 0);
    for (int c = 0; c < quantasColunas; ++c) {
        size_t& m = maxRows[static_cast<size_t>(mColunas[static_cast<size_t>(c)].pagina)];
        m = std::max(m, materias.DaColuna(c).size());
    }

    // Criação dos botões.

    mGrade.assign(static_cast<size_t>(quantasColunas), {});

    for (int coluna = 0; coluna < quantasColunas; ++coluna) {

        const std::vector<int> daColuna = materias.DaColuna(coluna);
        if (daColuna.empty()) continue;

        const MapaDaGrade::Coluna& lugar = mColunas[static_cast<size_t>(coluna)];
        const float xPos = lugar.x;
        const size_t linhasDaPagina = maxRows[static_cast<size_t>(lugar.pagina)];
        // Nas paginas das optativas a grade fica acima dos cartoes, que ocupam os dois cantos de baixo.
        const float fundo = lugar.pagina == 0 ? endY : screenH * kFundoDasOptativas;
        const float stepY = linhasDaPagina > 1 ? (fundo - startY) / static_cast<float>(linhasDaPagina - 1) : 0.0f;

        for (size_t i = 0; i < daColuna.size(); ++i) {

            // Coluna de uma materia so fica centrada na vertical.
            const float yPos = (daColuna.size() == 1)
                                   ? (startY + fundo) / 2.0f
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
        NoMapa(AddActor(std::move(rotulo)));
    }

    mButtonObservers.push_back(button.get());
    NoMapa(this->AddActor(std::move(button)));
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
    mTituloAtor = AddActor(std::move(tituloAtor));

    const Materias::Lista& materias = Materias::Carregadas();
    int aprovadas = 0;
    for (int i = 0; i < materias.Quantas(); ++i) {
        const Materias::Materia* m = materias.Por(i);
        if (m && !m->optativa && mGame->Aprovado(i)) ++aprovadas;
    }

    auto avanco = std::make_unique<Actor>(this);
    avanco->SetPosition(Vector2(w - 190.f, 52.f));
    const auto avancoDc = avanco->AddComponent<DrawTextComponent>(
        "Aprovadas: " + std::to_string(aprovadas) + " de " + std::to_string(materias.QuantasObrigatorias()),
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

    mCodigoAtor    = TextoDoCartao(kCartaoY - 86.f, 30, Color::White);
    mNomeAtor      = TextoDoCartao(kCartaoY - 46.f, 22, Color::LightYellow);
    mProfessorAtor = TextoDoCartao(kCartaoY - 12.f, 20, Color::LightBlue);
    mEstadoAtor    = TextoDoCartao(kCartaoY + 24.f, 22, Color::White);
    mRivalAtor     = TextoDoCartao(kCartaoY + 56.f, 17, Color::LightBlue);
    mMencoes = std::make_unique<SelosDeMencao>(
        *this, mStageSelectFont.get(), Vector2(kCartaoX - SelosDeMencao::kLarguraDaFileira / 2.0f, kCartaoY + 88.f));

    UpdateStageInfo();
    CriarProvaDaSemana();
}

void StageSelect::CriarProvaDaSemana() {

    const Materias::Lista& materias = Materias::Carregadas();
    const ProvaDaSemana::Semana semana = ProvaDaSemana::Da(Relogio::Agora());
    const int materia = ProvaDaSemana::MateriaDa(semana, materias);
    const Materias::Materia* m = materias.Por(materia);
    if (!m) return;
    mMateriaDaSemana = materia;

    // Um cartao pequeno, no canto: uma aba dourada com o titulo, a materia e o prazo, o podio com as
    // medalhas e o lugar de quem joga. So aparece com o cursor na materia da semana.
    const float topo = kProvaY - kProvaAltura / 2.0f;
    const auto caixa = [&](const Vector2& onde, const float w, const float h, const SDL_Color& moldura,
                           const SDL_Color& fundo, const int espessura, const int ordem) {
        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(onde);
        ator->AddComponent<DrawCaixaComponent>(static_cast<int>(w), static_cast<int>(h), moldura, fundo, espessura, ordem);
        mCartaoDaProva.push_back(AddActor(std::move(ator)));
    };
    const auto texto = [&](const std::string& t, const Vector2& onde, const int tamanho, const int largura,
                           const Vector3& cor, const bool aEsquerda = false) {
        auto* dc = CriarTexto(*this, mStageSelectFont.get(), t.empty() ? " " : t, onde, tamanho, largura, tamanho + 8,
                              kOrdemDoCartao + 2);
        dc->SetColor(cor);
        dc->SetAEsquerda(aEsquerda);
        mCartaoDaProva.push_back(dc->GetOwner());
    };

    const SDL_Color ouro{255, 214, 72, 235};
    caixa(Vector2(kProvaX, kProvaY), kProvaLargura, kProvaAltura, ouro, SDL_Color{24, 20, 10, 235}, 2, kOrdemDoCartao);
    caixa(Vector2(kProvaX, topo + 13.0f), kProvaLargura, 26.0f, ouro, ouro, 0, kOrdemDoCartao + 1);
    // \xc3\xba e o u com acento, \xc2\xb7 o ponto do meio, \xc2\xba o "o" de ordinal, \xc3\xad o i com acento.
    texto("PROVA DA SEMANA", Vector2(kProvaX, topo + 13.0f), 15, static_cast<int>(kProvaLargura) - 20,
          Vector3(0.12f, 0.09f, 0.02f));
    const std::string prazo = semana.faltam <= 1 ? std::string("\xc3\xbaltimo dia")
                                                 : "faltam " + std::to_string(semana.faltam) + " dias";
    texto(m->nome + "  \xc2\xb7  " + prazo, Vector2(kProvaX, topo + 42.0f), 15, static_cast<int>(kProvaLargura) - 20,
          Color::LightYellow);

    // O podio do modo e da dificuldade desta partida: medalha, quem e a nota.
    const bool dupla = mGame->QuantosJogadores() > 1;
    const bool dificil = mGame->NivelDeDificuldade() == Dificuldade::Nivel::Dificil;
    const auto ranking = ProvaDaSemana::Ranking(ProvaDaSemana::Carregar(), semana, dupla, dificil);
    const SDL_Color medalhas[] = {{255, 206, 60, 255}, {200, 208, 222, 255}, {205, 127, 70, 255}};
    for (int i = 0; i < ProvaDaSemana::kPodio; ++i) {
        const float y = topo + 68.0f + 22.0f * static_cast<float>(i);
        caixa(Vector2(kProvaX - 112.0f, y), 12.0f, 12.0f, medalhas[i], medalhas[i], 0, kOrdemDoCartao + 1);
        if (static_cast<size_t>(i) < ranking.size()) {
            const ProvaDaSemana::Colocado& c = ranking[static_cast<size_t>(i)];
            std::stringstream nota;
            nota << std::fixed << std::setprecision(2) << c.nota;
            texto(c.quem, Vector2(kProvaX - 6.0f, y), 15, 172, i == 0 ? Color::White : Color::LightBlue, true);
            texto(nota.str(), Vector2(kProvaX + 100.0f, y), 15, 60, i == 0 ? Color::White : Color::LightBlue);
        } else {
            texto("ningu\xc3\xa9m ainda", Vector2(kProvaX - 6.0f, y), 14, 172, Vector3(0.5f, 0.52f, 0.62f), true);
        }
    }

    // O lugar de quem esta jogando, e as regras numa linha.
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
    texto(meu, Vector2(kProvaX, topo + 138.0f), 14, static_cast<int>(kProvaLargura) - 20,
          lugar > 0 ? Color::LightGreen : Color::LightYellow);
    texto(std::string(dificil ? "Dif\xc3\xad" "cil" : "Normal") + (dupla ? " \xc2\xb7 dupla" : " \xc2\xb7 sozinho") +
              " \xc2\xb7 a mesma prova para todos",
          Vector2(kProvaX, topo + 159.0f), 12, static_cast<int>(kProvaLargura) - 16, Color::LightBlue);

    // A marca em cima do botao da materia da semana: e ela que chama o cursor.
    for (const StageSelectButton* botao : mButtonObservers) {
        if (!botao || botao->GetSubject() != materia) continue;
        NoMapa(CriarTexto(*this, mStageSelectFont.get(), "PROVA DA SEMANA", botao->GetPosition() + Vector2(0.0f, -44.0f),
                          13, 160, 20)->GetOwner());
        mNosDoMapa.back().ator->GetComponent<DrawTextComponent>()->SetColor(Color::Gold);
    }

    MostrarCartaoDaProva(mSelectedSubject == mMateriaDaSemana);
}

void StageSelect::MostrarCartaoDaProva(const bool mostrar) {
    for (Actor* a : mCartaoDaProva) {
        if (auto* dc = a ? a->GetComponent<DrawComponent>() : nullptr) dc->SetIsVisible(mostrar);
    }
}

void StageSelect::NoMapa(Actor* ator) {
    if (ator) mNosDoMapa.push_back({ator, ator->GetPosition()});
}

void StageSelect::SeguirOFoco(const bool deUmaVez) {
    if (mButtonObservers.empty()) return;
    size_t coluna = 0, linha = 0;
    if (Navegacao::Onde(mGrade, mSelectedIndex, coluna, linha) && coluna < mColunas.size()) {
        mPagina = mColunas[coluna].pagina;
    }
    if (deUmaVez) {
        mCamera = MapaDaGrade::DeslocamentoDa(mPagina, static_cast<float>(mGame->GetWindowWidth()));
        for (const NoDoMapa& n : mNosDoMapa) n.ator->SetPosition(n.base + Vector2(mCamera, 0.0f));
    }
    AtualizarDicas();
}

void StageSelect::AtualizarDicas() {
    const auto w = static_cast<float>(mGame->GetWindowWidth());
    const auto h = static_cast<float>(mGame->GetWindowHeight());
    if (mPaginas > 1 && !mDicaDireita) {
        // \xc2\xbb e \xc2\xab sao as aspas angulares, as setas que a fonte tem; \xc3\x93 e o O agudo.
        auto* d = CriarTexto(*this, mStageSelectFont.get(), " ", Vector2(w - 70.0f, h * 0.6f), 16, 140, 26);
        d->SetColor(Color::Gold);
        mDicaDireita = d->GetOwner();
        auto* e = CriarTexto(*this, mStageSelectFont.get(), "\xc2\xab OBRIGAT\xc3\x93RIAS", Vector2(96.0f, h * 0.48f), 16,
                             170, 26);
        e->SetColor(Color::Gold);
        mDicaEsquerda = e->GetOwner();
    }
    if (mDicaDireita) {
        auto* d = mDicaDireita->GetComponent<DrawTextComponent>();
        d->SetText(mPagina == 0 ? "OPTATIVAS \xc2\xbb" : "MAIS \xc2\xbb");
        d->SetIsVisible(mPagina + 1 < mPaginas);
    }
    if (mDicaEsquerda) mDicaEsquerda->GetComponent<DrawTextComponent>()->SetIsVisible(mPagina > 0);
    if (mTituloAtor) {
        if (auto* t = mTituloAtor->GetComponent<DrawTextComponent>()) t->SetText(mPagina == 0 ? "ESCOLHA A MATERIA" : "OPTATIVAS");
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
        SeguirOFoco(false);
        MostrarCartaoDaProva(mSelectedSubject == mMateriaDaSemana);
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

    // A camera desliza ate a pagina do foco: as obrigatorias, ou a ala das optativas.
    const float alvo = MapaDaGrade::DeslocamentoDa(mPagina, static_cast<float>(mGame->GetWindowWidth()));
    if (mCamera != alvo) {
        mCamera = MapaDaGrade::Aproximar(mCamera, alvo, deltaTime);
        for (const NoDoMapa& n : mNosDoMapa) n.ator->SetPosition(n.base + Vector2(mCamera, 0.0f));
    }

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

    // O rival da turma: so na materia aberta e com batalha, na lista do modo e do nivel da vez.
    std::string rival;
    Vector3 corDoRival = Color::LightBlue;
    if (m && !m->chefe.empty() && mGame->IsStageUnlocked(mSelectedSubject)) {
        std::vector<std::string> alunos;
        for (size_t i = 0; i < mGame->QuantosJogadores(); ++i) alunos.push_back(mGame->MatriculaDe(i));
        const Rival::Turma turma = Rival::DaTurma(mFichas, mSelectedSubject, mGame->NivelDeDificuldade(), alunos,
                                                  mGame->Sessoes().front().progresso);
        const Rival::Situacao s = Rival::Da(turma.colegas, turma.nota);
        rival = Rival::Frase(s);
        if (s.tipo == Rival::Tipo::Atras) corDoRival = Color::LightYellow;
        else if (s.tipo == Rival::Tipo::Lider) corDoRival = Color::LightGreen;
    }
    EscreverNoCartao(mRivalAtor, rival, corDoRival);

    // A fileira so aparece na materia que tem batalha; centrada, com as mencoes que ela tem.
    if (mMencoes) mMencoes->Mostrar(Mencoes::Da(mGame->Sessoes().front().progresso, materias, mSelectedSubject));
}
