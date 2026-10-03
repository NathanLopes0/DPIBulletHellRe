//
// A tela em que o aluno digita a matricula.
//

#include "Identificacao.h"

#include <SDL_scancode.h>

#include "../Game.h"
#include "../Font.h"
#include "../Matricula.h"
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
}

Identificacao::Identificacao(Game* game)
    : Scene(game, SceneType::Identificacao),
      mFonte(std::make_unique<Font>())
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
    mTituloAtor = Texto("IDENTIFICACAO", altura * 0.19f, 52, 760, 70);
    Texto("Digite sua matricula", altura * 0.30f, 28, 620, 42);

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

    mErroAtor  = Texto(" ", altura * 0.60f, 26, 760, 40);
    // Duas linhas proprias em vez de uma longa: assim cada uma fica centrada de
    // verdade, e nenhuma depende de caber numa largura de quebra.
    Texto("ENTER  entrar", altura * 0.76f, 24, 420, 36);
    Texto("TAB  jogar sem salvar", altura * 0.83f, 24, 460, 36);
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
        if (r.valida) {
            mGame->IdentificarAluno(r.canonica);
            mGame->RequestSceneChange(SceneType::StageSelect);
        }
        else {
            mErro = Matricula::MensagemDeErro(r.erro);
        }
    }
    mEnterAnterior = entrar;

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
