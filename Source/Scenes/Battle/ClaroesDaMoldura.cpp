//
// A moldura do campo pisca pelas bordas a cada acontecimento.
//

#include "ClaroesDaMoldura.h"

#include "../../Components/DrawComponents/DrawMolduraDoCampoComponent.h"

void ClaroesDaMoldura::AoTrocarDeFase(Battle&, const std::string&, const std::string&) {
    // A FASE NOVA passa pelas bordas do campo, em lilas.
    mMoldura.Acender(SDL_Color{196, 170, 230, 255}, 0.45f);
}

void ClaroesDaMoldura::AoSerAcertado(Battle&, const Player&, const bool porUmTiro) {
    // O ACERTO SOFRIDO SE VE: o campo pisca em vermelho pelas bordas. So o do
    // tiro - encostar no chefe ja tem o empurrao e o piscar do aluno.
    if (porUmTiro) mMoldura.Acender(SDL_Color{255, 60, 50, 255}, 0.5f);
}

void ClaroesDaMoldura::AoPegarPontoExtra(Battle&, const Player&) {
    mMoldura.Acender(SDL_Color{255, 215, 80, 255}, 0.3f);
}

void ClaroesDaMoldura::AoUsarPontoExtra(Battle&, const Player&) {
    mMoldura.Acender(SDL_Color{255, 215, 80, 255}, 0.7f);
}

void ClaroesDaMoldura::AoAtravessarAParede(Battle&) {
    mMoldura.Acender(SDL_Color{110, 210, 255, 255}, 0.25f);
}
