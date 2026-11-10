//
// A faixa da nota, na base da tela.
//

#include "FaixaDaNota.h"

#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Components/DrawComponents/DrawBarraDaNotaComponent.h"
#include "../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Regras/Nota.h"

namespace {

/// Duas casas, e nao uma.
std::string ComDuasCasas(const float nota) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << nota;
    return ss.str();
}

} // namespace

FaixaDaNota::FaixaDaNota(Scene& cena, Font* fonte, const int larguraDaTela, const int alturaDaTela,
                         const float nota, const Dificuldade::Nivel nivel)
    : mLarguraDaTela(larguraDaTela), mAlturaDaTela(alturaDaTela) {

    // A barra recuada, e nao de ponta a ponta.
    const int barraX = kMargemLateral;
    const int barraY = alturaDaTela - kDoFundo;
    const int barraLargura = larguraDaTela - 2 * kMargemLateral;

    auto barra = std::make_unique<Actor>(&cena);
    barra->SetPosition(Vector2(static_cast<float>(barraX), static_cast<float>(barraY)));
    // A barra desenhada por conta propria (DrawBarraDaNotaComponent).
    barra->AddComponent<DrawBarraDaNotaComponent>(barraLargura, kAltura, 200)->Comecar(nota);
    mBarra = barra.get();
    cena.AddActor(std::move(barra));

    // O numero no meio da barra. A caixa e 200x32, e nao 36x25.
    constexpr int tamanhoDaNota = 24;
    auto texto = std::make_unique<Actor>(&cena);
    // A placa primeiro, com ordem menor.
    texto->AddComponent<DrawCaixaComponent>(
        kPlacaLargura, kPlacaAltura - 4, kMolduraDaPlacaDaNota, kFundoDaPlacaDaNota, 1, kOrdemDaPlaca);
    // A nota de partida desde o primeiro quadro.
    auto* numero = texto->AddComponent<DrawTextComponent>(
        ComDuasCasas(nota), fonte, 200, tamanhoDaNota + 8, tamanhoDaNota, 304);
    numero->SetAjustarAoTexto(true);

    const auto textoX = static_cast<float>(larguraDaTela) / 2.f;
    const auto textoY = static_cast<float>(barraY + kAltura / 2);
    texto->SetPosition(Vector2(textoX, textoY));
    mTexto = texto.get();
    cena.AddActor(std::move(texto));

    // O anuncio do teste final, escondido ate a nota chegar a 100.
    auto teste = std::make_unique<Actor>(&cena);
    teste->AddComponent<DrawCaixaComponent>(
        kPlacaDoAvisoLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca)->SetIsVisible(false);
    auto* aviso = teste->AddComponent<DrawTextComponent>(
        "TESTE FINAL", fonte, 260, tamanhoDaNota + 8, tamanhoDaNota, 304);
    aviso->SetAjustarAoTexto(true);
    aviso->SetColor(Color::Gold);
    aviso->SetIsVisible(false);
    teste->SetPosition(Vector2(static_cast<float>(larguraDaTela) / 4.f, textoY));
    mTesteFinal = teste.get();
    cena.AddActor(std::move(teste));

    // O aviso de fase limpa, na direita da faixa.
    auto limpa = std::make_unique<Actor>(&cena);
    limpa->AddComponent<DrawCaixaComponent>(
        kPlacaDoAvisoLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca)->SetIsVisible(false);
    auto* faseLimpa = limpa->AddComponent<DrawTextComponent>(
        "FASE LIMPA", fonte, 260, tamanhoDaNota + 8, tamanhoDaNota, 304);
    faseLimpa->SetAjustarAoTexto(true);
    faseLimpa->SetColor(Color::LightGreen);
    faseLimpa->SetIsVisible(false);
    limpa->SetPosition(Vector2(static_cast<float>(larguraDaTela) * 3.f / 4.f, textoY));
    mFaseLimpa = limpa.get();
    cena.AddActor(std::move(limpa));

    // O selo do Dificil, na ponta direita da faixa.
    if (nivel == Dificuldade::Nivel::Dificil) {
        auto selo = std::make_unique<Actor>(&cena);
        selo->AddComponent<DrawCaixaComponent>(
            kPlacaLargura, kPlacaAltura, kSemMoldura, kFundoDaPlaca, 0, kOrdemDaPlaca);
        auto* nome = selo->AddComponent<DrawTextComponent>(
            Dificuldade::Nome(nivel), fonte, kPlacaLargura, tamanhoDaNota + 8, tamanhoDaNota, 304);
        nome->SetAjustarAoTexto(true);
        nome->SetColor(Vector3(Dificuldade::kCorDoDificil[0], Dificuldade::kCorDoDificil[1],
                               Dificuldade::kCorDoDificil[2]));
        selo->SetPosition(Vector2(static_cast<float>(larguraDaTela - kMargemLateral - kPlacaLargura / 2 - 8),
                                  textoY));
        cena.AddActor(std::move(selo));
    }
}

void FaixaDaNota::Atualizar(const float nota, const bool faseLimpa) {
    auto* barra = mBarra ? mBarra->GetComponent<DrawBarraDaNotaComponent>() : nullptr;
    if (!barra) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "GradeBarActor not found");
        return;
    }

    // A cor vai de um extremo ao outro da faixa em que a nota esta. O sulco e a moldura sao da
    // barra (DrawBarraDaNotaComponent).
    static const SDL_Color kReprovadoDe = {100, 0, 0, 255};
    static const SDL_Color kReprovadoAte = {255, 0, 0, 255};
    static const SDL_Color kFinalDe = {180, 150, 0, 255};
    static const SDL_Color kFinalAte = {255, 140, 0, 255};
    static const SDL_Color kAprovadoDe = {144, 238, 144, 255};
    static const SDL_Color kAprovadoAte = {0, 255, 128, 255};

    float fracao = 0.0f;
    SDL_Color de = kAprovadoDe, ate = kAprovadoAte;
    if (nota < 40.f) {
        fracao = nota / 40.f;
        de = kReprovadoDe; ate = kReprovadoAte;
    } else if (nota < 60.f) {
        fracao = (nota - 40.f) / 20.f;
        de = kFinalDe; ate = kFinalAte;
    } else {
        fracao = (nota - 60.f) / 40.f;
    }
    const auto lerp = [fracao](const Uint8 a, const Uint8 b) {
        return static_cast<Uint8>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * fracao);
    };
    const SDL_Color cor{lerp(de.r, ate.r), lerp(de.g, ate.g), lerp(de.b, ate.b), 255};

    barra->SetNota(nota, cor, nota < 40.f, Nota::ECheia(nota));
    AtualizarTexto(nota, faseLimpa);
}

void FaixaDaNota::AtualizarTexto(const float nota, const bool faseLimpa) {
    auto* texto = mTexto->GetComponent<DrawTextComponent>();
    texto->SetText(ComDuasCasas(nota));

    // O dourado segue a nota, em vez de ser ligado uma vez e ficar aceso.
    const bool cheia = Nota::ECheia(nota);
    texto->SetColor(cheia ? Color::Gold : Color::White);

    // O anuncio, ao contrario, nao volta atras.
    if (cheia && mTesteFinal) {
        // A placa acende junto.
        mTesteFinal->GetComponent<DrawCaixaComponent>()->SetIsVisible(true);
        mTesteFinal->GetComponent<DrawTextComponent>()->SetIsVisible(true);
    }

    // O aviso de fase limpa some sozinho.
    if (mFaseLimpa) {
        mFaseLimpa->GetComponent<DrawCaixaComponent>()->SetIsVisible(faseLimpa);
        mFaseLimpa->GetComponent<DrawTextComponent>()->SetIsVisible(faseLimpa);
    }
}

Vector2 FaixaDaNota::Ponta(const float nota) const {
    const float largura = static_cast<float>(mLarguraDaTela - 2 * kMargemLateral);
    return Vector2(static_cast<float>(kMargemLateral) + largura * std::clamp(nota, 0.0f, 100.0f) / 100.0f,
                   static_cast<float>(mAlturaDaTela - kDoFundo + kAltura / 2));
}
