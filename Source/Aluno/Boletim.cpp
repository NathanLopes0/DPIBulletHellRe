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
        float soma = 0.0f;

        // Periodo a periodo, e dentro dele na ordem da grade; as optativas, numa secao so, no fim.
        for (const bool optativas : {false, true}) {
        for (int coluna = 0; coluna < materias.QuantasColunas(); ++coluna) {
            for (const int i : materias.DaColuna(coluna)) {
                const Materias::Materia* m = materias.Por(i);
                if (!m || m->optativa != optativas) continue;
                Linha l;
                l.materia = i;
                l.codigo = m->nome;
                l.nome = m->nomeCompleto;
                l.professor = m->professor;
                l.periodo = coluna + 1;
                l.optativa = m->optativa;
                if (!l.optativa) ++d.total;
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
                if (l.situacao == Situacao::Aprovade && !l.optativa) ++d.aprovadas;
                d.linhas.push_back(l);
            }
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

    std::string Secao(const Linha& l) {
        return l.optativa ? std::string("Optativas") : Periodo(l.periodo);
    }

    std::string Encurtar(const std::string& texto, const size_t maximo) {
        // Conta as letras, e nao os bytes: um byte 10xxxxxx continua a letra anterior. 'inicio[k]' e
        // onde a letra k comeca.
        std::vector<size_t> inicio;
        for (size_t b = 0; b < texto.size(); ++b) {
            if ((static_cast<unsigned char>(texto[b]) & 0xC0) != 0x80) inicio.push_back(b);
        }
        if (inicio.size() <= maximo || maximo == 0) return texto;
        // Cabem maximo - 1 letras e a reticencia (\xe2\x80\xa6), sem espaco antes dela.
        std::string curto = texto.substr(0, inicio[maximo - 1]);
        while (!curto.empty() && curto.back() == ' ') curto.pop_back();
        return curto + "\xe2\x80\xa6";
    }
}
