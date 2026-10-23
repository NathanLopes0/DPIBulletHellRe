//
// A repescagem do Andre: a prova no BOCA.
//

#pragma once

#include <array>
#include <string>

/**
 * CAMADA PURA.
 *
 * A REPESCAGEM DO ANDRE E A PROVA PRATICA, no BOCA: cinco problemas, A a E,
 * cada um com o balao da sua cor, e cada um e o que uma das fases ensinou -
 * a prova cai tudo:
 *
 *   A  if / else                      o fluxograma da fase 1, um comando
 *   B  o laco aninhado                o vetor do chao da fase 2
 *   C  baloes(dobro(a), b)            as funcoes da fase 3: a chamada de
 *                                     dentro, o retorno e a de fora
 *   D  o switch sem break             o fluxograma de novo
 *   E  for (i = 0; i <= 12; ...)      o laco que passa do fim do vetor
 *
 * Cada problema abre com o placar dizendo qual e ("Problema B: for"), e ao
 * fechar o juiz da o veredito. "YES" e o balao do problema sobe do professor,
 * com a letra dele: e o que o BOCA da a quem resolve. O ultimo nao passa - o
 * programa capota, e o veredito e "NO - Runtime error": sem balao, como no
 * BOCA de verdade.
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
        const char* nome;       ///< o que o placar diz na abertura
        const char* veredito;   ///< o que o juiz diz quando ele fecha
    };

    constexpr const char* kAceito = "YES";

    /// A abertura de cada problema: o placar o anuncia, e so depois ele roda.
    constexpr float kAbertura = 0.6f;

    constexpr int kProblemas = 5;
    constexpr std::array<Problema, kProblemas> kProva = {{
        {'A', Tema::Condicional, 0, 1, 0.0f, 4.0f, "if / else", kAceito},
        {'B', Tema::Laco, 4, 1, 4.0f, 8.2f, "for aninhado", kAceito},
        {'C', Tema::Funcao, 2, 1, 8.2f, 12.1f, "funcoes", kAceito},
        {'D', Tema::Condicional, 3, 1, 12.1f, 16.1f, "switch", kAceito},
        {'E', Tema::Laco, 5, 1, 16.1f, 19.4f, "for (i <= 12)", "NO - Runtime error"},
    }};

    /// @brief O juiz aceitou o problema k? So o aceito ganha balao.
    bool Aceito(int k);

    /// @brief O problema em curso no instante 't' da repescagem, ou -1 depois
    /// do ultimo - o resto da repescagem e do veredito dele.
    int ProblemaEm(float t);

    /// @brief O placar da abertura do problema k: o veredito do anterior, se ha
    /// um, e o nome deste - "A: YES!  Problema B: for aninhado".
    std::string Abertura(int k);

    /// @brief O veredito do problema k, quando ele fecha: "Problema A: YES",
    /// "Problema E: NO - Runtime error".
    std::string Veredito(int k);

    /// @brief A animacao do balao do problema k: a letra dele, na cor dele.
    /// Vazio para o que nao foi aceito - ele nao ganha balao.
    std::string CaraDoTrofeu(int k);

    /// O balao do problema sobe devagar, do professor ate sair por cima.
    constexpr float kVelocidadeDoTrofeu = 90.0f;
}
