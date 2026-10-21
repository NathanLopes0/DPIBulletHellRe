//
// O Programa de Intercambio do Vladimir - para onde cada aluno vai, e quando.
//

#pragma once

#include <vector>

#include "Math.h"

/**
 * CAMADA PURA (mesma disciplina de Nota, Tabela e Fita).
 *
 * O Vladimir e o professor de LINGUAGENS nos dois sentidos - as formais da
 * disciplina e as estrangeiras da diretoria de Relacoes Internacionais. O
 * Intercambio e o segundo sentido virando mecanica: de tempos em tempos ele
 * manda os alunos para outro lugar.
 *
 * EM DUPLA, os dois trocam de lugar. SOZINHO, o aluno vai para a posicao
 * espelhada - para onde o outro aluno estaria. A regra e uma so ("voce vai para
 * o lugar de outro aluno"), e por isso o aviso tambem e um so.
 *
 * Quem move os alunos, desenha os marcadores e troca o texto do placar e
 * Battle. Aqui ficam as duas contas que podem errar: o destino e o relogio.
 */
namespace Intercambio {

    /**
     * @brief Para onde vai cada aluno, na mesma ordem em que eles vieram.
     *
     * Com dois ou mais, cada um vai para o lugar do SEGUINTE da lista - com dois,
     * e uma troca. Com um so, ele vai para o espelho horizontal (x vira
     * largura - x) e fica na mesma altura: o campo e simetrico, e subir ou descer
     * o aluno de surpresa o jogaria numa faixa que ele nao escolheu.
     *
     * Sem aluno nenhum, nao ha destino.
     */
    std::vector<Vector2> Destinos(const std::vector<Vector2>& posicoes, float largura);

    /**
     * @brief O relogio de uma fase com Intercambio.
     *
     * A troca acontece a cada 'aCada' segundos desde o comeco da fase, e o aviso
     * acende 'aviso' segundos antes de cada uma. Um relogio com 'aviso' maior ou
     * igual a 'aCada' - ou sem periodo - nunca troca: seria um aviso aceso para
     * sempre, e o leitor de fases.json ja os recusa.
     */
    struct Relogio {
        float aCada = 0.0f;
        float aviso = 0.0f;
        float t = 0.0f;
    };

    /// @brief O relogio e valido (periodo positivo, aviso menor que o periodo)?
    bool Valido(const Relogio& r);

    /**
     * @brief Anda o relogio. Devolve true no quadro em que a troca acontece.
     *
     * Depois da troca o relogio recomeca a contagem, carregando o que sobrou do
     * quadro - a segunda troca e exatamente 'aCada' depois da primeira, e nao
     * um quadro mais tarde.
     */
    bool Avancar(Relogio& r, float deltaTime);

    /// @brief O aviso esta aceso? (do inicio do aviso ate a troca)
    bool Avisando(const Relogio& r);
}
