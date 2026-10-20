//
// O escopo que cresce a cada rajada - a regra, sem nada do motor.
//

#pragma once

/**
 * CAMADA PURA
 *
 * Nada aqui conhece Boss, AttackParams nem SDL: sao duas contas e o teste as
 * alcanca. Quem as liga no chefe e MariaLucia::CustomizeAttackParams, que e a
 * parte que a suite nao consegue rodar (precisa de SDL).
 *
 * A IDEIA, para quem for mexer: na INF 221 a fase 1 e o levantamento de
 * requisitos, e requisito nao chega de uma vez - chega MAIS a cada conversa.
 * Entao cada rajada sai maior que a anterior ate a entrega, e aí recomeca do
 * tamanho de origem. E escopo crescendo, e o jogador sente como o espaco livre
 * encolhendo dentro de um mesmo ciclo.
 *
 * SAO ACRESCIMOS, E NAO VALORES ABSOLUTOS, de proposito. O tamanho de partida
 * continua morando em fases.json ("projeteis" e "angulo"), e o C++ so soma por
 * cima. Se devolvesse valores prontos, os numeros do arquivo virariam TEXTO
 * MORTO - que e exatamente a armadilha que "angulo" ja prega em
 * CircleSpreadAttack, onde o leitor aceita o campo e a estrategia o ignora.
 */
namespace Escopo {

    /// O que somar ao que o arquivo pediu, nesta rajada.
    struct Acrescimo {
        int   projeteis = 0;
        float leque     = 0.0f;   ///< graus
    };

    /**
     * @brief O acrescimo da rajada numero 'sprint' da volta atual.
     *
     * @param sprint          quantas rajadas ja sairam nesta volta (0 = a primeira,
     *                        que nao cresce nada - ela E o escopo de origem)
     * @param passoProjeteis  quantos projeteis a mais por rajada
     * @param passoLeque      quantos graus a mais por rajada
     *
     * O LEQUE CRESCE JUNTO COM A CONTAGEM, e nao e enfeite: com o leque fixo,
     * dobrar os projeteis so aproxima um do outro ate a rajada virar uma barra
     * solida - foi o que aconteceu com as tarefas do Vitor a 0,05s de
     * intervalo. Crescendo os dois, o espacamento quase nao muda e o que cresce
     * e a AREA coberta, que e a leitura certa de escopo crescendo: nao e mais
     * denso no mesmo lugar, e mais lugar ocupado.
     *
     * Um sprint negativo e tratado como zero: ninguem deveria chamar assim, e
     * devolver acrescimo negativo encolheria a rajada abaixo do que o arquivo
     * pediu, que e pior do que nao crescer.
     */
    Acrescimo DaRajada(int sprint, int passoProjeteis, float passoLeque);

    /**
     * @brief A rajada seguinte, reiniciando quando a entrega fecha.
     *
     * @param sprint             a rajada atual
     * @param sprintsAteAEntrega quantas rajadas cabem numa volta
     *
     * REINICIA PORQUE ESCOPO NAO CRESCE PARA SEMPRE: em algum momento a versao
     * sai e o ciclo recomeca. Sem o reinicio, uma fase de dezessete segundos a
     * uma rajada por segundo terminaria com rajadas de quarenta projeteis, e aí
     * nao ha o que jogar - seria uma parede, e nao uma fase.
     *
     * Com 'sprintsAteAEntrega' menor que 1 fica preso em zero, em vez de
     * dividir por zero ou crescer sem fim.
     */
    int SprintSeguinte(int sprint, int sprintsAteAEntrega);

}
