//
// O orientador do TCC: quem pode orientar, e quem a dupla escolhe.
//

#pragma once

#include <string>
#include <vector>

#include "Materias.h"
#include "Progresso.h"

/**
 * CAMADA PURA.
 *
 * O TCC do Hugo e a unica batalha que o aluno nao luta sozinho: antes dela ele
 * escolhe um ORIENTADOR, e o orientador luta do lado dele com um ataque da
 * materia que da. Aqui ficam as duas regras da escolha.
 *
 * QUEM PODE ORIENTAR. O professor de uma materia em que o aluno foi APROVADO e
 * que tem batalha - o chefe dela e o que o orientador sabe fazer -, menos as
 * materias que pedem orientacao. Das que sobram, as kCandidatos de nota mais
 * alta, a maior primeiro; no empate, a que vem antes no curso. Um professor
 * aparece uma vez so, pela melhor nota dele.
 *
 * EM DUPLA, A NOTA DE CADA MATERIA E A MAIOR DOS DOIS: a dupla e tao boa numa
 * materia quanto o melhor dela. E os dois VOTAM: se votam no mesmo, e esse; se
 * nao, vence o de nota mais alta, que e o que esta mais acima na lista.
 *
 * Quem desenha a tela e a Orientacao (a cena); quem guarda o escolhido e o
 * Game. Coberto por test_orientacao.cpp.
 */
namespace Orientacao {

    /// Quantos professores o aluno pode escolher.
    constexpr int kCandidatos = 3;

    struct Candidato {
        int materia = -1;        ///< o indice da materia na lista
        std::string professor;   ///< quem orienta
        std::string chefe;       ///< o que ele sabe fazer: o nome do chefe dele
        float nota = 0.0f;       ///< a maior nota (da dupla) naquela materia
        int deQuem = 0;          ///< qual aluno tirou essa nota; no empate, o primeiro
    };

    /// @brief Os candidatos a orientador de quem esta jogando: um Progresso por
    /// aluno (um sozinho, dois em dupla). Nulo conta como quem nunca jogou.
    std::vector<Candidato> Candidatos(const Materias::Lista& lista,
                                      const std::vector<const Progresso*>& alunos);

    /// @brief A votacao: um voto por aluno, o indice do candidato em quem ele
    /// votou. Devolve o indice do escolhido, ou -1 se alguem ainda nao votou
    /// (voto negativo, ou fora da lista) ou nao ha voto nenhum.
    int Resultado(const std::vector<Candidato>& candidatos, const std::vector<int>& votos);
}
