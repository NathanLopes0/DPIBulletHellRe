// A repescagem do Andre: a prova no boca.

#pragma once

#include "../Regras/Variacao.h"

#include <array>
#include <string>

/// A if / else o fluxograma da fase 1, um comando B o laco aninhado o vetor do chao da fase 2 C
/// baloes(dobro(a), b) as funcoes da fase 3.
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

    /// Uma prova: os cinco problemas, na ordem em que caem. A de sempre e
    /// kProva; a de cada luta, Embaralhada.
    using Lista = std::array<Problema, kProblemas>;
    constexpr Lista kProva = {{
        {'A', Tema::Condicional, 0, 1, 0.0f, 4.0f, "if / else", kAceito},
        {'B', Tema::Laco, 4, 1, 4.0f, 8.2f, "for aninhado", kAceito},
        {'C', Tema::Funcao, 2, 1, 8.2f, 12.1f, "funcoes", kAceito},
        {'D', Tema::Condicional, 3, 1, 12.1f, 16.1f, "switch", kAceito},
        {'E', Tema::Laco, 5, 1, 16.1f, 19.4f, "for (i <= 12)", "NO - Runtime error"},
    }};

    /// O juiz aceitou o problema k? So o aceito ganha balao.
    bool Aceito(int k, const Lista& prova = kProva);

    /// O problema em curso no instante 't' da repescagem, ou -1 depois do ultimo - o resto da
    /// repescagem e do veredito dele.
    int ProblemaEm(float t, const Lista& prova = kProva);

    /// O placar da abertura do problema k: o veredito do anterior, se ha um, e o nome deste - "a:
    /// yes! Problema B: for aninhado".
    std::string Abertura(int k, const Lista& prova = kProva);

    /// O veredito do problema k, quando ele fecha: "Problema A: YES", "Problema E: NO - Runtime
    /// error".
    std::string Veredito(int k, const Lista& prova = kProva);

    /// A animacao do balao do problema k: a letra dele, na cor dele. Vazio para o que nao foi
    /// aceito - ele nao ganha balao.
    std::string CaraDoTrofeu(int k, const Lista& prova = kProva);

    /// O balao do problema sobe devagar, do professor ate sair por cima.
    constexpr float kVelocidadeDoTrofeu = 90.0f;

    // ---- O que muda de uma luta para a outra ----

    /// A prova desta luta: os quatro primeiros problemas em outra ordem, e o que capota sempre por
    /// ultimo - o "NO" fecha a prova.
    Lista Embaralhada(Variacao::Sorteio& s);
}
