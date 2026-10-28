//
// A ponte entre o registro das tentativas e o disco.
//

#pragma once

#include "Tentativas.h"

namespace TentativasArquivo {

    /**
     * @brief Acrescenta a tentativa a <base>/Saves/tentativas.csv e
     * tentativas_fases.csv, e refaz tentativas_resumo.csv.
     *
     * A VEZ de cada aluno (1 na primeira tentativa dele naquela materia) sai do
     * proprio tentativas.csv, lido antes de acrescentar. Arquivo novo ganha o
     * cabecalho.
     *
     * Devolve false e registra no log quando nao conseguiu; nunca lanca, e nunca
     * impede o jogo de seguir - perder uma linha de dados e ruim, travar a
     * batalha do aluno e pior.
     */
    bool Gravar(const Tentativas::Tentativa& tentativa);
}
