//
// A ponte entre a exportacao e o disco.
//

#pragma once

#include <string>

#include "Materias.h"

namespace Exportacao {

    /**
     * @brief Regrava <base>/Saves/notas.csv com as notas de todos os alunos.
     *
     * Chamada depois de cada gravacao de ficha, entao o arquivo esta sempre em dia
     * e o professor nao precisa lembrar de exportar nada. O custo e uma varredura
     * da pasta de saves por batalha terminada, o que e irrelevante perto dos
     * dezessete segundos de uma fase.
     *
     * O CSV e DERIVADO: a verdade sao as fichas. Apagar notas.csv nao perde nada -
     * ele volta na proxima batalha.
     *
     * Devolve false e registra no log quando nao conseguiu; nunca lanca, e nunca
     * impede o jogo de seguir.
     */
    bool Regravar(const Materias::Lista& materias);
}
