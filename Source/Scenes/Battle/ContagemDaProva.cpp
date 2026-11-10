//
// O que a prova contou, para as horas complementares.
//

#include "ContagemDaProva.h"

#include <algorithm>

#include "Battle.h"

void ContagemDaProva::AoFimDoQuadro(Battle& b) {
    if (b.EmCombate()) mMenorNota = std::min(mMenorNota, b.NotaAtual());
}
