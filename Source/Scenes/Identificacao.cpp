//
// A tela em que o aluno digita a matricula.
//

#include "Identificacao.h"
#include "TextoDeCena.h"
#include "../Entrada/Painel.h"
#include "FundoDeCena.h"

#include <SDL_scancode.h>

#include "../Motor/Game.h"
#include "../Motor/Font.h"
#include "../Aluno/Matricula.h"
#include "../Aluno/FichaArquivo.h"
#include "../Motor/CaminhosArquivo.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {

    /// Os scancodes dos digitos da fileira de cima, na ordem 0..9.
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

    /// Quanto tempo o cursor fica aceso e apagado. --- O teclado digital ---.
    constexpr int kTeclaLargura  = 90;    // as teclas de digito
    constexpr int kAcaoLargura   = 240;   // a coluna de acoes, que tem rotulo
    constexpr int kTeclaAltura   = 70;
    constexpr int kVao           = 14;    // entre teclas
    constexpr int kVaoDaColuna   = 40;    // entre os digitos e as acoes
    // O teclado comeca abaixo da caixa da matricula, que vai ate 319 (centro em 0,33 da altura, 110
    // de alto).
    constexpr int kTopoDoTeclado = 380;

    constexpr SDL_Color kTeclaBorda   = { 90,  96, 110, 255};
    constexpr SDL_Color kTeclaAcesa   = {255, 214,   0, 255};   // Color::Gold, em SDL_Color
    constexpr SDL_Color kTeclaFundo   = { 24,  26,  32, 220};

    constexpr float kPiscada = 0.5f;

    // As duas recusas de fluxo ficam aqui, e nao em Matricula::MensagemDeErro, porque nao falam da
    // matricula: "89384" e igualmente valida nos dois casos.
    const char* kJaEstaJogando = "Essa matricula ja esta no outro lado do painel.";
}

Identificacao::Identificacao(Game* game)
    : Scene(game, SceneType::Identificacao),
      mFonte(std::make_unique<Font>()),
      mQuem(game->PosicaoEntrando()),
      mEmDupla(game->QuantosJogadores() > 1)
{
    mFonte->Load(Caminhos::Asset("Fonts/Zelda.ttf"));
}

void Identificacao::Load() {
    CriarFundoDoCorredor(*this);
    CriarTextos();
    Redesenhar();
}

Actor* Identificacao::Texto(const std::string& inicial, const float y, const int tamanho,
                            const int larguraMaxima, const int alturaMaxima) {
    // Centrado na largura: todo texto desta tela fica no eixo do meio.
    return CriarTexto(*this, mFonte.get(), inicial,
                      Vector2(static_cast<float>(mGame->GetWindowWidth()) / 2.0f, y),
                      tamanho, larguraMaxima, alturaMaxima)->GetOwner();
}

void Identificacao::CriarTextos() {

    const auto altura = static_cast<float>(mGame->GetWindowHeight());
    const auto largura = static_cast<float>(mGame->GetWindowWidth());

    // O titulo tinha sumido por causa disso.
    const std::string titulo = mEmDupla ? ("JOGADOR " + std::to_string(mQuem + 1))
                                        : std::string("SUA MATRICULA");

    const char* ajuda = "Digite sua matricula, ou entre como visitante";

    mTituloAtor = Texto(titulo, altura * 0.13f, 52, 760, 70);
    Texto(ajuda, altura * 0.21f, 28, 760, 42);

    auto caixa = std::make_unique<Actor>(this);
    caixa->SetPosition(Vector2(largura / 2.0f, altura * 0.33f));
    mCaixaDesenho = caixa->AddComponent<DrawCaixaComponent>(kCampoLargura, kCampoAltura,
                                                            kBorda, kFundoDoCampo, 3, 110);
    AddActor(std::move(caixa));

    // Os digitos, dentro da caixa.
    mCampoAtor = Texto(" ", altura * 0.33f, 72, kCampoLargura - 48, kCampoAltura - 28);

    mErroAtor  = Texto(" ", altura * 0.425f, 26, 1040, 40);

    // Linhas proprias em vez de uma longa.
    Texto(Painel::Rodape({Painel::Manche(PainelDaVez(), "escolher"),
                          {PainelDaVez(), Painel::Botao::Um,   "digitar"},
                          {PainelDaVez(), Painel::Botao::Dois, "apagar"}},
                         mGame->JeitoDoRodape()),
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

        // O rotulo e um ator proprio, por cima da caixa.
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
            // O cursor acompanha o texto em vez de ser um ator separado.
            std::string mostrar = mDigitado;
            if (mCursorAceso && mDigitado.size() < static_cast<size_t>(Matricula::kMaximoDeDigitos)) {
                // Uma fonte so desenha o que ela tem, e esta nao tem "_[]#*=+".
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

    // Antes de olhar o disco: o parceiro ja esta com esta matricula?
    if (Sessao::JaEstaEmUso(mGame->Sessoes(), mQuem, canonica)) {
        mErro = kJaEstaJogando;
        return;
    }

    // A matricula responde sozinha: ha arquivo, abre; nao ha, cria.
    const bool jaTemPerfil = FichaArquivo::Existe(canonica);

    // IdentificarAluno vale nos dois casos.
    mGame->IdentificarAluno(canonica);

    // Quem nao tem perfil monta a personagem.
    if (!jaTemPerfil) {
        // Ainda nao grava nada.
        mGame->RequestSceneChange(SceneType::CriacaoDePersonagem);
        return;
    }

    mGame->AvancarEntrada();
}

void Identificacao::Acionar(const Teclado::Tecla& tecla) {

    // Um lugar so decide o que cada acao faz.
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
            mGame->JogarComoVisitante();
            mGame->AvancarEntrada();
            break;

        case Teclado::Acao::Voltar:
            // Desistir desliga a dupla.
            mGame->CancelarEntrada();
            mGame->RequestSceneChange(SceneType::MainMenu);
            break;
    }
}

void Identificacao::LerManche(const Uint8* keyState, const float deltaTime) {

    if (mPassoTimer < kPassoDoManche) mPassoTimer += deltaTime;

    const size_t antes = mTeclaSelecionada;

    if (mPassoTimer >= kPassoDoManche) {
        const Painel::Jogador kQuem = PainelDaVez();

        if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Cima))
            mTeclaSelecionada = Navegacao::Cima(mGradeDoTeclado, mTeclaSelecionada);
        else if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Baixo))
            mTeclaSelecionada = Navegacao::Baixo(mGradeDoTeclado, mTeclaSelecionada);
        else if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Esquerda))
            mTeclaSelecionada = Navegacao::Esquerda(mGradeDoTeclado, mTeclaSelecionada);
        else if (Painel::Apertado(keyState, kQuem, Painel::Direcao::Direita))
            mTeclaSelecionada = Navegacao::Direita(mGradeDoTeclado, mTeclaSelecionada);
    }

    if (mTeclaSelecionada != antes) {
        mPassoTimer = 0.0f;
        PintarSelecao();
    }

    // Botao 1 - digita a tecla acesa.
    const bool acionar = Painel::Apertado(keyState, PainelDaVez(), Painel::Botao::Um);
    if (mBordaAcionar.Bateu(acionar)) {
        const auto& teclas = Teclado::Teclas();
        if (mTeclaSelecionada < teclas.size()) Acionar(teclas[mTeclaSelecionada]);
    }
}

void Identificacao::LerTeclado(const Uint8* keyState) {

    for (int d = 0; d < 10; ++d) {
        const bool agora = keyState[kDigitos[d]] || keyState[kDigitosNumerico[d]];
        if (mBordaDigito[d].Bateu(agora)) {
            const std::string antes = mDigitado;
            mDigitado = Matricula::Digitar(mDigitado, static_cast<char>('0' + d));

            // Limpar o erro ao digitar evita a frase velha contradizendo o campo.
            if (mDigitado != antes) mErro.clear();
        }
    }

    // Botao 2 do gabinete, junto com o backspace.
    const bool apagar = Painel::Apertado(keyState, PainelDaVez(), Painel::Botao::Dois)
                        || keyState[SDL_SCANCODE_BACKSPACE] || keyState[SDL_SCANCODE_DELETE];
    if (mBordaApagar.Bateu(apagar)) {
        mDigitado = Matricula::Apagar(mDigitado);
        mErro.clear();
    }

    const bool entrar = keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER];
    if (mBordaEnter.Bateu(entrar)) {
        const auto r = Matricula::Validar(mDigitado);
        if (r.valida) Confirmar(r.canonica);
        else          mErro = Matricula::MensagemDeErro(r.erro);
    }

    const bool voltar = keyState[SDL_SCANCODE_ESCAPE];
    if (mBordaVoltar.Bateu(voltar)) {
        mGame->CancelarEntrada();
        mGame->RequestSceneChange(SceneType::MainMenu);
        return;
    }

    const bool visitante = keyState[SDL_SCANCODE_TAB];
    if (mBordaVisitante.Bateu(visitante)) {
        mGame->JogarComoVisitante();
        mGame->AvancarEntrada();
    }
}

void Identificacao::OnProcessInput(const Uint8* keyState) {
    LerTeclado(keyState);

    // O manche e lido aqui junto com o resto, mas o atraso de repeticao avanca em OnUpdate.
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
