//
// Paineis e linhas de codigo desenhados no campo.
//

#include "PainelDeCodigo.h"

#include "../../../Components/DrawComponents/DrawCaixaComponent.h"
#include "../../../Components/DrawComponents/DrawTextComponent.h"
#include "../../../Scenes/Battle/Battle.h"

PainelDeCodigo::PainelDeCodigo(Scene* cena, const int largura, const int altura, const int ordem)
    : Actor(cena)
{
    mCaixa = AddComponent<DrawCaixaComponent>(largura, altura, SDL_Color{110, 130, 170, 200},
                                              SDL_Color{24, 30, 48, 110}, 2, ordem);
}

void PainelDeCodigo::Pintar(const SDL_Color moldura, const SDL_Color fundo) {
    mCaixa->SetMoldura(moldura);
    mCaixa->SetFundo(fundo);
}

void PainelDeCodigo::Mostrar(const bool mostrar) {
    mCaixa->SetIsVisible(mostrar);
}

EtiquetaDeCodigo::EtiquetaDeCodigo(Scene* cena, const std::string& texto, const int largura,
                                   const int altura, const int ordem)
    : Actor(cena)
{
    mCaixa = AddComponent<DrawCaixaComponent>(largura, altura, SDL_Color{120, 140, 180, 230},
                                              SDL_Color{28, 32, 46, 225}, 2, ordem);

    // SEM BATALHA NAO HA FONTE, e a etiqueta fica so com a caixa - nao ha
    // outra cena que desenhe codigo no campo.
    const auto* batalha = dynamic_cast<Battle*>(cena);
    if (!batalha || !batalha->FonteDoPlacar()) return;
    mTexto = AddComponent<DrawTextComponent>(texto, batalha->FonteDoPlacar(), largura - 10, altura - 4,
                                             kTamanhoDaLetra, ordem + 1);
    mTexto->SetAjustarAoTexto(true);
    mTexto->SetLarguraDeQuebra(2000);
}

void EtiquetaDeCodigo::Pintar(const SDL_Color moldura, const SDL_Color fundo, const SDL_Color tinta) {
    mCaixa->SetMoldura(moldura);
    mCaixa->SetFundo(fundo);
    if (mTexto) {
        mTexto->SetColor(Vector3(static_cast<float>(tinta.r) / 255.0f, static_cast<float>(tinta.g) / 255.0f,
                                 static_cast<float>(tinta.b) / 255.0f));
    }
}

void EtiquetaDeCodigo::Escrever(const std::string& texto) {
    if (mTexto) mTexto->SetText(texto);
}

void EtiquetaDeCodigo::Mostrar(const bool mostrar) {
    mCaixa->SetIsVisible(mostrar);
    if (mTexto) mTexto->SetIsVisible(mostrar);
}
