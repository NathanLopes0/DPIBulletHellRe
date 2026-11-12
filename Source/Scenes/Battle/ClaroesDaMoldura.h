//
// A moldura do campo pisca pelas bordas a cada acontecimento.
//

#pragma once

#include "OuvinteDaBatalha.h"

class DrawMolduraDoCampoComponent;

/// Ouve a batalha (OuvinteDaBatalha) e acende a moldura do campo. Cada clarao some sozinho
/// (DrawMolduraDoCampoComponent).
class ClaroesDaMoldura : public OuvinteDaBatalha {
public:
    explicit ClaroesDaMoldura(DrawMolduraDoCampoComponent& moldura) : mMoldura(moldura) {}

    void AoTrocarDeFase(Battle& b, const std::string& fase, const std::string& anuncio) override;
    void AoSerAcertado(Battle& b, const Player& aluno, bool porUmTiro) override;
    void AoPegarPontoExtra(Battle& b, const Player& aluno) override;
    void AoUsarPontoExtra(Battle& b, const Player& aluno) override;

private:
    DrawMolduraDoCampoComponent& mMoldura;
};
