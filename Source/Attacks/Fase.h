//
// Quanto dura uma fase.
//

#pragma once

/// Toda fase de todo chefe dura kDuracao segundos - as tres e a repescagem.
namespace Fase {
    constexpr float kDuracao = 20.0f;

    /// A excecao: o TCC do Hugo e uma batalha maior. Cada etapa tem um escudo a abrir (Objetivo.h)
    /// antes de o Hugo tomar dano, e 20 s davam uma janela so.
    constexpr float kDuracaoDoTcc = 30.0f;
}
