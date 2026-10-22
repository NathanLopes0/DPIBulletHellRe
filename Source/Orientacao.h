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
 * Quem desenha a tela e a cena EscolhaDoOrientador; quem guarda o escolhido e
 * o Game (Game::DefinirOrientador). Coberto por test_orientacao.cpp.
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

    /**
     * @brief A votacao em curso, na tela: onde esta o cursor de cada aluno e em
     * quem cada um ja votou.
     *
     * Um aluno so anda enquanto nao votou - o voto TRAVA o cursor, e e isso que
     * a tela mostra. Desfazer destrava, e o cursor fica onde estava. Quem le o
     * escolhido e Resultado, sobre Votos(): enquanto alguem nao votou, nao ha
     * escolhido.
     *
     * Um aluno fora da conta (indice negativo, ou alem dos que jogam) e
     * ignorado: nao anda, nao vota, e as perguntas sobre ele respondem como se
     * ele nao tivesse votado.
     */
    class Votacao {
    public:
        Votacao(int candidatos, int alunos);

        [[nodiscard]] int Alunos() const { return static_cast<int>(mCursor.size()); }
        [[nodiscard]] int QuantosCandidatos() const { return mCandidatos; }

        /// Em que candidato o cursor do aluno esta. Comeca no primeiro, o de
        /// nota mais alta.
        [[nodiscard]] int Cursor(int aluno) const;
        [[nodiscard]] bool Votou(int aluno) const;

        /// Anda 'passo' casas, dando a volta nas pontas. Quem ja votou nao anda.
        void Mover(int aluno, int passo);

        /// Vota onde o cursor esta. Sem candidato nenhum, nao ha em quem votar.
        void Votar(int aluno);

        /// Desfaz o voto. Devolve false se o aluno nao tinha votado - e quem
        /// chama decide o que isso quer dizer (o aluno 1 volta para a selecao).
        bool Desfazer(int aluno);

        /// Um voto por aluno, -1 para quem nao votou: o que Resultado le.
        [[nodiscard]] std::vector<int> Votos() const;

        /// Se todos votaram, e no mesmo. Sozinho, votar ja e concordar.
        [[nodiscard]] bool Concordaram() const;

    private:
        [[nodiscard]] bool Valido(int aluno) const { return aluno >= 0 && aluno < Alunos(); }

        int mCandidatos;
        std::vector<int> mCursor;
        std::vector<bool> mVotou;
    };
}
