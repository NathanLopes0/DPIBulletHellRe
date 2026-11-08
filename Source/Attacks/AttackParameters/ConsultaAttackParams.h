//
// Os campos que so a ConsultaAttack usa.
//

#pragma once

#include "AttackParams.h"
#include "../../Regras/Tabela.h"

/// Uma consulta: qual faixa da tabela varrer, e por qual eixo.
struct ConsultaAttackParams : AttackParams {

    /// Por qual eixo a varredura corre.
    enum class Eixo { Linha, Coluna };

    Tabela::Forma forma{};

    Eixo eixo = Eixo::Linha;

    /// Qual faixa varrer. Quem escreve isto e o chefe, em CustomizeAttackParams: numa fase e um
    /// contador, noutra e a faixa do proprio jogador.
    int indice = 0;

    /// Segundos entre o projetil aparecer e ele disparar. O telegrafo, e a peca mais importante do
    /// ataque.
    float aviso = 0.8f;

    /// Entra pelo outro lado (direita em vez de esquerda; baixo em vez de cima).
    bool invertido = false;
};
