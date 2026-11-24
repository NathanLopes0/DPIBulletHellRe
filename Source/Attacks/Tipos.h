//
// O tipo do aluno no Daniel: fogo, agua ou planta, escolhido no comeco e para a luta inteira.
//

#pragma once

#include <array>
#include <vector>

#include "../Motor/Math.h"
#include "../Regras/Variacao.h"

/// No comeco da batalha do Daniel, quem joga escolhe um monstrinho - e o tipo dele e o seu ate o fim
/// (nao muda). O triangulo vale nos dois sentidos, a luta inteira:
///   - na defesa, contra todo tiro com tipo: o do seu tipo passa por voce (BossProjectile::Classe);
///     o do tipo que voce vence, voce absorve - e e super efetivo: vale um pouco de nota; o do tipo
///     que vence o seu machuca;
///   - no ataque, contra todo alvo com tipo (as amostras, os nucleos): o seu tiro tira o dobro do
///     tipo que voce vence, e a metade do tipo que vence o seu (FatorDoDano).
/// Fogo vence planta, planta vence agua, agua vence fogo. O que nao tem tipo vale o normal.
namespace Tipos {

    /// Os tipos, na ordem das caras dos dados: fogo, agua e planta.
    constexpr int kTipos = 3;
    constexpr int kFogo = 0;
    constexpr int kAgua = 1;
    constexpr int kPlanta = 2;
    /// As caras dos tres tipos nas folhas dos monstrinhos.
    constexpr std::array<const char*, kTipos> kCaras = {"Fogo", "Agua", "Planta"};

    /// Se o tipo 'a' vence o 'b'.
    bool Vence(int a, int b);
    /// O tipo que vence 't'.
    int QueVence(int t);
    /// O tipo que 't' vence.
    int Vencido(int t);

    enum class Efeito { Passa, Absorve, Machuca };
    /// O que o tiro do tipo 'doTiro' (-1: sem tipo) faz no aluno do tipo 'doAluno' (-1: sem tipo).
    Efeito Contra(int doAluno, int doTiro);

    /// O tiro do aluno do tipo 'doTiro' num alvo do tipo 'doAlvo' (-1: sem tipo): com vantagem tira
    /// kVantagem vezes o normal; com desvantagem, kDesvantagem. O alvo mostra qual foi.
    constexpr float kVantagem = 2.0f;
    constexpr float kDesvantagem = 0.5f;
    enum class Golpe { Normal, Vantagem, Resistencia };
    Golpe GolpeContra(int doTiro, int doAlvo);
    float FatorDoDano(int doTiro, int doAlvo);

    /// "fogo", "agua", "planta" (com acento).
    const char* Nome(int t);

    /// O que cada tiro absorvido vale.
    constexpr float kPremioDoSuperEfetivo = 0.15f;

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

    // ---- Os efeitinhos de cada elemento ----
    //
    // Fogo solta fagulhas que sobem, agua solta gotas que caem, planta solta folhas que giram e
    // descem devagar: a forma e o movimento dizem o elemento, alem da cor.

    struct Particula {
        Vector2 onde;
        Vector2 velocidade;
        float idade = 0.0f;
        float vida = 0.6f;
        int tipo = 0;
        float giro = 0.0f;       ///< a folha gira
        float tamanho = 3.0f;
    };
    /// 'quantas' particulas do 'tipo' saindo de 'onde', com 'forca' (px/s) de espalhamento.
    std::vector<Particula> Explodir(int tipo, Vector2 onde, int quantas, float forca, Variacao::Sorteio& s);
    /// Anda 'dt': a fagulha sobe e freia, a gota cai, a folha balanca e desce devagar.
    void Andar(Particula& p, float dt);
    [[nodiscard]] inline bool Viva(const Particula& p) { return p.idade < p.vida; }
    /// Nunca mais que isto no campo.
    constexpr size_t kMaisParticulas = 420;
}
