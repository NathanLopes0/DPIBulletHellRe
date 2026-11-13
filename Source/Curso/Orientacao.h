//
// O orientador do TCC: quem pode orientar, e quem a dupla escolhe.
//

#pragma once

#include <string>
#include <vector>

#include "Materias.h"
#include "../Aluno/Progresso.h"

/// O TCC do Hugo e a unica batalha que o aluno nao luta sozinho. Aqui ficam as duas regras da
/// escolha.
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

    /// Os candidatos a orientador de quem esta jogando: um Progresso por aluno (um sozinho, dois em
    /// dupla). Nulo conta como quem nunca jogou. Sao os kCandidatos das maiores notas aprovadas - mas
    /// o professor de uma optativa aprovada sempre entra: o orientador novo e o premio dela.
    std::vector<Candidato> Candidatos(const Materias::Lista& lista,
                                      const std::vector<const Progresso*>& alunos);

    /// A votacao: um voto por aluno, o indice do candidato em quem ele votou.
    int Resultado(const std::vector<Candidato>& candidatos, const std::vector<int>& votos);

    /// A votacao em curso, na tela: onde esta o cursor de cada aluno e em quem cada um ja votou.
    class Votacao {
    public:
        Votacao(int candidatos, int alunos);

        [[nodiscard]] int Alunos() const { return static_cast<int>(mCursor.size()); }

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
