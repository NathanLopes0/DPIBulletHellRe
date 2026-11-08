//
// A tela que cobre a batalha enquanto os projeteis sao preparados.
//

#include "TelaDeCarregamento.h"

#include <memory>
#include <string>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Motor/Game.h"
#include "../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"
#include "../../Components/DrawComponents/ProgressBarComponent.h"

namespace {
    /// Acima de tudo, ate da pausa (900).
    constexpr int kOrdemDoVeu = 1000;
    constexpr SDL_Color kFundo{5, 6, 11, 255};
    constexpr int kLarguraDaBarra = 520, kAlturaDaBarra = 14;
}

TelaDeCarregamento::TelaDeCarregamento(Scene& cena, const Materias::Materia* materia, const Dificuldade::Nivel nivel,
                                       Font* fonteDoTitulo, Font* fonteDoTexto) {
    const auto w = static_cast<float>(cena.GetGame()->GetWindowWidth());
    const auto h = static_cast<float>(cena.GetGame()->GetWindowHeight());

    auto veu = std::make_unique<Actor>(&cena);
    veu->SetPosition(Vector2(w / 2.0f, h / 2.0f));
    veu->AddComponent<DrawCaixaComponent>(static_cast<int>(w) + 4, static_cast<int>(h) + 4,
                                          SDL_Color{0, 0, 0, 0}, kFundo, 0, kOrdemDoVeu);
    mVeu = cena.AddActor(std::move(veu));

    // O que vem ai: o codigo da materia, o nome dela e o professor.
    const auto texto = [&](const std::string& t, Font* fonte, const float y, const int tamanho, const Vector3& cor) {
        if (t.empty() || !fonte) return;
        auto ator = std::make_unique<Actor>(&cena);
        ator->SetPosition(Vector2(w / 2.0f, y));
        auto* dc = ator->AddComponent<DrawTextComponent>(t, fonte, 1000, tamanho + 10, tamanho, kOrdemDoVeu + 2);
        dc->SetAjustarAoTexto(true);
        dc->SetColor(cor);
        mTextos.push_back(cena.AddActor(std::move(ator)));
    };
    if (materia) {
        texto(materia->nome, fonteDoTitulo, h * 0.36f, 52, Color::White);
        texto(materia->nomeCompleto, fonteDoTexto, h * 0.44f, 28, Vector3(0.75f, 0.78f, 0.9f));
        if (!materia->professor.empty()) {
            texto("Prof. " + materia->professor, fonteDoTexto, h * 0.50f, 22, Vector3(0.55f, 0.58f, 0.7f));
        }
    }
    if (nivel == Dificuldade::Nivel::Dificil) {
        texto(Dificuldade::Nome(nivel), fonteDoTitulo, h * 0.57f, 24,
              Vector3(Dificuldade::kCorDoDificil[0], Dificuldade::kCorDoDificil[1], Dificuldade::kCorDoDificil[2]));
    }
    texto("preparando a prova", fonteDoTexto, h * 0.66f, 20, Vector3(0.45f, 0.48f, 0.6f));

    auto barra = std::make_unique<Actor>(&cena);
    barra->SetPosition(Vector2((w - static_cast<float>(kLarguraDaBarra)) / 2.0f, h * 0.70f));
    auto* pb = barra->AddComponent<ProgressBarComponent>(kOrdemDoVeu + 1);
    pb->SetSize(kLarguraDaBarra, kAlturaDaBarra);
    pb->SetGradientColors(SDL_Color{22, 24, 36, 255}, SDL_Color{80, 120, 230, 255}, SDL_Color{110, 230, 150, 255});
    pb->SetFillPercent(0.0f);
    mBarra = cena.AddActor(std::move(barra));
}

void TelaDeCarregamento::Avancar(const float feito, const float deltaTime) {
    Carregamento::Avancar(mEstado, feito, deltaTime);
    if (auto* pb = mBarra->GetComponent<ProgressBarComponent>()) {
        pb->SetFillPercent(mEstado.mostrado);
        pb->SetColorPercent(mEstado.mostrado);
    }
    if (!Carregamento::Pronto(mEstado)) return;

    // Pronta: primeiro somem o texto e a barra, depois o veu esmaece sobre a batalha parada.
    for (Actor* t : mTextos) {
        if (auto* dc = t->GetComponent<DrawTextComponent>()) dc->SetIsVisible(false);
    }
    if (auto* pb = mBarra->GetComponent<ProgressBarComponent>()) pb->SetIsVisible(false);
    if (auto* veu = mVeu->GetComponent<DrawCaixaComponent>()) {
        veu->SetFundo(SDL_Color{5, 6, 11, static_cast<Uint8>(255.0f * Carregamento::Opacidade(mEstado))});
    }
}

void TelaDeCarregamento::Remover() {
    for (Actor* t : mTextos) t->SetState(ActorState::Destroy);
    mTextos.clear();
    mVeu->SetState(ActorState::Destroy);
    mBarra->SetState(ActorState::Destroy);
}
