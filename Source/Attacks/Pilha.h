//
// A pilha da fase 1 do Salles.
//

#pragma once

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <cstdint>
#include <string>

/// A fase 1 do Salles e uma pilha, pendurada nele.
namespace Pilha {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaPilha = "StateOne";
    constexpr const char* kPostoDaPilha = "pilha";
    constexpr const char* kGatilhoDoPush = "push";
    constexpr const char* kGatilhoDoEstouro = "estourar";

    /// Um 0 nao estoura.
    constexpr float kEstouraEm = 0.45f;
    constexpr float kVelocidadeDoPop = 340.0f;

    /// Quantos blocos cabem: o sexto push derruba a pilha.
    constexpr int kCapacidade = 5;

    /// O relogio do programa: a primeira operacao, e uma a cada kPasso.
    constexpr float kPrimeira = 1.2f;
    constexpr float kPasso = 0.7f;

    /// A chance de um push quando ha escolha (pilha nem vazia nem cheia).
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

    /// Onde fica o bloco k em relacao ao professor, no y.
    float AbaixoDoProfessor(int k);

    /// A pilha fica pendurada ao lado do professor, e nao embaixo dele: kAoLado para o lado que da
    /// para o meio do campo. Assim ela nao o cobre por baixo - quem vai atras dele acerta -, e
    /// continua sendo alvo e arma.
    constexpr float kAoLado = 78.0f;

    /// O lado da pilha (+1 a direita, -1 a esquerda) com o professor na coluna 'coluna' de
    /// 'colunas': a da esquerda pendura a direita, a da direita a esquerda, e as do meio sorteiam.
    int LadoDaPilha(int coluna, int colunas, std::uint32_t sorteio);

    /// O placar da operacao. 'topo' e o valor que sai num pop.
    std::string Placar(const Operacao& op, int topo);

    /// A animacao do bloco de valor v: "N0" a "N9".
    std::string CaraDoBloco(int valor);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// A pilha desta luta: quantos blocos cabem (4 a 6) e a chance do push (de 0,55 a 0,65 na de
    /// quatro, 0,05 a mais por bloco.
    struct Plano {
        int capacidade = kCapacidade;
        float chanceDePush = kChanceDePush;
    };
    Plano Sortear(Variacao::Sorteio& s);
    constexpr int kMenorCapacidade = 4;
    constexpr int kMaiorCapacidade = 6;

    /// A proxima operacao, com a pilha de 'altura' blocos. Vazia, push; cheia, o push que derruba;
    /// no meio, push com a chance do plano.
    Operacao Proxima(int altura, std::uint32_t sorteio, const Plano& p = Plano{});

    /// O que o Dificil muda: uma operacao a cada 0,66 s, e nao 0,7, e o pop, as lascas e a pilha
    /// derrubada andam 10% mais depressa.
    struct Degrau {
        float passo = kPasso;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
