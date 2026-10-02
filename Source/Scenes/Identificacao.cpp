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

void Identificacao::CriarTextos() {

    const auto largura = static_cast<float>(mGame->GetWindowWidth());
    const auto altura  = static_cast<float>(mGame->GetWindowHeight());

    auto texto = [&](const std::string& inicial, const float y, const int tamanho,
                     const int caixaLargura, const int caixaAltura) {
        auto ator = std::make_unique<Actor>(this);
        ator->SetPosition(Vector2(largura / 2.0f, y));
        ator->AddComponent<DrawTextComponent>(inicial, mFonte.get(),
                                              caixaLargura, caixaAltura, tamanho, 255);
        Actor* bruto = ator.get();
        AddActor(std::move(ator));
        return bruto;
    };

    mTituloAtor = texto("Digite sua matricula", altura * 0.25f, 48, 560, 60);
    mCampoAtor  = texto("_",                    altura * 0.45f, 72, 420, 90);
    mErroAtor   = texto(" ",                    altura * 0.60f, 28, 700, 40);
    mAjudaAtor  = texto("ENTER para entrar   -   TAB para jogar sem salvar",
                        altura * 0.78f, 24, 760, 40);
}

void Identificacao::Redesenhar() const {

    if (mCampoAtor) {
        if (const auto dc = mCampoAtor->GetComponent<DrawTextComponent>()) {
            // O tracinho e so para o campo vazio nao parecer quebrado.
            dc->SetText(mDigitado.empty() ? "_" : mDigitado);
        }
    }

    if (mErroAtor) {
        if (const auto dc = mErroAtor->GetComponent<DrawTextComponent>()) {
            // Espaco, e nao string vazia: o DrawTextComponent constroi uma textura a
            // partir do texto, e texto vazio nao da textura nenhuma.
            dc->SetText(mErro.empty() ? " " : mErro);
        }
    }
}

void Identificacao::LerTeclado(const Uint8* keyState) {

    for (int d = 0; d < 10; ++d) {
        const bool agora = keyState[kDigitos[d]] || keyState[kDigitosNumerico[d]];
        if (agora && !mDigitoAnterior[d]) {
            // Quem decide se o digito entra e Matricula::Digitar: aqui so se sabe
            // que uma tecla foi apertada.
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

void Identificacao::OnUpdate(float deltaTime) {
    (void)deltaTime;
    Redesenhar();
}
