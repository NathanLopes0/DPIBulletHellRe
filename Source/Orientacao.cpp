//
// O orientador do TCC: quem pode orientar, e quem a dupla escolhe.
//

#include "Orientacao.h"

#include <algorithm>
#include <set>

namespace Orientacao {

    std::vector<Candidato> Candidatos(const Materias::Lista& lista,
                                      const std::vector<const Progresso*>& alunos) {
        std::vector<Candidato> todos;
        for (int i = 0; i < lista.Quantas(); ++i) {
            const Materias::Materia* m = lista.Por(i);
            if (!m || m->orientacao || m->chefe.empty() || m->professor.empty()) continue;

            // A NOTA DA DUPLA E A MAIOR DOS DOIS - e quem a tirou.
            Candidato c;
            c.materia = i;
            c.professor = m->professor;
            c.chefe = m->chefe;
            for (size_t a = 0; a < alunos.size(); ++a) {
                if (!alunos[a]) continue;
                const float nota = alunos[a]->MelhorNota(i);
                if (nota > c.nota) {
                    c.nota = nota;
                    c.deQuem = static_cast<int>(a);
                }
            }
            if (c.nota < Progresso::kNotaDeAprovacao) continue;
            todos.push_back(c);
        }

        // A MAIOR PRIMEIRO; no empate, a ordem do curso (a ordenacao e estavel).
        std::stable_sort(todos.begin(), todos.end(),
                         [](const Candidato& a, const Candidato& b) { return a.nota > b.nota; });

        // UM PROFESSOR UMA VEZ SO, pela melhor nota - que, com a lista em
        // ordem, e a primeira em que ele aparece.
        std::vector<Candidato> escolhidos;
        std::set<std::string> vistos;
        for (const auto& c : todos) {
            if (static_cast<int>(escolhidos.size()) >= kCandidatos) break;
            if (!vistos.insert(c.professor).second) continue;
            escolhidos.push_back(c);
        }
        return escolhidos;
    }

    int Resultado(const std::vector<Candidato>& candidatos, const std::vector<int>& votos) {
        if (votos.empty() || candidatos.empty()) return -1;
        const int n = static_cast<int>(candidatos.size());

        // CONCORDANDO OU NAO, vence o mais acima na lista - que e o de nota
        // mais alta. Se todos votaram no mesmo, e ele.
        int vencedor = n;
        for (const int voto : votos) {
            if (voto < 0 || voto >= n) return -1;   // alguem ainda nao votou
            vencedor = std::min(vencedor, voto);
        }
        return vencedor;
    }

    Votacao::Votacao(const int candidatos, const int alunos)
        : mCandidatos(std::max(0, candidatos)),
          mCursor(static_cast<size_t>(std::max(0, alunos)), 0),
          mVotou(static_cast<size_t>(std::max(0, alunos)), false) {
    }

    int Votacao::Cursor(const int aluno) const {
        return Valido(aluno) ? mCursor[static_cast<size_t>(aluno)] : 0;
    }

    bool Votacao::Votou(const int aluno) const {
        return Valido(aluno) && mVotou[static_cast<size_t>(aluno)];
    }

    void Votacao::Mover(const int aluno, const int passo) {
        if (!Valido(aluno) || mCandidatos <= 0 || Votou(aluno)) return;
        int& c = mCursor[static_cast<size_t>(aluno)];
        // O RESTO DE C++ E NEGATIVO para negativo: soma n antes de tirar o resto.
        c = ((c + passo) % mCandidatos + mCandidatos) % mCandidatos;
    }

    void Votacao::Votar(const int aluno) {
        if (!Valido(aluno) || mCandidatos <= 0) return;
        mVotou[static_cast<size_t>(aluno)] = true;
    }

    bool Votacao::Desfazer(const int aluno) {
        if (!Votou(aluno)) return false;
        mVotou[static_cast<size_t>(aluno)] = false;
        return true;
    }

    std::vector<int> Votacao::Votos() const {
        std::vector<int> votos;
        votos.reserve(mCursor.size());
        for (int a = 0; a < Alunos(); ++a) votos.push_back(Votou(a) ? Cursor(a) : -1);
        return votos;
    }

    bool Votacao::Concordaram() const {
        if (Alunos() == 0) return false;
        for (int a = 0; a < Alunos(); ++a) {
            if (!Votou(a) || Cursor(a) != Cursor(0)) return false;
        }
        return true;
    }
}
