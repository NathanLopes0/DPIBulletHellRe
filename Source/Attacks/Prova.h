//
// A repescagem do Andre: a prova no BOCA.
//

#pragma once

#include <array>
#include <string>

/**
 * CAMADA PURA.
 *
 * A REPESCAGEM DO ANDRE E A PROVA PRATICA, no BOCA: quatro problemas, A a D,
 * cada um com o balao da sua cor, e cada um e o que uma das fases ensinou -
 * a prova cai tudo:
 *
 *   A  if / else                      o fluxograma da fase 1, um comando
 *   B  o laco aninhado                o vetor do chao da fase 2
 *   C  baloes(dobro(a), b)            as funcoes da fase 3: a chamada de
 *                                     dentro, o retorno e a de fora
 *   D  o switch sem break             o fluxograma de novo
 *
 * Cada problema abre com o placar dizendo qual e ("Problema B: for"), e ao
 * fechar o juiz da o veredito - "YES" - e o balao do problema sobe do
 * professor, com a letra dele: e o que o BOCA da a quem resolve.
 *
 * Quem roda cada problema e o Andre, com o mesmo codigo da fase dele; aqui
 * ficam a ordem, o relogio e os textos. Coberto por test_prova.cpp.
 */
namespace Prova {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaProva = "StateFinal";
    constexpr const char* kGatilhoDoTrofeu = "trofeu";

    /// O que cada problema cobra: a fase de onde ele vem.
    enum class Tema { Condicional, Laco, Funcao };

    struct Problema {
        char letra;
        Tema tema;
        int vez;            ///< o primeiro comando dele, na ordem da fase de onde vem
        int quantos;        ///< quantos comandos (ou lacos) ele roda
        float inicio;       ///< segundos desde o comeco da repescagem
        float fim;
        const char* nome;   ///< o que o placar diz na abertura
    };

    /// A abertura de cada problema: o placar o anuncia, e so depois ele roda.
    constexpr float kAbertura = 0.6f;

    constexpr int kProblemas = 4;
    constexpr std::array<Problema, kProblemas> kProva = {{
        {'A', Tema::Condicional, 0, 1, 0.0f, 4.1f, "if / else"},
        {'B', Tema::Laco, 4, 1, 4.1f, 8.3f, "for aninhado"},
        {'C', Tema::Funcao, 2, 1, 8.3f, 12.2f, "funcoes"},
        {'D', Tema::Condicional, 3, 1, 12.2f, 16.3f, "switch"},
    }};

    /// @brief O problema em curso no instante 't' da repescagem, ou -1 depois
    /// do ultimo - o resto da repescagem e do veredito dele.
    int ProblemaEm(float t);

    /// @brief O placar da abertura do problema k: o veredito do anterior, se ha
    /// um, e o nome deste - "A: YES!  Problema B: for aninhado".
    std::string Abertura(int k);

    /// @brief O veredito do problema k, quando ele fecha: "Problema A: YES".
    std::string Veredito(int k);

    /// @brief A animacao do balao do problema k: a letra dele, na cor dele.
    std::string CaraDoTrofeu(int k);

    /// O balao do problema sobe devagar, do professor ate sair por cima.
    constexpr float kVelocidadeDoTrofeu = 90.0f;
}
