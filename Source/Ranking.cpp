//
// A ordem das melhores notas de uma materia.
//

#include "Ranking.h"

#include <algorithm>

namespace Ranking {

namespace {

    /// Ordena e numera. AS DUAS LISTAS PASSAM POR AQUI: se cada uma tivesse a
    /// propria ordenacao, a regra de empate poderia divergir entre elas, e o
    /// mesmo par de alunos apareceria empatado numa tela e desempatado na outra.
    ///
    /// @param notaDe como tirar a nota de uma ficha. E a unica coisa que muda
    ///        entre o ranking de uma materia e o geral.
    template <typename NotaDe, typename JogadasDe>
    std::vector<Linha> Ordenar(const std::vector<Exportacao::FichaDeAluno>& fichas,
                               NotaDe notaDe, JogadasDe jogadasDe) {

        std::vector<Linha> linhas;
        linhas.reserve(fichas.size());

        for (const auto& f : fichas) {
            // Nao disputou: fora. Ver o comentario de DaMateria.
            if (const float nota = notaDe(f); nota > 0.0f) {
                linhas.push_back(Linha{1, f.matricula, nota, jogadasDe(f)});
            }
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

    std::vector<Linha> Completa(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                const int materia) {
        return Ordenar(fichas,
            [materia](const Exportacao::FichaDeAluno& f) { return f.progresso.MelhorNota(materia); },
            [](const Exportacao::FichaDeAluno&) { return 1; });
    }

    std::vector<Linha> CompletaGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                                     const int quantasMaterias) {

        if (quantasMaterias <= 0) return {};

        auto media = [quantasMaterias](const Exportacao::FichaDeAluno& f) {
            float soma = 0.0f;
            for (int m = 0; m < quantasMaterias; ++m) {
                const float nota = f.progresso.MelhorNota(m);
                if (nota > 0.0f) soma += nota;    // materia nao jogada vale zero
            }
            return soma / static_cast<float>(quantasMaterias);
        };

        auto jogadas = [quantasMaterias](const Exportacao::FichaDeAluno& f) {
            int n = 0;
            for (int m = 0; m < quantasMaterias; ++m) if (f.progresso.MelhorNota(m) > 0.0f) ++n;
            return n;
        };

        return Ordenar(fichas, media, jogadas);
    }

    std::vector<Linha> Cortar(std::vector<Linha> linhas, const size_t quantas) {
        if (quantas > 0 && linhas.size() > quantas) linhas.resize(quantas);
        return linhas;
    }
}

std::vector<Linha> DaMateria(const std::vector<Exportacao::FichaDeAluno>& fichas,
                             const int materia, const size_t quantas) {
    return Cortar(Completa(fichas, materia), quantas);
}

std::vector<Linha> Geral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                         const int quantasMaterias, const size_t quantas) {
    return Cortar(CompletaGeral(fichas, quantasMaterias), quantas);
}

int PosicaoDe(const std::vector<Exportacao::FichaDeAluno>& fichas,
              const int materia, const std::string& matricula) {

    for (const auto& l : Completa(fichas, materia)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

int PosicaoNoGeral(const std::vector<Exportacao::FichaDeAluno>& fichas,
                   const int quantasMaterias, const std::string& matricula) {

    for (const auto& l : CompletaGeral(fichas, quantasMaterias)) {
        if (l.matricula == matricula) return l.posicao;
    }
    return 0;
}

}
