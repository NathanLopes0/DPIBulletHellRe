//
// A fita da maquina de Turing do Vladimir: o castor mais ocupado de quatro
// estados, rodando de verdade.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Dificuldade.h"
#include "../Variacao.h"
#include "Fase.h"

/**
 * CAMADA PURA. Coberto por test_fita.cpp.
 *
 * A FASE 3 DO VLADIMIR RODA UMA MAQUINA DE TURING DE VERDADE, e nao uma
 * qualquer: o CASTOR MAIS OCUPADO de quatro estados (Rado, 1962; Brady, 1983)
 * - entre todas as maquinas de 4 estados e 2 simbolos que param, a que deixa
 * mais marcas na fita. Comecando da fita em branco, ela da 107 passos, vai e
 * volta numa faixa de 14 casas, escreve 46 vezes, apaga 33 e para com 13
 * marcas: Sigma(4) = 13. O problema de achar o castor de n estados e
 * incomputavel - e a ponte para a repescagem, o problema da parada.
 *
 * NA TELA: a fita e uma fileira de casas no meio do campo, e a cabeca (o
 * quadro desenhado em volta da casa lida) anda uma casa por passo, para onde
 * a tabela manda. O Vladimir a segue de cima, mais devagar. A marca e o "a":
 *
 *   escreve a no branco   a casa ganha um "a", parado na fileira
 *   apaga o a             o "a" vira "b" e sai perseguindo o aluno
 *   le o a e o mantem     a casa atira uma copia reta no aluno
 *   para                  um anel, e o placar diz Sigma(4) = 13
 *
 * O placar e a transicao de cada passo - "delta(q1, a) = (q2, branco, <-)" -,
 * e com ele o aluno sabe se a casa embaixo da cabeca vai ser escrita, apagada
 * ou relida.
 */
namespace Fita {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaFita = "StateThree";
    constexpr const char* kGatilhoDaEscrita = "escrever";   ///< a marca nova, na casa
    constexpr const char* kGatilhoDoApagar = "apagar";      ///< a marca apagada, caindo reta
    constexpr const char* kGatilhoDaReescrita = "reescrever";   ///< no Dificil: apagada, vira "b" e persegue
    constexpr const char* kGatilhoDoReler = "reler";        ///< a copia da marca relida
    constexpr const char* kGatilhoDaParada = "pare";        ///< o anel de quando a maquina para
    constexpr const char* kPostoDaFita = "fita";            ///< as marcas paradas na fileira

    // ---- A MAQUINA ---------------------------------------------------------------

    constexpr int kEstados = 4;
    constexpr int kParada = -1;   ///< o "proximo" de quem para

    /// O que a tabela manda: o simbolo a escrever (0 branco, 1 a marca), o
    /// sentido (+1 direita, -1 esquerda) e o proximo estado.
    struct Regra {
        int escreve = 0;
        int sentido = 1;
        int proximo = kParada;
    };

    /// A tabela do castor de 4 estados: [estado][simbolo lido].
    using Tabela = std::array<std::array<Regra, 2>, kEstados>;
    const Tabela& Castor();

    /// O que se sabe dele, e um teste confere rodando.
    constexpr int kPassos = 107;
    constexpr int kMarcas = 13;

    // ---- A FITA NA TELA ------------------------------------------------------------

    /// As 14 casas por onde ele anda, da esquerda para a direita, e onde a
    /// cabeca comeca: na casa 10 - a maquina anda mais para a esquerda.
    constexpr int kCasas = 14;
    constexpr int kOrigem = 10;
    constexpr float kEntreCasas = 66.0f;
    constexpr float kYDaFita = 379.0f;
    constexpr float kAlturaDaCabeca = 144.0f;

    float XDaCasa(int casa, float largura);

    /// @brief Onde o professor fica: inclinado kInclinacao do caminho entre o
    /// meio e a cabeca. EM CIMA DELA, quem segue o professor fica na mira de
    /// toda marca apagada: o piloto que persegue tomava 6 a 8 acertos na fase
    /// sozinha. PERTO DO MEIO (0,35), ele pairava sobre quem fica parado no
    /// centro, que o acertava a fase inteira e passava no Normal.
    constexpr float kInclinacao = 0.7f;

    float XDoProfessor(float xDaCabeca, float largura);


    // ---- RODANDO -------------------------------------------------------------------

    struct Estado {
        std::array<int, kCasas> fita{};
        int cabeca = kOrigem;
        int q = 0;
        int passos = 0;
        bool parou = false;
    };

    /// O comeco: a fita em branco, em q0, na origem - ou no espelho dela.
    Estado Inicio(bool espelho);

    /// Um passo: a casa lida, o que tinha e o que ficou, de que estado para
    /// qual, e o sentido NA TELA (no espelho, a esquerda da maquina e a direita
    /// da tela).
    struct Passo {
        int casa = 0;
        int leu = 0;
        int escreveu = 0;
        int de = 0;
        int para = 0;
        int sentido = 1;
    };
    Passo Avancar(Estado& e, bool espelho);

    /// Quantas marcas ha na fita.
    int Marcas(const Estado& e);

    // ---- O RELOGIO ------------------------------------------------------------------

    /// O primeiro passo logo depois de abrir, e um a cada kPasso: o ultimo,
    /// o que para, cai antes do fim da fase.
    constexpr float kPrimeiroPasso = 0.5f;
    constexpr float kPasso = 0.18f;

    // ---- O PLACAR E A LEGENDA -------------------------------------------------------

    /// @brief A transicao do passo, em UTF-8: "delta(q0, branco) = (q1, a, ->)";
    /// quem para vai para "pare".
    std::string Transicao(const Passo& p);

    /// @brief O placar: a transicao, e embaixo o passo e as marcas - ou, parada,
    /// "parou: 107 passos, Sigma(4) = 13".
    std::string Placar(const Estado& e, const Passo& p);


    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA ----------------------------------------

    /// A fita desta luta: a maquina como ela e, ou espelhada - a mesma
    /// maquina com esquerda e direita trocadas, que escreve a fita ao
    /// contrario, comecando na casa 3.
    struct Plano {
        bool espelho = false;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// O que o Dificil muda. No Normal a marca apagada cai reta da casa, e a
    /// relida so e lida; no Dificil a apagada e REESCRITA - vira "b" e sai
    /// perseguindo -, e a relida atira copias no aluno. Com o "b" e a copia ja
    /// no Normal, o piloto que persegue tomava 7 acertos na fase sozinha: ele
    /// fica embaixo do professor, quase parado, e cada acerto custa 6 pontos.
    struct Degrau {
        bool apagadaPersegue = false;
        bool relerAtira = false;
    };

    Degrau DegrauDo(Dificuldade::Nivel n);

    /// @brief A legenda da fase, em portugues: o que a maquina e e o que cada
    /// marca faz no modo da luta - ou, parada, o que isso quer dizer.
    std::string Legenda(bool parou, const Degrau& d);
}

