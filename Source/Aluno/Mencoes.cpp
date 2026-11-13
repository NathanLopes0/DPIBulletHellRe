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
        for (const Mencao desafio : kDesafios) {
            por(desafio, progresso.Atividades().count(Chave(desafio, m->codigo)) > 0);
        }
        return d;
    }

    std::string Chave(const Mencao desafio, const std::string& materia) {
        const char* nome = "";
        switch (desafio) {
            case Mencao::Pacifista: nome = "pacifista"; break;
            case Mencao::Intocavel: nome = "intocavel"; break;
            case Mencao::Mira:      nome = "mira"; break;
            case Mencao::DePerto:   nome = "de-perto"; break;
            default:                return "";
        }
        return std::string("desafio-") + nome + "@" + materia;
    }

    std::vector<std::string> Desafios(const Prova& p, const std::string& materia) {
        std::vector<std::string> chaves;
        if (!p.aprovado) return chaves;
        const auto se = [&](const Mencao m, const bool cumpriu) { if (cumpriu) chaves.push_back(Chave(m, materia)); };
        se(Mencao::Pacifista, p.tiros == 0);
        se(Mencao::Intocavel, p.acertosSofridos == 0);
        se(Mencao::Mira, p.tiros >= kTirosDaMira &&
                         static_cast<float>(p.acertosNoProfessor) >= kPrecisaoDaMira * static_cast<float>(p.tiros));
        se(Mencao::DePerto, p.acertosNoProfessor >= kAcertosDePerto &&
                            static_cast<float>(p.acertosDePerto) >= kFracaoDePerto * static_cast<float>(p.acertosNoProfessor));
        return chaves;
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
            case Mencao::Pacifista: return "Q";
            case Mencao::Intocavel: return "I";
            case Mencao::Mira:      return "A";
            case Mencao::DePerto:   return "F";
        }
        return "";
    }

    const char* ChaveDe(const Mencao m) {
        switch (m) {
            case Mencao::Bronze:    return "bronze";
            case Mencao::Prata:     return "prata";
            case Mencao::Ouro:      return "ouro";
            case Mencao::NotaCheia: return "notaCheia";
            case Mencao::Dificil:   return "dificil";
            case Mencao::Dupla:     return "dupla";
            case Mencao::Orientou:  return "orientou";
            case Mencao::Pacifista: return "pacifista";
            case Mencao::Intocavel: return "intocavel";
            case Mencao::Mira:      return "mira";
            case Mencao::DePerto:   return "dePerto";
        }
        return "";
    }

    std::optional<Mencao> PelaChave(const std::string& chave) {
        for (const Mencao m : kTodas) {
            if (chave == ChaveDe(m)) return m;
        }
        return std::nullopt;
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
            case Mencao::Pacifista: return "quietinho da turma";
            case Mencao::Intocavel: return "intoc\xc3\xa1vel";
            case Mencao::Mira:      return "prestando aten\xc3\xa7\xc3\xa3o";
            case Mencao::DePerto:   return "sentar na frente";
        }
        return "";
    }

    const char* ComoGanhar(const Mencao m) {
        // As duvidas sao os tiros do aluno. \xc3\xa1 a, \xc3\xa9 e, \xc3\xad i, \xc3\xb3 o, \xc3\xba u agudos;
        // \xc3\xa7 c cedilha; \xc3\xa3 a til; \xc3\xaa e circunflexo.
        switch (m) {
            case Mencao::Bronze:    return "Passe na mat\xc3\xa9ria.";
            case Mencao::Prata:     return "Passe com nota 80 ou mais.";
            case Mencao::Ouro:      return "Passe com nota 90 ou mais.";
            case Mencao::NotaCheia: return "Termine a prova com a nota cheia: 100.";
            case Mencao::Dificil:   return "Passe no modo Dif\xc3\xad" "cil.";
            case Mencao::Dupla:     return "Passe jogando em dupla.";
            case Mencao::Orientou:  return "Passe no TCC com o professor da mat\xc3\xa9ria de orientador.";
            case Mencao::Pacifista: return "Passe sem fazer nenhuma pergunta.";
            case Mencao::Intocavel: return "Passe sem ser acertado nenhuma vez.";
            case Mencao::Mira:      return "70% das suas d\xc3\xba" "vidas chegam ao professor (de 50 ou mais).";
            case Mencao::DePerto:   return "Metade dos acertos feitos de perto (de 50 ou mais).";
        }
        return "";
    }

    std::string Legenda() {
        // \xc3\xa7\xc3\xa3 e "ca" de aprovacao; \xc2\xb7, o ponto do meio.
        const std::string ponto = "  \xc2\xb7  ";
        return std::string("B aprova\xc3\xa7\xc3\xa3o") + ponto + "P nota 80" + ponto + "O nota 90" + ponto +
               "100 nota cheia" + ponto + "D no Dif\xc3\xad" "cil" + ponto + "2 em dupla" + ponto +
               "T orientou o seu TCC" + ponto + "Q quietinho" + ponto + "I intoc\xc3\xa1vel" + ponto +
               "A prestando aten\xc3\xa7\xc3\xa3o" + ponto + "F sentar na frente";
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
            case Mencao::Pacifista: return {140, 225, 160};
            case Mencao::Intocavel: return {255, 150, 205};
            case Mencao::Mira:      return {100, 225, 215};
            case Mencao::DePerto:   return {255, 160, 60};
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
