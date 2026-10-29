//
// A caixa do HTML - a geometria de um retangulo que cresce sem perder a forma.
//

#pragma once

#include <vector>

#include "../Core/Math.h"

/**
 * CAMADA PURA (mesma disciplina de Fita, Tabela e PathAim).
 *
 * A fase 1 do Lucas e o HTML: caixas que nascem em volta dele e crescem, uma
 * dentro da outra, como divs aninhadas. Quem cria os projeteis e CaixaAttack;
 * aqui ficam as duas contas que podem errar - onde cada projetil nasce no
 * contorno, e com que velocidade ele precisa sair para a caixa continuar caixa.
 */
namespace Caixa {

    /**
     * @brief 'n' pontos igualmente espacados pelo CONTORNO de um retangulo
     * centrado na origem, em sentido horario a partir do canto de cima a
     * esquerda.
     *
     * IGUALMENTE ESPACADOS PELO COMPRIMENTO DO CONTORNO, e nao pelo angulo. Pelo
     * angulo - como num anel - os pontos se amontoariam nos lados compridos e
     * abririam buracos nos curtos, e a caixa teria uma porta onde ninguem pediu.
     *
     * O primeiro ponto e sempre o canto de cima a esquerda, entao a caixa tem
     * pelo menos um canto marcado. Sem n positivo, ou com um lado nao positivo,
     * nao ha ponto.
     */
    std::vector<Vector2> PontosDoContorno(int n, float meiaLargura, float meiaAltura);

    /**
     * @brief A velocidade de um ponto do contorno para a caixa crescer SEM
     * DEFORMAR.
     *
     * Cada ponto sai na direcao em que esta do centro, com modulo proporcional
     * a distancia: e uma homotetia, e a caixa fica semelhante a si mesma o tempo
     * todo. 'velocidade' e a da meia-altura - o meio do lado de cima e o do lado
     * de baixo andam exatamente a ela; os cantos, mais rapido.
     *
     * Por isso os vizinhos se afastam enquanto a caixa cresce: a parede que nasce
     * fechada em volta do chefe chega ao aluno com espaco entre os ladrilhos.
     */
    Vector2 Velocidade(const Vector2& ponto, float meiaAlturaInicial, float velocidade);
}
