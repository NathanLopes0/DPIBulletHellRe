//
// O boletim do semestre: as materias do aluno, periodo a periodo, com a melhor nota, a situacao e
// as tentativas - e o CR.
//

#include "Boletim.h"

#include <algorithm>
#include <cstdio>

#include "../Regras/Dificuldade.h"

namespace Boletim {

    Dados Montar(const Progresso& progresso, const Materias::Lista& materias) {
        Dados d;
        d.total = materias.Quantas();
        float soma = 0.0f;

        // Periodo a periodo, e dentro dele na ordem da grade.
        for (int coluna = 0; coluna < materias.QuantasColunas(); ++coluna) {
            for (const int i : materias.DaColuna(coluna)) {
                const Materias::Materia* m = materias.Por(i);
                if (!m) continue;
                Linha l;
                l.materia = i;
                l.codigo = m->nome;
                l.nome = m->nomeCompleto;
                l.professor = m->professor;
                l.periodo = coluna + 1;
                l.melhor = progresso.MelhorNota(i);
                l.tentativas = progresso.Tentativas(i);
                l.reprovacoes = progresso.Reprovacoes(i);
                for (const Progresso::Modo modo : {Progresso::Modo::Solo, Progresso::Modo::Dupla}) {
                    if (progresso.Recorde(i, modo, Dificuldade::Nivel::Dificil).nota >= Progresso::kNotaDeAprovacao) {
                        l.noDificil = true;
                    }
                }

                const bool cursou = l.melhor > 0.0f || l.tentativas > 0;
                if (progresso.Aprovado(i)) l.situacao = Situacao::Aprovade;
                else if (cursou) l.situacao = Situacao::Reprovade;
                else if (materias.Desbloqueada(i, progresso)) l.situacao = Situacao::ACursar;
                else l.situacao = Situacao::Fechada;

                if (cursou) {
                    ++d.cursadas;
                    soma += l.melhor;
                }
                if (l.situacao == Situacao::Aprovade) ++d.aprovadas;
                d.linhas.push_back(l);
            }
        }
        d.cr = d.cursadas > 0 ? soma / static_cast<float>(d.cursadas) : 0.0f;
        return d;
    }

    const char* NomeDa(const Situacao s) {
        switch (s) {
            case Situacao::Aprovade:  return "Aprovade";
            case Situacao::Reprovade: return "Reprovade";
            case Situacao::ACursar:   return "A cursar";
            case Situacao::Fechada:   return "Fechada";
        }
        return "";
    }

    std::string Tentativas(const Linha& l) {
        return l.tentativas > 0 ? std::to_string(l.tentativas) : "-";
    }

    std::string Nota(const float n) {
        if (n <= 0.0f) return "-";
        char b[16];
        std::snprintf(b, sizeof(b), "%.2f", static_cast<double>(n));
        return b;
    }

    std::string Periodo(const int n) {
        // \xc2\xba e o "o" sobrescrito.
        return std::to_string(n) + "\xc2\xba per\xc3\xadodo";
    }
}
