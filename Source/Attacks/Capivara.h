//
// A capivara que atravessa o campo do Salles.
//

#pragma once

#include <cstdint>

/// O Salles poe capivara em todo exercicio da disciplina, e de vez em quando ela aparece no meio da
/// fase.
namespace Capivara {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kGatilho = "capivara";
    constexpr const char* kAnimacaoDireita = "AndaDireita";
    constexpr const char* kAnimacaoEsquerda = "AndaEsquerda";

    /// A chance de ela aparecer numa fase, e quando.
    constexpr float kChance = 0.6f;
    constexpr float kDe = 4.0f;
    constexpr float kAte = 12.0f;     // a 170 px/s a travessia leva 7,6 s: acaba antes da fase (ha teste)

    /// Por onde ela anda: a altura, e de quanto fora da tela ela entra.
    constexpr float kAltura = 470.0f;
    constexpr float kForaDaTela = 50.0f;

    /// O que o placar diz quando ela aparece, e por quanto tempo.
    constexpr const char* kAnuncio = "uma capivara apareceu!";
    constexpr float kAnuncioNaTela = 2.2f;

    /// A visita de uma fase: se ela vem, quando, e de que lado entra.
    struct Visita {
        bool vem = false;
        float quando = 0.0f;
        bool daEsquerda = true;
    };

    /// A visita, tirada do 'sorteio'. A mesma semente da a mesma visita.
    Visita Sortear(std::uint32_t sorteio);
}
