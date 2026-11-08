//
// O problema da parada: a repescagem do Vladimir.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

/// A repescagem do Vladimir e o problema da parada.
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

    /// As sementes: de 14 a 29 passos ate o 1, e nenhuma passa de 400 no caminho. Com 0,45s por
    /// passo, cada programa roda de 7 a 13s - um ou dois por fase.
    constexpr std::array<std::uint32_t, 12> kSementes = {7, 9, 15, 19, 25, 33, 43, 49, 51, 65, 67, 77};

    /// A faixa da altura do professor: no teto do granizo (x = 2^kBitsDoTeto)
    /// ele esta em kAlto, e no 1 em kBaixo.
    constexpr float kAlto = 110.0f;
    constexpr float kBaixo = 330.0f;
    constexpr float kBitsDoTeto = 8.0f;

    /// Um passo do programa. O 1 (e o 0) ficam onde estao: parou.
    std::uint32_t Proximo(std::uint32_t x);

    /// O programa parou?
    bool Parou(std::uint32_t x);

    /// A altura do professor com o programa valendo x: quanto maior x, mais alto - de kBaixo no 1 a
    /// kAlto no teto, em escala de log2.
    float Altura(std::uint32_t x);

    /// O placar do passo: "x = 41: 3x + 1", "x = 124: x / 2", "x = 1: parou".
    std::string Placar(std::uint32_t x);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// As sementes desta luta, na ordem em que rodam: as doze, embaralhadas, sem repetir nenhuma
    /// antes de rodar todas.
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
