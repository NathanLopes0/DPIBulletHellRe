//
// O tipo do aluno no Daniel: fogo, agua ou planta, escolhido no comeco e para a luta inteira.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"

/// No comeco da batalha do Daniel, quem joga escolhe um monstrinho - e o tipo dele e o seu ate o fim
/// (nao muda). Todo tiro com tipo segue o triangulo, contra o tipo de cada aluno:
///   - o do seu tipo passa por voce (e da sua especie: BossProjectile::Classe);
///   - o do tipo que voce vence, voce absorve - e e super efetivo: vale um pouco de nota;
///   - o do tipo que vence o seu machuca.
/// Fogo vence planta, planta vence agua, agua vence fogo. O tiro sem tipo machuca todo mundo.
namespace Tipos {

    /// Os tipos, na ordem das caras dos dados (Preparacao::kCaras): fogo, agua e planta.
    constexpr int kTipos = 3;
    constexpr int kFogo = 0;
    constexpr int kAgua = 1;
    constexpr int kPlanta = 2;

    /// Se o tipo 'a' vence o 'b'.
    bool Vence(int a, int b);
    /// O tipo que vence 't'.
    int QueVence(int t);
    /// O tipo que 't' vence.
    int Vencido(int t);

    enum class Efeito { Passa, Absorve, Machuca };
    /// O que o tiro do tipo 'doTiro' (-1: sem tipo) faz no aluno do tipo 'doAluno' (-1: sem tipo).
    Efeito Contra(int doAluno, int doTiro);

    /// "fogo", "agua", "planta" (com acento).
    const char* Nome(int t);

    /// O que cada tiro absorvido vale.
    constexpr float kPremioDoSuperEfetivo = 0.3f;

    /// A escolha: os monstrinhos ficam no campo por kTempoDaEscolha segundos; quem nao escolheu fica
    /// com o mais perto. Escolher pede ficar em cima de um por kParaEscolher segundos - quem so passa
    /// por cima, andando, nao escolhe nada.
    constexpr float kTempoDaEscolha = 5.0f;
    constexpr float kToqueDaEscolha = 46.0f;
    constexpr float kParaEscolher = 0.6f;
    /// O indice do lugar mais perto de 'p' (-1 sem lugar).
    int MaisPerto(const std::vector<Vector2>& lugares, Vector2 p);

    /// Os tres lugares da escolha, num campo de 'largura' por 'altura' com o canto em (x, y): no alto,
    /// num arco - os dos lados um pouco mais baixos -, longe do chao, onde o aluno nasce. Quem anda
    /// para cima sem pensar nao cai em nenhum. A ordem dos tipos neles e sorteada a cada luta.
    std::vector<Vector2> LugaresDaEscolha(float x, float y, float largura, float altura);

    /// A roleta dos tipos, no sorteio: tres bracos girando em volta do professor, um de cada tipo. O
    /// angulo (graus) do braco 'k' no instante 't', girando para o lado 'sentido' (+1 ou -1).
    float AnguloDoBraco(int k, float t, float sentido);
    constexpr float kGiroDaRoleta = 50.0f;   ///< graus por segundo
}
