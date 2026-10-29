//
// O bubble sort da repescagem do Salles.
//

#pragma once

#include "../Gameplay/Dificuldade.h"
#include "../Gameplay/Variacao.h"

#include <cstdint>
#include <array>
#include <string>
#include <vector>

/**
 * CAMADA PURA.
 *
 * A REPESCAGEM DO SALLES E ORDENAR UM VETOR, na frente do aluno: kTamanho
 * elementos numerados lado a lado no alto do campo, e o professor rodando o
 * BUBBLE SORT neles. Cada passo compara um par vizinho - o par sobe, para ser
 * visto - e o placar diz o que acontece: "57 > 42: troca" ou "42 < 57: fica".
 * Numa troca, os dois pulam um por cima do outro e cada um atira no aluno.
 *
 * A BOLHA SOBE: numa passada, o maior elemento ainda fora do lugar e levado
 * troca a troca ate a fronteira, e la ele ASSENTA - "87 no lugar" -, desce para
 * a prateleira dos que ja estao ordenados e solta um anel. O professor fica
 * parado nessa fronteira, entre o que falta ordenar e o que ja esta pronto, e
 * da um passo para a esquerda a cada passada.
 *
 * ORDENADO: uma passada sem troca nenhuma ja prova que o vetor esta em ordem
 * ("nenhuma troca: ordenado!"). O vetor ordenado e ENTREGUE: cada elemento cai
 * tanto mais depressa quanto maior ele e, e a queda desenha uma rampa - a prova
 * de que esta ordenado, para quem esta embaixo. Depois, um vetor novo.
 *
 * O ALUNO REMOVE A TIRO: o elemento e derrubavel, e o vetor fecha o buraco
 * (Remover). Tirar um elemento do meio da passada RECOMECA a passada - sem
 * isso, o bubble sort assentaria no lugar errado (ha teste).
 *
 * Quem desenha, move e atira e o Salles; aqui ficam o vetor, o passo, os
 * lugares e os textos. Coberto por test_ordenacao.cpp.
 */
namespace Ordenacao {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaOrdenacao = "StateFinal";
    constexpr const char* kPostoDoVetor = "vetor";
    constexpr const char* kGatilhoDoElemento = "elemento";
    constexpr const char* kGatilhoDaTroca = "trocar";
    constexpr const char* kGatilhoDoLugar = "assentar";

    /// Quantos elementos tem um vetor, e de quanto a quanto vao os valores (as
    /// caras sao as chaves da tabela hash, de K0 a K99).
    constexpr int kTamanho = 8;
    constexpr int kMenorValor = 1;
    constexpr int kMaiorValor = 99;

    /// O relogio: o primeiro vetor, do vetor novo a primeira comparacao, uma
    /// comparacao a cada kPasso, a pausa de quem assenta, e da entrega ao vetor
    /// seguinte.
    constexpr float kPrimeiro = 0.6f;
    constexpr float kAteComparar = 1.0f;
    constexpr float kPasso = 0.32f;
    constexpr float kNoLugar = 0.4f;
    constexpr float kDepoisDeOrdenar = 1.6f;

    /// Onde fica o vetor: a fileira, a distancia entre dois elementos, quanto o
    /// par comparado sobe, quanto desce quem ja esta no lugar, e a altura do
    /// professor em cima da fronteira.
    constexpr float kYDoVetor = 230.0f;
    constexpr float kEntre = 120.0f;
    constexpr float kErguido = 28.0f;
    constexpr float kPrateleira = 46.0f;
    constexpr float kYDoProfessor = 95.0f;

    /// O pulo da troca: quanto o que vai para a direita sobe (e o que vai para a
    /// esquerda desce), e por quanto tempo.
    constexpr float kArco = 42.0f;
    constexpr float kDuracaoDoArco = 0.2f;

    /// A queda do vetor ordenado: kQuedaMinima mais kQuedaPorValor por unidade
    /// do valor, entao o maior cai mais depressa e a fileira vira uma rampa.
    constexpr float kQuedaMinima = 120.0f;
    constexpr float kQuedaPorValor = 3.2f;

    /// @brief O x do lugar k num vetor de n, centrado em 'largura'. k pode ser
    /// fracionario: k + 0,5 e a fronteira entre o lugar k e o seguinte.
    float X(float k, int n, float largura);

    /// @brief A velocidade da queda do elemento de valor v.
    float VelocidadeDaQueda(int valor);

    /// @brief Um vetor de n valores diferentes, de kMenorValor a kMaiorValor,
    /// fora de ordem (um vetor que ja nascesse ordenado nao teria aula).
    std::vector<int> Sortear(std::uint32_t sorteio, int n);

    /// O bubble sort em curso. v[fim + 1 ...] ja esta no lugar; a passada
    /// compara v[i] com v[i + 1], de i = 0 ate o par que termina em v[fim].
    struct Bolha {
        std::vector<int> v;
        int i = 0;
        int fim = 0;
        bool trocou = false;     ///< houve troca nesta passada
        bool ordenado = false;
    };

    /// @brief O bubble sort do vetor v, antes da primeira comparacao.
    Bolha Comecar(std::vector<int> v);

    enum class Tipo { Fica, Troca, NoLugar, Ordenado };

    struct Passo {
        Tipo tipo = Tipo::Fica;
        int i = 0;          ///< Fica/Troca: o par e v[i], v[i + 1]; NoLugar: quem assentou
        int a = 0;          ///< o valor da esquerda antes do passo (NoLugar: o que assentou)
        int b = 0;          ///< o valor da direita antes do passo
        bool semTroca = false;   ///< Ordenado: a passada inteira nao trocou nada
    };

    /// @brief O proximo passo do bubble sort, e o vetor ja depois dele. Uma
    /// comparacao por passo; no fim de cada passada, quem chegou a fronteira
    /// assenta (NoLugar), e uma passada sem troca termina tudo (Ordenado).
    /// Depois de Ordenado, devolve Ordenado para sempre.
    Passo Avancar(Bolha& b);

    /// @brief Tira o elemento k do vetor. Dentro da parte que falta ordenar, a
    /// passada recomeca do zero; na parte ja ordenada, nada muda.
    void Remover(Bolha& b, int k);

    /// @brief Os placares: "bubble sort, n = 8", "57 > 42: troca", "42 < 57:
    /// fica", "87 no lugar", "ordenado!" e "nenhuma troca: ordenado!".
    std::string PlacarDoComeco(int n);
    std::string Placar(const Passo& p);

    /// @brief A cara do elemento de valor v: a chave "K42" da folha da hash.
    std::string CaraDoElemento(int valor);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// COMO O VETOR CHEGA: em qualquer ordem, QUASE ORDENADO (dois pares
    /// trocados - o bubble sort acaba cedo, na passada sem troca) ou INVERTIDO
    /// (o pior caso: toda comparacao e uma troca). O placar diz qual.
    enum class Entrada { Aleatoria, QuaseOrdenada, Invertida };
    std::vector<int> Sortear(std::uint32_t sorteio, int n, Entrada e);
    std::string PlacarDoComeco(int n, Entrada e);

    /// A repescagem desta luta: os vetores chegam um de cada jeito, numa
    /// ordem sorteada (e depois a ordem recomeca), com 7 ou 8 elementos.
    struct Plano {
        std::array<Entrada, 3> entradas{Entrada::Aleatoria, Entrada::QuaseOrdenada, Entrada::Invertida};
        int tamanho = kTamanho;
    };
    Plano PlanoDeSempre();
    Plano Sortear(Variacao::Sorteio& s);
    /// A entrada do vetor de numero k da fase.
    Entrada EntradaDaVez(const Plano& p, int k);

    /// O que o Dificil muda: uma comparacao a cada 0,27 s, as capivaras da
    /// troca 25% mais rapidas, e quem assenta solta um anel de dez.
    struct Degrau {
        float passo = kPasso;
        float rapidez = 1.0f;
        int anelDoLugar = 8;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
