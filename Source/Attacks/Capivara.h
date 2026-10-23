//
// A capivara que atravessa o campo do Salles.
//

#pragma once

#include <cstdint>

/**
 * CAMADA PURA.
 *
 * O SALLES POE CAPIVARA EM TODO EXERCICIO DA DISCIPLINA, e de vez em quando
 * ela aparece no meio da fase: atravessa o campo de uma borda a outra, andando,
 * na altura do meio para baixo. Em toda fase dele, com chance kChance, uma vez,
 * num instante sorteado entre kDe e kAte segundos.
 *
 * ELA E UM ABRIGO (DescricaoDeProjetil::abrigo): todo projetil do professor que
 * encosta nela some - todo bicho se da bem com a capivara -, e ela nao machuca
 * ninguem. Quem anda embaixo dela fica protegido. Mas o tiro do aluno some
 * nela tambem: escondido, ele nao acerta o professor.
 *
 * Quem a solta e o Salles (o gatilho "capivara"); aqui fica o sorteio da
 * visita. Coberto por test_capivara.cpp.
 */
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

    /// @brief A visita de uma fase: se ela vem, quando, e de que lado entra.
    struct Visita {
        bool vem = false;
        float quando = 0.0f;
        bool daEsquerda = true;
    };

    /// @brief A visita, tirada do 'sorteio'. A mesma semente da a mesma visita.
    Visita Sortear(std::uint32_t sorteio);
}
