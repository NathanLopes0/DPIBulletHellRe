//
// O clock da fase 2 do Ricardo.
//

#pragma once

#include <array>

#include "../Regras/Dificuldade.h"
#include "../Regras/Variacao.h"

#include <cstdint>
#include <optional>
#include <string>

/// A fase 2 do Ricardo e Organizacao de Computadores, e o coracao de um computador e o clock.
namespace Relogio {

    /// Os nomes que o codigo e os dados dividem. Um teste confere cada um.
    constexpr const char* kFaseDoRelogio = "StateTwo";
    constexpr const char* kGatilhoDoContador = "contador";
    constexpr const char* kPostoDosRegistradores = "registradores";

    /// Quanto um registrador anda a cada borda de subida: um bit e um pouco.
    constexpr float kPasso = 72.0f;

    /// A frequencia escrita no placar: o numero logo antes de "Hz" ("clock 2 Hz", "overclock 8 Hz";
    /// a virgula vale como ponto).
    std::optional<float> LerFrequencia(const std::string& placar);

    /// O instante zero nao e borda.
    bool Subiu(float antes, float agora, float frequencia);

    /// O clock esta no nivel alto em 't'? A primeira metade de cada periodo,
    /// logo depois da borda de subida.
    bool Alto(float t, float frequencia);

    /// Quanto dura o passo do registrador: depressa, para o passo ser um
    /// salto, e sempre menos que o meio periodo - ele para antes da proxima
    /// borda.
    float DuracaoDoPasso(float frequencia);

    /// O estado k de um contador Johnson de 'bits' bits: os uns entram pela esquerda ate encher, e
    /// saem pela esquerda ate esvaziar.
    std::uint32_t Johnson(int k, int bits);

    // ---- O que muda de uma luta para a outra, e o degrau ----

    /// O clock desta luta: quanto dura cada frequencia (as etapas de fases.json, 1, 2, 4 e 8 Hz,
    /// sempre nessa ordem.
    struct Plano {
        std::array<int, 4> duracoes{5, 5, 5, 5};
        int inicioDoContador = 1;
    };

    /// O contador comeca na primeira metade da volta (0 a 5, enchendo de uns).
    constexpr int kMaiorInicioDoContador = 5;
    Plano Sortear(Variacao::Sorteio& s);

    /// O que o Dificil muda: o registrador anda mais a cada borda (84 px, e
    /// nao 72) - o mesmo clock, tiros mais rapidos.
    struct Degrau {
        float passo = kPasso;
    };
    Degrau DegrauDo(Dificuldade::Nivel n);
}
