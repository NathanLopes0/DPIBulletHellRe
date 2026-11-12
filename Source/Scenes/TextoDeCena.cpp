//
// O texto de tela que as cenas compartilham.
//

#include "TextoDeCena.h"

#include <memory>

#include "Scene.h"
#include "../Actors/Actor.h"
#include "../Components/DrawComponents/DrawTextComponent.h"

DrawTextComponent* CriarTexto(Scene& cena, Font* fonte, const std::string& texto, const Vector2& onde,
                              const int tamanho, const int largura, const int altura, const int ordem) {
    auto ator = std::make_unique<Actor>(&cena);
    ator->SetPosition(onde);
    auto* dc = ator->AddComponent<DrawTextComponent>(texto.empty() ? " " : texto, fonte,
                                                     largura, altura, tamanho, ordem);
    dc->SetLarguraDeQuebra(static_cast<unsigned>(largura));
    dc->SetAjustarAoTexto(true);
    cena.AddActor(std::move(ator));
    return dc;
}
