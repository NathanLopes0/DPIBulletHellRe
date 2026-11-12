//
// As mencoes de cada materia.
//

#include "Mencoes.h"

#include <algorithm>

namespace Mencoes {

    int DaMateria::Ganhas() const {
        return static_cast<int>(std::count(tem.begin(), tem.end(), true));
    }

    int DaMateria::Possiveis() const {
        return static_cast<int>(std::count(existe.begin(), existe.end(), true));
    }

    DaMateria Da(const Progresso& progresso, const Materias::Lista& materias, const int materia) {
        DaMateria d;
        const Materias::Materia* m = materias.Por(materia);
        if (!m || m->chefe.empty()) return d;

        d.existe.fill(true);
        // Quem pede orientador nao orienta.
        d.existe[static_cast<size_t>(Mencao::Orientou)] = !m->orientacao;

        const float melhor = progresso.MelhorNota(materia);
        const auto aprovadoEm = [&](const Progresso::Modo modo, const Dificuldade::Nivel nivel) {
            return progresso.Recorde(materia, modo, nivel).nota >= Progresso::kNotaDeAprovacao;
        };
        const auto por = [&d](const Mencao qual, const bool tem) {
            d.tem[static_cast<size_t>(qual)] = tem && d.existe[static_cast<size_t>(qual)];
        };
        por(Mencao::Bronze, progresso.Aprovado(materia));
        por(Mencao::Prata, melhor >= kNotaDaPrata);
        por(Mencao::Ouro, melhor >= kNotaDoOuro);
        por(Mencao::NotaCheia, Nota::ECheia(melhor));
        por(Mencao::Dificil, aprovadoEm(Progresso::Modo::Solo, Dificuldade::Nivel::Dificil) ||
                             aprovadoEm(Progresso::Modo::Dupla, Dificuldade::Nivel::Dificil));
        por(Mencao::Dupla, aprovadoEm(Progresso::Modo::Dupla, Dificuldade::Nivel::Normal) ||
                           aprovadoEm(Progresso::Modo::Dupla, Dificuldade::Nivel::Dificil));
        por(Mencao::Orientou, progresso.Orientacoes(materia) > 0);
        return d;
    }

    Total DoCurso(const Progresso& progresso, const Materias::Lista& materias) {
        Total t;
        for (int i = 0; i < materias.Quantas(); ++i) {
            const DaMateria d = Da(progresso, materias, i);
            t.ganhas += d.Ganhas();
            t.possiveis += d.Possiveis();
        }
        return t;
    }

    std::vector<Mencao> Novas(const DaMateria& antes, const DaMateria& depois) {
        std::vector<Mencao> novas;
        for (const Mencao m : kTodas) {
            if (depois.Tem(m) && !antes.Tem(m)) novas.push_back(m);
        }
        return novas;
    }

    const char* Selo(const Mencao m) {
        switch (m) {
            case Mencao::Bronze:    return "B";
            case Mencao::Prata:     return "P";
            case Mencao::Ouro:      return "O";
            case Mencao::NotaCheia: return "100";
            case Mencao::Dificil:   return "D";
            case Mencao::Dupla:     return "2";
            case Mencao::Orientou:  return "T";
        }
        return "";
    }

    const char* Nome(const Mencao m) {
        // \xc3\xad e o i com acento: o MSVC nao le acento em literal.
        switch (m) {
            case Mencao::Bronze:    return "bronze";
            case Mencao::Prata:     return "prata";
            case Mencao::Ouro:      return "ouro";
            case Mencao::NotaCheia: return "nota cheia";
            case Mencao::Dificil:   return "no Dif\xc3\xad" "cil";
            case Mencao::Dupla:     return "em dupla";
            case Mencao::Orientou:  return "orientou o TCC";
        }
        return "";
    }

    std::string Legenda() {
        // \xc3\xa7\xc3\xa3 e "ca" de aprovacao; \xc2\xb7, o ponto do meio.
        const std::string ponto = "  \xc2\xb7  ";
        return std::string("B aprova\xc3\xa7\xc3\xa3o") + ponto + "P nota 80" + ponto + "O nota 90" + ponto +
               "100 nota cheia" + ponto + "D no Dif\xc3\xad" "cil" + ponto + "2 em dupla" + ponto +
               "T orientou o seu TCC";
    }

    Cor CorDa(const Mencao m) {
        switch (m) {
            case Mencao::Bronze:    return {214, 138, 72};
            case Mencao::Prata:     return {204, 212, 224};
            case Mencao::Ouro:      return {255, 214, 72};
            case Mencao::NotaCheia: return {255, 244, 170};
            case Mencao::Dificil:   return {240, 104, 104};
            case Mencao::Dupla:     return {110, 200, 255};
            case Mencao::Orientou:  return {186, 158, 250};
        }
        return {};
    }

    std::string FraseDasNovas(const std::vector<Mencao>& novas) {
        if (novas.empty()) return "";
        // \xc3\xa7\xc3\xa3o e "cao"; \xc3\xa7\xc3\xb5 e "co" de mencoes.
        std::string frase = novas.size() == 1 ? "Nova men\xc3\xa7\xc3\xa3o: " : "Novas men\xc3\xa7\xc3\xb5" "es: ";
        for (size_t i = 0; i < novas.size(); ++i) frase += (i == 0 ? "" : ", ") + std::string(Nome(novas[i]));
        return frase;
    }
}
