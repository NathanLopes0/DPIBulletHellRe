//
// Os guardrails da fase 3 do Julio: cada onda e um retreino.
//

#pragma once

/**
 * CAMADA PURA. Coberto por test_guardrail.cpp.
 *
 * A CADA 6 s O MODELO E RETREINADO, e o anel de guardrails que volta nao e o
 * mesmo: alterna entre FECHADO, colado no professor, e ABERTO, longe dele;
 * troca o sentido do giro; e gira mais depressa a cada retreino, ate um teto.
 * Antes todo anel era igual - 95 px, horario, 160 px/s -, e o aluno decorava
 * a janela por onde atirar.
 *
 * Quem aplica e Julio::OnProjectilesCreated, trocando a corda de cada
 * guardrail que nasce (AmarrarBehavior).
 */
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

    /// @brief A k-esima onda da fase (0 e a primeira).
    Onda DaVez(int k);
}
