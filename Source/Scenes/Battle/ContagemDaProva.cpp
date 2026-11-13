//
// O que a prova contou, para as horas complementares.
//

#include "ContagemDaProva.h"

#include <algorithm>

#include "Battle.h"
#include "../../Actors/Player/Player.h"

void ContagemDaProva::AoAcertarOProfessor(Battle&, const Actor* quemAtirou, bool) {
    if (dynamic_cast<const Player*>(quemAtirou)) ++mAcertosNoProfessor;
}

void ContagemDaProva::AoFimDoQuadro(Battle& b) {
    if (b.EmCombate()) mMenorNota = std::min(mMenorNota, b.NotaAtual());
}
