//
// A serpente: sequi, "seguir".
//

#pragma once

#include <string>
#include <vector>

#include "../Math.h"

/**
 * CAMADA PURA.
 *
 * Na fase final do Luizinho (Sequenciadores) o ataque e uma SERPENTE: a cabeca
 * persegue o aluno e cada gomo do corpo passa exatamente por onde a cabeca
 * passou - sequi, "seguir". E o sequenciador da materia e o goto: de tempos em
 * tempos a cabeca SALTA para um rotulo, e o corpo salta junto quando chega no
 * ponto do salto, como as instrucoes que vem depois de um goto.
 *
 * Quem faz isso no jogo sao os behaviors da serpente (Behaviors.h) e o
 * Luizinho, que escolhe o rotulo; aqui ficam o rastro, o giro da cabeca e os
 * numeros romanos dos rotulos.
 */
namespace Rastro {

    /**
     * @brief O caminho por onde a cabeca passou, medido pelo comprimento.
     *
     * Cada ponto marcado soma ao comprimento a distancia ate o anterior - a
     * menos que seja um SALTO, que nao soma nada: o goto leva a cabeca de um
     * lugar a outro sem passar pelo meio. Um gomo que esta 'd' pixels atras da
     * cabeca esta no ponto do caminho em (Comprimento() - d).
     */
    class Caminho {
    public:
        /// Marca onde a cabeca esta agora. O primeiro ponto nunca e salto.
        void Marcar(Vector2 ponto, bool salto = false);

        /// O comprimento percorrido ate o ultimo ponto, sem contar os saltos.
        [[nodiscard]] float Comprimento() const;

        /// Onde esta o ponto 'distancia' pixels atras do ultimo, pelo caminho.
        /// Antes do comeco, devolve o primeiro ponto: o gomo ainda nao saiu de
        /// onde a serpente nasceu. Sem nenhum ponto, devolve zero.
        [[nodiscard]] Vector2 Atras(float distancia) const;

        /// A direcao do caminho nesse mesmo ponto: para onde o gomo esta indo.
        /// Num salto, ou sem caminho ainda, e a direcao do trecho seguinte.
        [[nodiscard]] Vector2 DirecaoAtras(float distancia) const;

        /// O ponto e a direcao na posicao 's' do caminho, contada do comeco. E
        /// como o gomo anda: ele soma a propria velocidade a 's', e o caminho
        /// diz onde isso fica. Depois do fim, vale o ultimo ponto.
        [[nodiscard]] Vector2 Em(float s) const { return Atras(Comprimento() - s); }
        [[nodiscard]] Vector2 DirecaoEm(float s) const { return DirecaoAtras(Comprimento() - s); }

        [[nodiscard]] bool Vazio() const { return mPontos.empty(); }

    private:
        struct Ponto { Vector2 p; float s; bool salto; };
        std::vector<Ponto> mPontos;
        /// O indice do trecho que contem a posicao 's' do caminho.
        [[nodiscard]] size_t Trecho(float s) const;
    };

    /// @brief O que a cabeca e os gomos de UMA serpente dividem: o caminho.
    /// Mora num shared_ptr dos behaviors - morre com o ultimo gomo.
    struct Serpente {
        Caminho caminho;
    };

    /**
     * @brief O goto do professor: um contador e o rotulo do ultimo salto.
     *
     * Quem escreve e o Luizinho, quando o aviso acaba; quem le e a cabeca de
     * cada serpente, que salta quando ve o contador mudar. Um contador, e nao
     * um "salte agora", para que a cabeca que ainda nao nasceu nao salte por um
     * goto que ja passou.
     */
    struct Desvio {
        int geracao = 0;
        Vector2 rotulo;
    };

    /**
     * @brief A cabeca vira em direcao ao alvo, no maximo 'giroMaximo' radianos
     * neste quadro. Devolve a direcao nova, unitaria; com o alvo atras, vira
     * pelo lado que for (o de giro positivo).
     */
    Vector2 Girar(Vector2 direcao, Vector2 paraOAlvo, float giroMaximo);

    /// @brief O numero em algarismos romanos, de 1 a 3999. Fora disso, vazio.
    std::string Romano(int n);
}
