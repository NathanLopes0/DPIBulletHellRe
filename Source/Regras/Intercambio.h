//
// O Programa de Intercambio do Vladimir - para onde cada aluno vai, e quando.
//

#pragma once

#include <vector>

#include "../Motor/Math.h"

/// O Vladimir e o professor de linguagens nos dois sentidos - as formais da disciplina e as
/// estrangeiras da diretoria de Relacoes Internacionais.
namespace Intercambio {

    /// Para onde vai cada aluno, na mesma ordem em que eles vieram. Com dois ou mais, cada um vai
    /// para o lugar do seguinte da lista - com dois, e uma troca.
    std::vector<Vector2> Destinos(const std::vector<Vector2>& posicoes, float largura);

    /// O relogio de uma fase com Intercambio.
    struct Relogio {
        float aCada = 0.0f;
        float aviso = 0.0f;
        float t = 0.0f;
    };

    /// O relogio e valido (periodo positivo, aviso menor que o periodo)?
    bool Valido(const Relogio& r);

    /// Anda o relogio. Devolve true no quadro em que a troca acontece.
    bool Avancar(Relogio& r, float deltaTime);

    /// O aviso esta aceso? (do inicio do aviso ate a troca)
    bool Avisando(const Relogio& r);
}
