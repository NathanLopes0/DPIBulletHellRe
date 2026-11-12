//
// As notas da turma em texto, para o professor.
//

#include "Exportacao.h"

#include <algorithm>
#include <sstream>

namespace Exportacao {

std::string Campo(const std::string& valor) {

    const bool precisa = valor.find(',') != std::string::npos
                      || valor.find('"') != std::string::npos
                      || valor.find('\n') != std::string::npos;

    if (!precisa) return valor;

    std::string saida = "\"";
    for (const char c : valor) {
        if (c == '"') saida += "\"\"";   // A aspa duplicada e como o CSV a escapa.
        else saida += c;
    }
    saida += "\"";
    return saida;
}

namespace {

    /// Duas casas. Com uma, o teto de 99,99 sairia "100.0" na planilha, e o professor leria nota
    /// cheia de um aluno que nao a tirou.
    std::string Nota(const float v) {
        std::ostringstream s;
        s.precision(2);
        s << std::fixed << v;
        return s.str();
    }
}

std::string ParaCsv(const std::vector<FichaDeAluno>& fichas,
                    const Materias::Lista& materias) {

    std::ostringstream s;
    // A planilha ganhou as duas colunas do co-op, e nao por capricho.
    s << "matricula,materia,nome,recorde,recordeDupla,parceiro,retomada,aprovado,quando,"
         "recordeDificil,recordeDuplaDificil,parceiroDificil\n";

    // Ordem de matricula: numerica, nao alfabetica ("9" antes de "10").
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

            // A mesma regra que o jogo le, e nao uma comparacao repetida.
            const bool aprovado = f->progresso.Aprovado(e.materia);

            using Modo = Progresso::Modo;
            using Dificuldade::Nivel;
            const Progresso::Marca& duplaNormal = e.De(Modo::Dupla, Nivel::Normal);
            const Progresso::Marca& duplaDificil = e.De(Modo::Dupla, Nivel::Dificil);

            s << Campo(f->matricula) << ','
              << Campo(m->codigo) << ','
              << Campo(m->nome) << ','
              << Nota(e.De(Modo::Solo, Nivel::Normal).nota) << ','
              << Nota(duplaNormal.nota) << ','
              << Campo(duplaNormal.parceiro) << ','
              << Nota(e.retomada) << ','
              << (aprovado ? "sim" : "nao") << ','
              << Campo(e.quando) << ','
              << Nota(e.De(Modo::Solo, Nivel::Dificil).nota) << ','
              << Nota(duplaDificil.nota) << ','
              << Campo(duplaDificil.parceiro) << '\n';
        }
    }

    return s.str();
}

}
