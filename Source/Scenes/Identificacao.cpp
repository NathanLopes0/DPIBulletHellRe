//
// A tela em que o aluno digita a matricula.
//

#include "Identificacao.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../Matricula.h"
#include "../FichaArquivo.h"
#include "../CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    /// Os scancodes dos digitos da fileira de cima, na ordem 0..9.
    ///
    /// TABELA, e nao aritmetica: no SDL o SDL_SCANCODE_0 vem DEPOIS do 9, entao
    /// SDL_SCANCODE_0 + 3 cai em BACKSPACE e + 4 em TAB. Ja escrevi a conta errada
    /// uma vez e o campo se apagava sozinho.
    const SDL_Scancode kDigitos[10] = {
        SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
        SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9
    };

    /// E os do teclado numerico, para quem digitar por ali.
    const SDL_Scancode kDigitosNumerico[10] = {
        SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3,
        SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7,
        SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9
    };

    // A caixa do campo. Larga o bastante para seis digitos com folga.
    constexpr int kCampoLargura = 420;
    constexpr int kCampoAltura  = 110;

    constexpr SDL_Color kBorda        = {235, 235, 235, 255};
    constexpr SDL_Color kBordaErro    = {235, 110,  90, 255};
    constexpr SDL_Color kFundoDoCampo = { 24,  26,  32, 220};

    /// Quanto tempo o cursor fica aceso e apagado.
    // --- O teclado digital ---
    constexpr int kTeclaLargura  = 90;    // as teclas de digito
    constexpr int kAcaoLargura   = 240;   // a coluna de acoes, que tem rotulo
    constexpr int kTeclaAltura   = 70;
    constexpr int kVao           = 14;    // entre teclas
    constexpr int kVaoDaColuna   = 40;    // entre os digitos e as acoes
    // O teclado comeca ABAIXO da caixa da matricula, que vai ate 319 (centro em
    // 0,33 da altura, 110 de alto). A primeira versao disto comecava em 400 e a
    // caixa cobria a fila do 1-2-3: a moldura sumia atras das teclas e o campo
    // parecia cortado.
    constexpr int kTopoDoTeclado = 380;

    constexpr SDL_Color kTeclaBorda   = { 90,  96, 110, 255};
    constexpr SDL_Color kTeclaAcesa   = {255, 214,   0, 255};   // Color::Gold, em SDL_Color
    constexpr SDL_Color kTeclaFundo   = { 24,  26,  32, 220};

    constexpr float kPiscada = 0.5f;

    // AS DUAS RECUSAS DE FLUXO ficam aqui, e nao em Matricula::MensagemDeErro,
    // porque nao falam da matricula: "89384" e igualmente valida nos dois casos.
    // Elas falam do DISCO, que a camada pura nao conhece nem deve conhecer.
    const char* kJaExiste = "Ja existe um perfil com essa matricula. Use Carregar Perfil.";
    const char* kNaoExiste = "Nao ha perfil com essa matricula. Confira, ou use Novo Jogo.";
}

Identificacao::Identificacao(Game* game)
    : Scene(game, SceneType::Identificacao),
      mFonte(std::make_unique<Font>()),
      mModoNovo(game->ModoDeEntradaAtual() == Game::ModoDeEntrada::Novo)
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void Identificacao::Load() {
    CriarTextos();
    Redesenhar();
}

Actor* Identificacao::Texto(const std::string& inicial, const float y, const int tamanho,
                            const int larguraMaxima, const int alturaMaxima) {

    auto ator = std::make_unique<Actor>(this);
    ator->SetPosition(Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f, y));

    auto dc = ator->AddComponent<DrawTextComponent>(inicial, mFonte.get(),
                                                    larguraMaxima, alturaMaxima, tamanho, 255);

    // A quebra acompanha a largura pedida: com os 500 fixos de antes, a linha de
    // ajuda quebrava no meio e as duas metades se sobrepunham.
    dc->SetLarguraDeQuebra(static_cast<unsigned>(larguraMaxima));

    // SEM ESTICAR. Era o que deixava a matricula deformada: com um digito so, a
    // textura de um caractere era esticada para a caixa inteira e saia borrada,
    // "espichando" na horizontal ate a matricula ficar completa. Agora a caixa e
    // so um limite, e o texto e desenhado no tamanho dele.
    dc->SetAjustarAoTexto(true);

    Actor* bruto = ator.get();
    AddActor(std::move(ator));
    return bruto;
}

void Identificacao::CriarTextos() {

    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    // 52 e nao 54: Font::Load so pre-carrega alguns tamanhos, e pedir um que nao
    // existe desenhava NADA. O titulo tinha sumido por causa disso.
    //
    // O TITULO DIZ A QUE VEIO. Sem isso as duas telas sao identicas, e quem
    // errou de opcao no menu so descobre quando a matricula e recusada.
    mTituloAtor = Texto(mModoNovo ? "NOVO JOGO" : "CARREGAR PERFIL",
                        altura * 0.13f, 52, 760, 70);
    Texto(mModoNovo ? "Digite sua matricula para criar o perfil"
                    : "Digite sua matricula para continuar",
          altura * 0.21f, 28, 760, 42);

    // A moldura do campo: um ator proprio, com ordem de desenho menor que a do
    // texto para ficar atras dele.
    auto caixa = std::make_unique<Actor>(this);
    caixa->SetPosition(Vector2(largura / 2.0f, altura * 0.33f));
    mCaixaDesenho = caixa->AddComponent<DrawCaixaComponent>(kCampoLargura, kCampoAltura,
                                                            kBorda, kFundoDoCampo, 3, 110);
    AddActor(std::move(caixa));

    // Os digitos, dentro da caixa. O limite e um pouco menor que ela para o texto
    // nunca encostar na moldura.
    mCampoAtor = Texto(" ", altura * 0.33f, 72, kCampoLargura - 48, kCampoAltura - 28);

    // Largura folgada: as frases de recusa sao bem mais longas que as de erro de
    // formato, e uma delas quebrava linha com os 760 de antes.
    mErroAtor  = Texto(" ", altura * 0.425f, 26, 1040, 40);

    // Linhas proprias em vez de uma longa: assim cada uma fica centrada de
    // verdade, e nenhuma depende de caber numa largura de quebra.
    // A ajuda agora fala do GABINETE, e nao do teclado: o manche escolhe, um
    // botao digita e o outro apaga. As teclas do computador continuam valendo
    // para quem desenvolve, mas nao e para elas que esta tela e desenhada.
    Texto("MANCHE  escolher          BOTAO 1  digitar          BOTAO 2  apagar",
          altura * 0.955f, 22, 1100, 34);

    CriarTeclado();
}

void Identificacao::CriarTeclado() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    mGradeDoTeclado = Teclado::Grade();
    mTeclaSelecionada = Teclado::Inicial();

    const int blocoDeDigitos = 3 * kTeclaLargura + 2 * kVao;
    const int total = blocoDeDigitos + kVaoDaColuna + kAcaoLargura;
    const float x0 = (largura - static_cast<float>(total)) / 2.0f;

    const auto& teclas = Teclado::Teclas();
    mCaixasDasTeclas.assign(teclas.size(), nullptr);

    for (size_t i = 0; i < teclas.size(); ++i) {

        const Teclado::Tecla& tecla = teclas[i];
        const bool ehAcao = (tecla.acao != Teclado::Acao::Digito);

        const int larguraDaTecla = ehAcao ? kAcaoLargura : kTeclaLargura;
        const float cx = ehAcao
            ? x0 + static_cast<float>(blocoDeDigitos + kVaoDaColuna + kAcaoLargura / 2)
            : x0 + static_cast<float>(tecla.coluna * (kTeclaLargura + kVao) + kTeclaLargura / 2);
        const float cy = static_cast<float>(kTopoDoTeclado + tecla.linha * (kTeclaAltura + kVao)
                                            + kTeclaAltura / 2);

        auto caixa = std::make_unique<Actor>(this);
        caixa->SetPosition(Vector2(cx, cy));
        mCaixasDasTeclas[i] = caixa->AddComponent<DrawCaixaComponent>(
            larguraDaTecla, kTeclaAltura, kTeclaBorda, kTeclaFundo, 3, 110);
        AddActor(std::move(caixa));

        // O rotulo e um ator proprio, por cima da caixa. Texto curto no tamanho
        // do digito, rotulo de acao menor para caber na largura declarada.
        auto rotulo = std::make_unique<Actor>(this);
        rotulo->SetPosition(Vector2(cx, cy));
        const int tamanho = ehAcao ? 24 : 44;
        auto dc = rotulo->AddComponent<DrawTextComponent>(
            tecla.rotulo, mFonte.get(), larguraDaTecla - 16, tamanho + 8, tamanho, 120);
        dc->SetAjustarAoTexto(true);
        AddActor(std::move(rotulo));
    }

    PintarSelecao();
}

void Identificacao::PintarSelecao() const {

    for (size_t i = 0; i < mCaixasDasTeclas.size(); ++i) {
        if (mCaixasDasTeclas[i]) {
            mCaixasDasTeclas[i]->SetMoldura(i == mTeclaSelecionada ? kTeclaAcesa : kTeclaBorda);
        }
    }
}

void Identificacao::Redesenhar() const {

    if (mCampoAtor) {
        if (const auto dc = mCampoAtor->GetComponent<DrawTextComponent>()) {
            // O cursor acompanha o texto em vez de ser um ator separado: assim ele
            // fica sempre colado no ultimo digito, sem conta de posicao nenhuma.
            std::string mostrar = mDigitado;
            if (mCursorAceso && mDigitado.size() < static_cast<size_t>(Matricula::kMaximoDeDigitos)) {
                // BARRA, e nao sublinhado: a Zelda.ttf nao tem o glifo "_", e o
                // cursor saia como aquele quadradinho de caractere faltando. Uma
                // fonte so desenha o que ela tem, e esta nao tem "_[]#*=+".
                mostrar += "|";
            }
            // Espaco, e nao vazio: texto vazio nao gera textura, e o campo sumiria.
            dc->SetText(mostrar.empty() ? " " : mostrar);
        }
    }

    if (mErroAtor) {
        if (const auto dc = mErroAtor->GetComponent<DrawTextComponent>()) {
            dc->SetText(mErro.empty() ? " " : mErro);
        }
    }

    // A moldura avisa junto com a frase: quem errou olha para o campo, nao para o
    // rodape.
    if (mCaixaDesenho) {
        mCaixaDesenho->SetMoldura(mErro.empty() ? kBorda : kBordaErro);
    }
}

void Identificacao::Confirmar(const std::string& canonica) {

    // A PERGUNTA E A MESMA nos dois modos - "ja ha perfil?" -, e so a resposta
    // aceitavel muda. Por isso uma tela so, e nao duas.
    const bool jaTemPerfil = FichaArquivo::Existe(canonica);

    if (mModoNovo && jaTemPerfil) {
        mErro = kJaExiste;
        return;
    }
    if (!mModoNovo && !jaTemPerfil) {
        mErro = kNaoExiste;
        return;
    }

    // Os dois modos passam por IdentificarAluno: para quem esta criando, nao ha
    // arquivo para ler e a sessao comeca vazia, que e exatamente o certo.
    mGame->IdentificarAluno(canonica);

    if (mModoNovo) {
        // AINDA NAO GRAVA NADA. O perfil so nasce quando a personagem e
        // confirmada (D5), e por isso dar ESC na tela de criacao deixa a
        // matricula livre de novo - do contrario, voltar e tentar outra vez
        // esbarraria no proprio perfil recem-criado.
        mGame->RequestSceneChange(SceneType::CriacaoDePersonagem);
        return;
    }

    mGame->RequestSceneChange(SceneType::StageSelect);
}

void Identificacao::Acionar(const Teclado::Tecla& tecla) {

    // UM LUGAR SO decide o que cada acao faz. O botao do gabinete e as teclas do
    // computador chegam os dois aqui, entao nao ha como um caminho fazer uma
    // coisa e o outro fazer outra - que e como telas assim costumam divergir.
    switch (tecla.acao) {

        case Teclado::Acao::Digito: {
            const std::string antes = mDigitado;
            mDigitado = Matricula::Digitar(mDigitado, tecla.digito);
            if (mDigitado != antes) mErro.clear();
            break;
        }

        case Teclado::Acao::Apagar:
            mDigitado = Matricula::Apagar(mDigitado);
            mErro.clear();
            break;

        case Teclado::Acao::Entrar: {
            const auto r = Matricula::Validar(mDigitado);
            if (r.valida) Confirmar(r.canonica);
            else          mErro = Matricula::MensagemDeErro(r.erro);
            break;
        }

        case Teclado::Acao::Visitante:
            // Continua existindo, e agora tem tecla propria: era o TAB, que o
            // gabinete nao tem. Ver o comentario de classe sobre por que entrar
            // sem se identificar e permitido.
            mGame->JogarComoVisitante();
            mGame->RequestSceneChange(SceneType::StageSelect);
            break;

        case Teclado::Acao::Voltar:
            mGame->RequestSceneChange(SceneType::MainMenu);
            break;
    }
}

void Identificacao::LerManche(const Uint8* keyState, const float deltaTime) {

    if (mPassoTimer < kPassoDoManche) mPassoTimer += deltaTime;

    const size_t antes = mTeclaSelecionada;

    if (mPassoTimer >= kPassoDoManche) {
        if (keyState[SDL_SCANCODE_UP]    || keyState[SDL_SCANCODE_W])
            mTeclaSelecionada = Navegacao::Cima(mGradeDoTeclado, mTeclaSelecionada);
        else if (keyState[SDL_SCANCODE_DOWN]  || keyState[SDL_SCANCODE_S])
            mTeclaSelecionada = Navegacao::Baixo(mGradeDoTeclado, mTeclaSelecionada);
        else if (keyState[SDL_SCANCODE_LEFT]  || keyState[SDL_SCANCODE_A])
            mTeclaSelecionada = Navegacao::Esquerda(mGradeDoTeclado, mTeclaSelecionada);
        else if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
            mTeclaSelecionada = Navegacao::Direita(mGradeDoTeclado, mTeclaSelecionada);
    }

    if (mTeclaSelecionada != antes) {
        mPassoTimer = 0.0f;
        PintarSelecao();
    }

    // BOTAO 1 - digita a tecla acesa. ENTER tambem cai aqui, e nao mais direto
    // na validacao: no gabinete quem valida e a tecla ENTRAR da grade.
    const bool acionar = keyState[SDL_SCANCODE_SPACE];
    if (acionar && !mAcionarAnterior) {
        const auto& teclas = Teclado::Teclas();
        if (mTeclaSelecionada < teclas.size()) Acionar(teclas[mTeclaSelecionada]);
    }
    mAcionarAnterior = acionar;
}

void Identificacao::LerTeclado(const Uint8* keyState) {

    for (int d = 0; d < 10; ++d) {
        const bool agora = keyState[kDigitos[d]] || keyState[kDigitosNumerico[d]];
        if (agora && !mDigitoAnterior[d]) {
            const std::string antes = mDigitado;
            mDigitado = Matricula::Digitar(mDigitado, static_cast<char>('0' + d));

            // Limpar o erro ao digitar evita a frase velha contradizendo o campo.
            if (mDigitado != antes) mErro.clear();
        }
        mDigitoAnterior[d] = agora;
    }

    // BOTAO 2 do gabinete, junto com o BACKSPACE: apagar e frequente demais para
    // exigir uma viagem ate a tecla APAGAR da grade a cada erro de digitacao.
    const bool apagar = keyState[SDL_SCANCODE_BACKSPACE] || keyState[SDL_SCANCODE_DELETE]
                        || keyState[SDL_SCANCODE_B];
    if (apagar && !mApagarAnterior) {
        mDigitado = Matricula::Apagar(mDigitado);
        mErro.clear();
    }
    mApagarAnterior = apagar;

    const bool entrar = keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (entrar && !mEnterAnterior) {
        const auto r = Matricula::Validar(mDigitado);
        if (r.valida) Confirmar(r.canonica);
        else          mErro = Matricula::MensagemDeErro(r.erro);
    }
    mEnterAnterior = entrar;

    const bool voltar = keyState[SDL_SCANCODE_ESCAPE];
    if (voltar && !mVoltarAnterior) {
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }
    mVoltarAnterior = voltar;

    const bool visitante = keyState[SDL_SCANCODE_TAB];
    if (visitante && !mVisitanteAnterior) {
        mGame->JogarComoVisitante();
        mGame->RequestSceneChange(SceneType::StageSelect);
    }
    mVisitanteAnterior = visitante;
}

void Identificacao::OnProcessInput(const Uint8* keyState) {
    LerTeclado(keyState);

    // O manche e lido aqui junto com o resto, mas o atraso de repeticao avanca em
    // OnUpdate: e la que existe deltaTime. Passar zero aqui faz o temporizador
    // depender so do que OnUpdate ja somou, que e exatamente o que se quer.
    LerManche(keyState, 0.0f);
}

void Identificacao::OnUpdate(const float deltaTime) {

    if (mPassoTimer < kPassoDoManche) mPassoTimer += deltaTime;

    mPiscaTimer += deltaTime;
    if (mPiscaTimer >= kPiscada) {
        mPiscaTimer -= kPiscada;
        mCursorAceso = !mCursorAceso;
    }

    Redesenhar();
}
