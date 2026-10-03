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
                        altura * 0.19f, 52, 760, 70);
    Texto(mModoNovo ? "Digite sua matricula para criar o perfil"
                    : "Digite sua matricula para continuar",
          altura * 0.30f, 28, 760, 42);

    // A moldura do campo: um ator proprio, com ordem de desenho menor que a do
    // texto para ficar atras dele.
    auto caixa = std::make_unique<Actor>(this);
    caixa->SetPosition(Vector2(largura / 2.0f, altura * 0.46f));
    mCaixaDesenho = caixa->AddComponent<DrawCaixaComponent>(kCampoLargura, kCampoAltura,
                                                            kBorda, kFundoDoCampo, 3, 110);
    AddActor(std::move(caixa));

    // Os digitos, dentro da caixa. O limite e um pouco menor que ela para o texto
    // nunca encostar na moldura.
    mCampoAtor = Texto(" ", altura * 0.46f, 72, kCampoLargura - 48, kCampoAltura - 28);

    // Largura folgada: as frases de recusa sao bem mais longas que as de erro de
    // formato, e uma delas quebrava linha com os 760 de antes.
    mErroAtor  = Texto(" ", altura * 0.60f, 26, 1040, 40);

    // Linhas proprias em vez de uma longa: assim cada uma fica centrada de
    // verdade, e nenhuma depende de caber numa largura de quebra.
    Texto(mModoNovo ? "ENTER  criar perfil" : "ENTER  entrar", altura * 0.74f, 24, 460, 36);
    Texto("TAB  jogar sem salvar", altura * 0.80f, 24, 460, 36);
    Texto("ESC  voltar ao menu", altura * 0.86f, 24, 460, 36);
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

    const bool apagar = keyState[SDL_SCANCODE_BACKSPACE] || keyState[SDL_SCANCODE_DELETE];
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
}

void Identificacao::OnUpdate(const float deltaTime) {

    mPiscaTimer += deltaTime;
    if (mPiscaTimer >= kPiscada) {
        mPiscaTimer -= kPiscada;
        mCursorAceso = !mCursorAceso;
    }

    Redesenhar();
}
