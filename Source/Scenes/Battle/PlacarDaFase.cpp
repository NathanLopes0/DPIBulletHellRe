//
// O placar da fase.
//

#include "PlacarDaFase.h"

#include <memory>

#include "../Scene.h"
#include "../../Actors/Actor.h"
#include "../../Components/DrawComponents/DrawQuadroComponent.h"
#include "../../Components/DrawComponents/DrawTextComponent.h"

PlacarDaFase::PlacarDaFase(Scene& cena, Font* fonte, const float esquerda) {
    auto placar = std::make_unique<Actor>(&cena);

    // Verde de quadro-negro e moldura de giz.
    auto* quadro = placar->AddComponent<DrawQuadroComponent>(kLargura, kAltura, kOrdem);
    quadro->SetIsVisible(false);

    // A caixa do texto e menor que o quadro, e o ajuste ao texto so encolhe.
    auto* texto = placar->AddComponent<DrawTextComponent>(" ", fonte, kLargura - 24, kAltura - 6, kTamanho, kOrdem + 1);
    texto->SetAjustarAoTexto(true);
    texto->SetIsVisible(false);

    placar->SetPosition(Vector2(esquerda + static_cast<float>(kMargem + kLargura / 2) - 4.0f,
                                static_cast<float>(kMargem + kAltura / 2)));
    mAtor = placar.get();
    cena.AddActor(std::move(placar));
}

void PlacarDaFase::Mostrar(const std::string& anuncio) {
    if (anuncio == mTexto) return;
    mTexto = anuncio;

    // So escreve quando ha o que escrever.
    const bool aceso = !anuncio.empty();
    auto* texto = mAtor->GetComponent<DrawTextComponent>();
    if (aceso) texto->SetText(anuncio);
    texto->SetIsVisible(aceso);
    auto* quadro = mAtor->GetComponent<DrawQuadroComponent>();
    quadro->SetIsVisible(aceso);
    if (aceso) quadro->Apagar();
}
