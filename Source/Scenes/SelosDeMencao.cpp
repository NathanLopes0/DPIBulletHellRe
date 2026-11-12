//
// Os selos das mencoes de uma materia, numa fileira.
//

#include "SelosDeMencao.h"

#include <memory>

#include <SDL_pixels.h>

#include "Scene.h"
#include "TextoDeCena.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawCaixaComponent.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

namespace {
    constexpr int kLetra = 13;
    constexpr SDL_Color kFundoApagado{14, 18, 32, 210};
    constexpr SDL_Color kMolduraApagada{66, 76, 100, 255};
    const Vector3 kTintaApagada(0.36f, 0.40f, 0.48f);
    const Vector3 kTintaAcesa(0.07f, 0.08f, 0.13f);
}

SelosDeMencao::SelosDeMencao(Scene& cena, Font* fonte, const Vector2& esquerda, const int ordem, const bool compacta) {
    const float largura = compacta ? kLarguraCompacta : kLargura;
    const float entre = compacta ? kEntreCompacta : kEntre;
    for (int i = 0; i < Mencoes::kQuantas; ++i) {
        // Os desafios vem depois de uma folga: sao outro jeito de jogar, e nao mais nota.
        const bool desafio = Mencoes::kTodas[static_cast<size_t>(i)] >= Mencoes::kDesafios.front();
        const Vector2 onde(esquerda.x + largura / 2.0f + static_cast<float>(i) * (largura + entre) +
                               (desafio ? kEntreOsGrupos : 0.0f),
                           esquerda.y);
        auto caixa = std::make_unique<Actor>(&cena);
        caixa->SetPosition(onde);
        mSelos[static_cast<size_t>(i)].caixa = caixa->AddComponent<DrawCaixaComponent>(
            static_cast<int>(largura), static_cast<int>(kAltura), kMolduraApagada, kFundoApagado, 1, ordem);
        cena.AddActor(std::move(caixa));
        mSelos[static_cast<size_t>(i)].texto = CriarTexto(cena, fonte, Mencoes::Selo(Mencoes::kTodas[static_cast<size_t>(i)]),
                                                          onde, kLetra, static_cast<int>(largura) - 2,
                                                          static_cast<int>(kAltura), ordem + 1);
    }
    Esconder();
}

void SelosDeMencao::Mostrar(const Mencoes::DaMateria& mencoes) {
    for (int i = 0; i < Mencoes::kQuantas; ++i) {
        const Mencoes::Mencao m = Mencoes::kTodas[static_cast<size_t>(i)];
        const Selo& s = mSelos[static_cast<size_t>(i)];
        const bool existe = mencoes.Existe(m);
        s.caixa->SetIsVisible(existe);
        s.texto->SetIsVisible(existe);
        if (!existe) continue;
        if (mencoes.Tem(m)) {
            const Mencoes::Cor c = Mencoes::CorDa(m);
            s.caixa->SetFundo(SDL_Color{c.r, c.g, c.b, 255});
            s.caixa->SetMoldura(SDL_Color{255, 255, 255, 255});
            s.texto->SetColor(kTintaAcesa);
        } else {
            s.caixa->SetFundo(kFundoApagado);
            s.caixa->SetMoldura(kMolduraApagada);
            s.texto->SetColor(kTintaApagada);
        }
    }
}

void SelosDeMencao::Esconder() {
    for (const Selo& s : mSelos) {
        s.caixa->SetIsVisible(false);
        s.texto->SetIsVisible(false);
    }
}
