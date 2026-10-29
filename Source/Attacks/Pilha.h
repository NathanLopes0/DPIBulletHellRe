//
// A pilha da fase 1 do Salles.
//

#pragma once

#include "../Gameplay/Dificuldade.h"
#include "../Gameplay/Variacao.h"

#include <cstdint>
#include <string>

/**
 * CAMADA PURA.
 *
 * A FASE 1 DO SALLES E UMA PILHA, pendurada nele. Cada push poe um bloco
 * numerado na ponta de baixo da pilha - o TOPO, o ultimo a entrar -, e cada pop
 * tira o topo e o atira no aluno. LIFO: o ultimo que entra e o primeiro que
 * sai, e a tela mostra isso, porque o topo e sempre o bloco de baixo.
 *
 * O ALUNO TAMBEM DA POP, A TIRO. O bloco e derrubavel, e o tiro que sobe do
 * aluno acerta primeiro o bloco de baixo - o topo. So da para tirar o topo, e
 * nao ha como furar a pilha pelo meio: e a pilha pela geometria. Enquanto ela
 * esta pendurada, ela e a armadura do professor.
 *
 * STACK OVERFLOW: um push na pilha cheia derruba a pilha inteira, que cai em
 * cima de quem estiver embaixo.
 *
 * O POP E O VALOR: o bloco tirado voa para o aluno e estoura em tantas lascas
 * quanto o numero dele (kEstouraEm).
 *
 * O PLACAR E A OPERACAO: "push(7)", "pop() -> 7", "stack overflow!". Quem
 * pendura, solta e atira e o Salles; aqui ficam o programa, o lugar de cada
 * bloco e o texto. Coberto por test_pilha.cpp.
 */
namespace Pilha {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaPilha = "StateOne";
    constexpr const char* kPostoDaPilha = "pilha";
    constexpr const char* kGatilhoDoPush = "push";
    constexpr const char* kGatilhoDoEstouro = "estourar";

    /// O POP E O VALOR DO TOPO: o bloco tirado voa para o aluno e, depois de
    /// kEstouraEm, estoura num leque de tantas lascas quanto o numero dele,
    /// mirado no aluno - o placar diz "pop() -> 7", e o aluno sabe que vem um
    /// leque de sete. Um 0 nao estoura.
    constexpr float kEstouraEm = 0.45f;
    constexpr float kVelocidadeDoPop = 340.0f;

    /// Quantos blocos cabem: o sexto push derruba a pilha.
    constexpr int kCapacidade = 5;

    /// O relogio do programa: a primeira operacao, e uma a cada kPasso.
    constexpr float kPrimeira = 1.2f;
    constexpr float kPasso = 0.7f;

    /// A chance de um push quando ha escolha (pilha nem vazia nem cheia). Mais
    /// push que pop: a pilha cresce, e o overflow chega - em quase toda fase
    /// (ha teste).
    constexpr float kChanceDePush = 0.65f;

    /// Onde fica o bloco k, contado de cima (0 e o da base, colado no
    /// professor): kPrimeiroBloco abaixo dele, e um a cada kEntreBlocos.
    constexpr float kPrimeiroBloco = 78.0f;
    constexpr float kEntreBlocos = 46.0f;

    enum class Tipo { Push, Pop, Overflow };

    struct Operacao {
        Tipo tipo = Tipo::Push;
        int valor = 0;   ///< o numero do bloco que entra (push)
    };

    /// @brief Onde fica o bloco k em relacao ao professor (so o y; o x e o dele).
    float AbaixoDoProfessor(int k);

    /// @brief O placar da operacao. 'topo' e o valor que sai num pop.
    std::string Placar(const Operacao& op, int topo);

    /// @brief A animacao do bloco de valor v: "N0" a "N9".
    std::string CaraDoBloco(int valor);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// A pilha desta luta: quantos blocos cabem (4 a 6) e a chance do push (de
    /// 0,55 a 0,65 na de quatro, 0,05 a mais por bloco - o overflow continua
    /// vindo em quase toda fase). A regra e a mesma: cheia, o push derruba.
    struct Plano {
        int capacidade = kCapacidade;
        float chanceDePush = kChanceDePush;
    };
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kMenorCapacidade = 4;
    constexpr int kMaiorCapacidade = 6;

    /// @brief A proxima operacao, com a pilha de 'altura' blocos. Vazia, push;
    /// cheia, o push que derruba; no meio, push com a chance do plano. O valor
    /// do push e um digito, de 0 a 9.
    Operacao Proxima(int altura, std::uint32_t sorteio, const Plano& p = Plano{});

    /// O que o Dificil muda: uma operacao a cada 0,66 s, e nao 0,7, e o pop,
    /// as lascas e a pilha derrubada andam 10% mais depressa. E POUCO DE
    /// PROPOSITO: com 0,62 s e 15%, o piloto que desvia saia da fase 1 com 17
    /// a 45 (no Normal, 61 a 66); com uma lasca a mais por pop, igual.
    struct Degrau {
        float passo = kPasso;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
