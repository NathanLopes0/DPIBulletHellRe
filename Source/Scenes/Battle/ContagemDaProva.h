//
// O que a prova contou, para as horas complementares.
//

#pragma once

#include "OuvinteDaBatalha.h"

/// Ouve a batalha (OuvinteDaBatalha) e conta o que as atividades complementares perguntam (Horas.h)
/// e mais ninguem guarda: os raspoes, os pontos extras usados, as coberturas e a menor nota a que a
/// prova chegou. Da dupla inteira: a prova e dos dois.
class ContagemDaProva : public OuvinteDaBatalha {
public:
    void AoFimDoQuadro(Battle& b) override;
    void AoRaspar(Battle&, const Player&) override { ++mRaspoes; }
    void AoUsarPontoExtra(Battle&, const Player&) override { ++mPontosExtras; }
    void AoCobrir(Battle&, const Player&, const Player&) override { ++mCoberturas; }

    [[nodiscard]] int Raspoes() const { return mRaspoes; }
    [[nodiscard]] int PontosExtras() const { return mPontosExtras; }
    [[nodiscard]] int Coberturas() const { return mCoberturas; }
    [[nodiscard]] float MenorNota() const { return mMenorNota; }

private:
    int mRaspoes = 0;
    int mPontosExtras = 0;
    int mCoberturas = 0;
    float mMenorNota = 100.0f;
};
