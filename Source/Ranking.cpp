//
// A ordem das melhores notas de uma materia.
//

#include "Ranking.h"

#include <algorithm>

namespace Ranking {

namespace {

    /// A lista completa, ordenada, sem cortar. As duas funcoes publicas saem
    /// daqui para nao existirem duas ordenacoes que possam discordar.
    std::vector<Linha> Completa(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                const int materia) {

        std::vector<Linha> linhas;
        linhas.reserve(fichas.size());

        for (const auto& f : fichas) {
            // Nunca jogou esta materia: nao disputa. Ver o comentario de DaMateria.
            if (f.progresso.MelhorNota(materia) <= 0.0f) continue;
            linhas.push_back(Linha{1, f.matricula, f.progresso.MelhorNota(materia)});
        }

        // Nota decrescente; empate em ordem de matricula, so para a ordem ser
        // sempre a mesma. stable_sort nao bastaria: a ordem de entrada vem da
        // listagem de arquivos, que nao e garantida.
        std::sort(linhas.begin(), linhas.end(), [](const Linha& a, const Linha& b) {
            if (a.nota != b.nota) return a.nota > b.nota;
            return a.matricula < b.matricula;
        });

        // A posicao e por NOTA, e nao por indice: dois empatados dividem a
        // posicao, e a seguinte pula. E como se le um placar.
        for (size_t i = 0; i < linhas.size(); ++i) {
            linhas[i].posicao = (i > 0 && linhas[i].nota == linhas[i - 1].nota)
                                    ? linhas[i - 1].posicao
                                    : static_cast<int>(i) + 1;
        }

        return linhas;
    }
}

std::vector<Linha> DaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                             const int materia, const size_t quantas) {

    std::vector<Linha> linhas = Completa(fichas, materia);

    if (quantas > 0 && linhas.size() > quantas) linhas.resize(quantas);
    return linhas;
}

int PosicaoDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
              const int materia, const std::string& matricula) {

    for (const auto& l : Completa(fichas, materia)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

}
