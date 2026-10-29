//
// O problema da parada: a repescagem do Vladimir.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../Dificuldade.h"
#include "../Variacao.h"

/**
 * CAMADA PURA.
 *
 * A REPESCAGEM DO VLADIMIR E O PROBLEMA DA PARADA. Acima da maquina de Turing
 * so ha o que nenhuma maquina decide - e o exemplo mais simples de programa
 * que ninguem sabe dizer se sempre para e este:
 *
 *     while (x != 1)
 *         x = (x par) ? x / 2 : 3x + 1;
 *
 * A conjectura de Collatz diz que ele para para todo x; ninguem provou. Os
 * valores sobem e descem antes de cair no 1 - os "numeros granizo" -, e e o
 * gesto da fase: o professor RODA o programa, um passo a cada kPasso, e a
 * altura dele e o tamanho de x (Altura): sobe quando x cresce, cai quando
 * diminui, e chega ao 1 la embaixo. Cada passo atira: x par, um leque de "a"
 * retos (a metade); x impar, "b" que perseguem (o 3x + 1). No 1, o programa
 * PAROU - e estoura num anel. Depois roda outro x.
 *
 * O PLACAR E O PASSO DE AGORA: "x = 41: 3x + 1". Com ele o aluno sabe o que
 * vem - e para onde o professor vai.
 *
 * Quem voa e atira e o Vladimir; aqui ficam o programa, as sementes, a altura
 * e o texto. Coberto por test_parada.cpp.
 */
namespace Parada {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDaParada = "StateFinal";
    constexpr const char* kGatilhoDaMetade = "metade";
    constexpr const char* kGatilhoDoTriplo = "triplo";
    constexpr const char* kGatilhoDeParar = "parou";

    /// Quanto dura um passo do programa, quanto ele fica parado no 1, e quando
    /// o primeiro programa comeca.
    constexpr float kPasso = 0.45f;
    constexpr float kParado = 1.3f;
    constexpr float kPrimeiro = 1.4f;

    /// AS SEMENTES: de 14 a 29 passos ate o 1, e nenhuma passa de 400 no
    /// caminho. Com 0,45s por passo, cada programa roda de 7 a 13s - um ou dois
    /// por fase. O 27, o mais famoso, leva 111 passos e passa de 9000: nao cabe.
    constexpr std::array<std::uint32_t, 12> kSementes = {7, 9, 15, 19, 25, 33, 43, 49, 51, 65, 67, 77};

    /// A faixa da altura do professor: no teto do granizo (x = 2^kBitsDoTeto)
    /// ele esta em kAlto, e no 1 em kBaixo.
    constexpr float kAlto = 110.0f;
    constexpr float kBaixo = 330.0f;
    constexpr float kBitsDoTeto = 8.0f;

    /// @brief Um passo do programa. O 1 (e o 0) ficam onde estao: parou.
    std::uint32_t Proximo(std::uint32_t x);

    /// @brief O programa parou?
    bool Parou(std::uint32_t x);

    /// @brief A altura do professor com o programa valendo x: quanto maior x,
    /// mais alto - de kBaixo no 1 a kAlto no teto, em escala de log2.
    float Altura(std::uint32_t x);

    /// @brief O placar do passo: "x = 41: 3x + 1", "x = 124: x / 2", "x = 1: parou".
    std::string Placar(std::uint32_t x);

    // ---- O QUE MUDA DE UMA LUTA PARA A OUTRA, E O DEGRAU -------------------

    /// As sementes desta luta, na ordem em que rodam: as doze, embaralhadas,
    /// sem repetir nenhuma antes de rodar todas. E o lado para onde o professor
    /// comeca a varrer.
    struct Plano {
        std::vector<std::uint32_t> sementes;
        float sentido = 1.0f;
    };
    Plano Sortear(Variacao::Sorteio& s);

    /// A semente do programa numero 'vez' da luta.
    std::uint32_t SementeDaVez(const Plano& p, int vez);

    /// O que o Dificil muda: um passo a cada 0,38 s (e nao 0,45), e o que ele
    /// atira sai 15% mais rapido.
    struct Degrau {
        float passo = kPasso;
        float rapidez = 1.0f;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
