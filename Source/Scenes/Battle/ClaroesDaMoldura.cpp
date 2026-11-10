//
// A moldura do campo pisca pelas bordas a cada acontecimento.
//

#include "ClaroesDaMoldura.h"

#include "../../Components/DrawComponents/DrawMolduraDoCampoComponent.h"

void ClaroesDaMoldura::AoTrocarDeFase(Battle&, const std::string&, const std::string&) {
    // A fase nova passa pelas bordas do campo, em lilas.
    mMoldura.Acender(SDL_Color{196, 170, 230, 255}, 0.45f);
}

void ClaroesDaMoldura::AoSerAcertado(Battle&, const Player&, const bool porUmTiro) {
    // O acerto sofrido se ve: o campo pisca em vermelho pelas bordas.
    if (porUmTiro) mMoldura.Acender(SDL_Color{255, 60, 50, 255}, 0.5f);
}

void ClaroesDaMoldura::AoPegarPontoExtra(Battle&, const Player&) {
    mMoldura.Acender(SDL_Color{255, 215, 80, 255}, 0.3f);
}

void ClaroesDaMoldura::AoUsarPontoExtra(Battle&, const Player&) {
    mMoldura.Acender(SDL_Color{255, 215, 80, 255}, 0.7f);
}
