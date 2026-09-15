//
// Formas de caminho prontas para o PathBehavior.
//

#pragma once

#include <vector>
#include "../Math.h"

/**
 * @namespace PathShapes
 * @brief Geradores de caminhos para o PathBehavior.
 *
 * ESPAÇO DE CAMINHO
 * Todas as funções devolvem offsets, não coordenadas de tela:
 *   +X = a direção em que o projétil viaja quando o PathBehavior ativa
 *   +Y = a perpendicular, à direita desse movimento
 *
 * Ou seja, você desenha o caminho como se o projétil sempre saísse para a
 * direita, e o PathBehavior gira tudo para a direção real do disparo. Isso
 * significa que a mesma forma funciona em qualquer ângulo, e que um anel de
 * CircleSpreadAttack produz a forma girada para cada direção do anel.
 *
 * NÚMERO DE SEGMENTOS
 * Mais segmentos deixam a curva mais suave, mas cada waypoint é um alvo que o
 * projétil precisa alcançar dentro de uma tolerância. Muitos pontos muito
 * próximos fazem o projétil gastar frames "chegando" em cada um e a trajetória
 * fica truncada. Entre 8 e 16 costuma ser o ponto certo.
 *
 * LEGIBILIDADE ANTES DE TUDO
 * Caminho elaborado vira bala imprevisível, e bala imprevisível é injusta, não
 * difícil. Prefira uma forma que o jogador consiga descrever depois de ver
 * duas vezes.
 */
namespace PathShapes {

    /**
     * @brief Arco: avança enquanto faz uma barriga para um dos lados e volta
     * ao eixo no fim. Lê como um tiro que "contorna" alguma coisa.
     *
     * @param forward Distância total percorrida na direção do disparo.
     * @param lateral Largura da barriga. Negativo faz a curva para o outro
     *        lado — alterne o sinal entre projéteis vizinhos para que os arcos
     *        se cruzem.
     * @param segments Quantidade de pontos da curva.
     */
    std::vector<Vector2> Arc(float forward, float lateral, int segments = 10);

    /**
     * @brief Laço: o projétil dá uma volta completa e retoma o rumo original.
     *
     * É a forma mais legível do conjunto — o jogador vê a bala girar e entende
     * imediatamente que ela vai voltar. Boa para telegrafar um ataque lento.
     *
     * @param radius Raio do laço.
     * @param exitDistance Quanto avança em linha reta depois de fechar a volta.
     * @param segments Pontos usados para desenhar o círculo.
     */
    std::vector<Vector2> Loop(float radius, float exitDistance = 400.f, int segments = 12);

    /**
     * @brief Ziguezague: avança alternando para os dois lados.
     *
     * Diferente do WobbleBehavior, aqui as quinas são exatas e previsíveis,
     * então o jogador consegue ler o padrão e se posicionar.
     *
     * @param step Avanço por perna do zigue-zague.
     * @param amplitude Deslocamento lateral de cada perna.
     * @param legs Quantidade de pernas.
     */
    std::vector<Vector2> Zigzag(float step, float amplitude, int legs = 5);

}
