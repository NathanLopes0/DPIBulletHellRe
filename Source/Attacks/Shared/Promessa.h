//
// A Promise do JavaScript - a decisao de resolver ou rejeitar.
//

#pragma once

/**
 * CAMADA PURA.
 *
 * Na fase final do Lucas os projeteis sao PROMESSAS: saem, param pendentes e,
 * quando assentam, ou se resolvem - disparam contra o aluno - ou sao
 * rejeitados e somem. Quem faz isso e PromessaBehavior (Behaviors.h); aqui fica
 * a decisao.
 *
 * O SORTEIO ENTRA COMO PARAMETRO, como em RegraSeAplica: e o que deixa o teste
 * passar 0,1 e 0,9 e conferir os dois lados sem depender do gerador.
 */
namespace Promessa {

    /**
     * @brief A promessa se resolve? 'fracao' e a parte das promessas que
     * resolvem, de 0 (nenhuma) a 1 (todas); 'sorteio' e um numero em [0, 1).
     *
     * Resolve quando o sorteio fica ABAIXO da fracao. Com fracao zero nenhuma
     * resolve, e com fracao um todas resolvem - inclusive um sorteio de 1,0,
     * que uniform_real_distribution<float> pode devolver por arredondamento
     * (LWG 2524), embora o intervalo dela seja aberto em cima.
     */
    bool Resolve(float sorteio, float fracao);
}
