//
// A ponte entre as materias lidas de arquivo e o jogo.
//

#pragma once

#include "Materias.h"

namespace Materias {

    /**
     * @brief As materias do curso, lidas de Assets/materias.json na primeira chamada.
     *
     * Devolve referencia para uma lista que vive enquanto o processo viver: ela e
     * consultada a cada troca de tela e a cada gravacao de ficha, e reler o arquivo
     * toda vez nao traria nada.
     *
     * Arquivo ausente ou ilegivel devolve lista VAZIA, com o motivo no log. Quem
     * chama precisa aguentar isso: com lista vazia nao ha o que jogar, mas o jogo
     * nao deve cair - um erro de instalacao tem de aparecer como mensagem, nao como
     * fechamento.
     */
    const Lista& Carregadas();
}
