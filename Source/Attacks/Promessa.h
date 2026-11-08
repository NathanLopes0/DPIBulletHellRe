//
// A Promise do JavaScript - a decisao de resolver ou rejeitar.
//

#pragma once

/// Na fase final do Lucas os projeteis sao promessas: saem, param pendentes e, quando assentam, ou
/// se resolvem - disparam contra o aluno.
namespace Promessa {

    /// A promessa se resolve? 'fracao' e a parte das promessas que resolvem, de 0 (nenhuma) a 1
    /// (todas); 'sorteio' e um numero em [0, 1).
    bool Resolve(float sorteio, float fracao);
}
