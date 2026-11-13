//
// Os selos das mencoes de uma materia, numa fileira.
//

#include "SelosDeMencao.h"

#include <memory>
#include <string>

#include "Scene.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Motor/CaminhosArquivo.h"

SelosDeMencao::SelosDeMencao(Scene& cena, const Vector2& esquerda, const int ordem, const bool compacta) {
    const float largura = compacta ? kLarguraCompacta : kLargura;
    const float entre = compacta ? kEntreCompacta : kEntre;
    for (int i = 0; i < Mencoes::kQuantas; ++i) {
        // Os desafios vem depois de uma folga: sao outro jeito de jogar, e nao mais nota.
        const bool desafio = Mencoes::kTodas[static_cast<size_t>(i)] >= Mencoes::kDesafios.front();
        const Vector2 onde(esquerda.x + largura / 2.0f + static_cast<float>(i) * (largura + entre) +
                               (desafio ? kEntreOsGrupos : 0.0f),
                           esquerda.y);
        auto ator = std::make_unique<Actor>(&cena);
        ator->SetPosition(onde);
        auto* selo = ator->AddComponent<DrawAnimatedComponent>(Caminhos::Asset("Icons/DPIBHSelos.png"),
                                                               Caminhos::Asset("Icons/DPIBHSelos.json"), ordem);
        // Na folha, o aceso da mencao i e o quadro 2i, e o apagado o 2i + 1 (gerar_interface.py).
        selo->AddAnimation("aceso", {2 * i});
        selo->AddAnimation("apagado", {2 * i + 1});
        selo->SetAnimation("apagado");
        selo->SetEscalaPropria(kEscala);
        mSelos[static_cast<size_t>(i)] = selo;
        cena.AddActor(std::move(ator));
    }
    Esconder();
}

void SelosDeMencao::Mostrar(const Mencoes::DaMateria& mencoes) {
    for (int i = 0; i < Mencoes::kQuantas; ++i) {
        const Mencoes::Mencao m = Mencoes::kTodas[static_cast<size_t>(i)];
        DrawAnimatedComponent* selo = mSelos[static_cast<size_t>(i)];
        const bool existe = mencoes.Existe(m);
        selo->SetIsVisible(existe);
        if (existe) selo->SetAnimation(mencoes.Tem(m) ? "aceso" : "apagado");
    }
}

void SelosDeMencao::Esconder() {
    for (DrawAnimatedComponent* selo : mSelos) selo->SetIsVisible(false);
}
