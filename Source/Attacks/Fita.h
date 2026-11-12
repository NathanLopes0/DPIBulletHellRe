//
// A fita da maquina de Turing do Vladimir: o castor mais ocupado de quatro
// estados, rodando de verdade.
//

#pragma once

#include <array>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"
#include "Fase.h"

/// A fase 3 do Vladimir roda uma maquina de Turing de verdade, e nao uma qualquer: o castor mais
/// ocupado de quatro estados (Rado, 1962; Brady, 1983).
namespace Fita {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaFita = "StateThree";
    constexpr const char* kGatilhoDaEscrita = "escrever";   ///< a marca nova, na casa
    constexpr const char* kGatilhoDoApagar = "apagar";      ///< a marca apagada, caindo reta
    constexpr const char* kGatilhoDaReescrita = "reescrever";   ///< no Dificil: apagada, vira "b" e persegue
    constexpr const char* kGatilhoDoReler = "reler";        ///< a copia da marca relida
    constexpr const char* kGatilhoDaParada = "pare";        ///< o anel de quando a maquina para
    constexpr const char* kPostoDaFita = "fita";            ///< as marcas paradas na fileira

    // ---- A maquina ----

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

    // ---- A fita na tela ----

    /// As 14 casas por onde ele anda, da esquerda para a direita, e onde a
    /// cabeca comeca: na casa 10 - a maquina anda mais para a esquerda.
    constexpr int kCasas = 14;
    constexpr int kOrigem = 10;
    constexpr float kEntreCasas = 66.0f;
    constexpr float kYDaFita = 379.0f;
    constexpr float kAlturaDaCabeca = 144.0f;

    float XDaCasa(int casa, float largura);

    /// Onde o professor fica: inclinado kInclinacao do caminho entre o meio e a cabeca.
    constexpr float kInclinacao = 0.7f;

    float XDoProfessor(float xDaCabeca, float largura);

    // ---- Rodando ----

    struct Estado {
        std::array<int, kCasas> fita{};
        int cabeca = kOrigem;
        int q = 0;
        int passos = 0;
        bool parou = false;
    };

    /// O comeco: a fita em branco, em q0, na origem - ou no espelho dela.
    Estado Inicio(bool espelho);

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

    // ---- O relogio ----

    /// O primeiro passo logo depois de abrir, e um a cada kPasso: o ultimo,
    /// o que para, cai antes do fim da fase.
    constexpr float kPrimeiroPasso = 0.5f;
    constexpr float kPasso = 0.18f;

    // ---- O placar e a legenda ----

    /// A transicao do passo, em UTF-8: "delta(q0, branco) = (q1, a, ->)"; quem para vai para
    /// "pare".
    std::string Transicao(const Passo& p);

    /// O placar: a transicao, e embaixo o passo e as marcas - ou, parada, "parou: 107 passos,
    /// Sigma(4) = 13".
    std::string Placar(const Estado& e, const Passo& p);

    // ---- O que muda de uma luta para a outra ----

    /// A fita desta luta: a maquina como ela e, ou espelhada.
    struct Plano {
        bool espelho = false;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// O que o Dificil muda.
    struct Degrau {
        bool apagadaPersegue = false;
        bool relerAtira = false;
    };

    Degrau DegrauDo(Dificuldade::Nivel n);

}

