//
// Os guardrails da fase 3 do Julio: cada onda e um retreino.
//

#pragma once

/// A cada 6 s o modelo e retreinado, e o anel de guardrails que volta nao e o mesmo: alterna entre
/// fechado, colado no professor, e aberto, longe dele.
namespace Guardrail {

    constexpr float kFechado = 85.0f;
    constexpr float kAberto = 110.0f;
    constexpr float kRapidezMaxima = 1.5f;   ///< fator sobre a velocidade do arquivo
    constexpr float kPorRetreino = 0.15f;

    struct Onda {
        float comprimento = kFechado;
        bool horario = true;
        float rapidez = 1.0f;    ///< fator sobre a velocidade do arquivo
    };

    /// A k-esima onda da fase (0 e a primeira).
    Onda DaVez(int k);
}
