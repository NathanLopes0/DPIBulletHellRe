//
// Os campos que so a ConsultaAttack usa.
//

#pragma once

#include "AttackParams.h"
#include "../../Tabela.h"

/**
 * @brief Uma consulta: qual faixa da tabela varrer, e por qual eixo.
 *
 * Segue o mesmo padrao do BaloonAttackParams: a ConsultaAttack faz um
 * dynamic_cast para esta struct e, se o cast falhar, escreve no log e nao
 * dispara. Por isso o bloco "consulta" e obrigatorio no fases.json.
 */
struct ConsultaAttackParams : AttackParams {

    /// Por qual eixo a varredura corre.
    ///   Linha  = faixa horizontal, projeteis atravessam a tela na horizontal
    ///   Coluna = faixa vertical, projeteis descem (ou sobem)
    enum class Eixo { Linha, Coluna };

    Tabela::Forma forma{};

    Eixo eixo = Eixo::Linha;

    /// Qual faixa varrer. QUEM ESCREVE ISTO E O CHEFE, em CustomizeAttackParams:
    /// numa fase e um contador, noutra e a faixa do proprio jogador. A estrategia
    /// so obedece - ela nao sabe onde o jogador esta, e nao deve saber.
    int indice = 0;

    /**
     * @brief Segundos entre o projetil aparecer e ele disparar.
     *
     * O TELEGRAFO, e a peca mais importante do ataque. Durante este tempo os
     * projeteis ficam parados e VISIVEIS na borda da faixa: e assim que o jogador
     * descobre qual linha vai ser varrida, e e o que separa "dificil" de
     * "injusto". Zero aqui transforma a batalha inteira em adivinhacao.
     */
    float aviso = 0.8f;

    /// Entra pelo outro lado (direita em vez de esquerda; baixo em vez de cima).
    bool invertido = false;
};
