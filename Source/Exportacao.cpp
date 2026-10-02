//
// As notas da turma em texto, para o professor.
//

#include "Exportacao.h"

#include <algorithm>
#include <sstream>

namespace Exportacao {

std::string Campo(const std::string& valor) {

    // Um nome de materia com virgula quebraria a coluna seguinte, e e exatamente o
    // tipo de coisa que so aparece quando alguem renomeia uma materia meses depois.
    const bool precisa = valor.find(',') != std::string::npos
                      || valor.find('"') != std::string::npos
                      || valor.find('\n') != std::string::npos;

    if (!precisa) return valor;

    std::string saida = "\"";
    for (const char c : valor) {
        if (c == '"') saida += "\"\"";   // a aspa duplicada e como o CSV a escapa
        else saida += c;
    }
    saida += "\"";
    return saida;
}

namespace {

    std::string Nota(const float v) {
        std::ostringstream s;
        s.precision(1);
        s << std::fixed << v;
        return s.str();
    }
}

std::string ParaCsv(const std::vector<FichaDeAluno>& fichas,
                    const Materias::Lista& materias) {

    std::ostringstream s;
    s << "matricula,materia,nome,recorde,retomada,aprovado,quando\n";

    // Ordem de matricula: numerica, nao alfabetica ("9" antes de "10"). O mesmo
    // criterio de FichaArquivo::ListarMatriculas.
    std::vector<const FichaDeAluno*> ordenadas;
    ordenadas.reserve(fichas.size());
    for (const auto& f : fichas) ordenadas.push_back(&f);

    std::sort(ordenadas.begin(), ordenadas.end(),
              [](const FichaDeAluno* a, const FichaDeAluno* b) {
                  if (a->matricula.size() != b->matricula.size())
                      return a->matricula.size() < b->matricula.size();
                  return a->matricula < b->matricula;
              });

    for (const auto* f : ordenadas) {
        for (const auto& e : f->progresso.Entradas()) {

            const Materias::Materia* m = materias.Por(e.materia);
            if (!m) continue;   // materia que saiu do curso

            s << Campo(f->matricula) << ','
              << Campo(m->codigo) << ','
              << Campo(m->nome) << ','
              << Nota(e.recorde) << ','
              << Nota(e.retomada) << ','
              << (e.recorde >= Progresso::kNotaDeAprovacao ? "sim" : "nao") << ','
              << Campo(e.quando) << '\n';
        }
    }

    return s.str();
}

}
