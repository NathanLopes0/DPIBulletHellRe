//
// A prova da semana.
//

#include "ProvaDaSemana.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sstream>

#include "../Regras/Variacao.h"

namespace ProvaDaSemana {

    namespace {
        /// Os dias desde 1970-01-01 de uma data do calendario (a conta de Howard Hinnant).
        long DiasDesde1970(int ano, const int mes, const int dia) {
            ano -= mes <= 2 ? 1 : 0;
            const long era = (ano >= 0 ? ano : ano - 399) / 400;
            const long doAno = ano - era * 400;
            const long doDia = (153L * (mes + (mes > 2 ? -3 : 9)) + 2) / 5 + dia - 1;
            const long daEra = doAno * 365 + doAno / 4 - doAno / 100 + doDia;
            return era * 146097L + daEra - 719468L;
        }

        std::vector<std::string> Campos(const std::string& linha) {
            std::vector<std::string> campos;
            std::string campo;
            std::istringstream s(linha);
            while (std::getline(s, campo, ';')) campos.push_back(campo);
            return campos;
        }
    }

    Semana Da(const std::string& quando) {
        Semana s;
        int ano = 0, mes = 0, dia = 0;
        if (std::sscanf(quando.c_str(), "%d-%d-%d", &ano, &mes, &dia) != 3) return s;
        if (ano < 1970 || mes < 1 || mes > 12 || dia < 1 || dia > 31) return s;
        // 1970-01-05 foi uma segunda-feira: dai para a frente, de sete em sete.
        const long desdeASegunda = DiasDesde1970(ano, mes, dia) - 4;
        if (desdeASegunda < 0) return s;
        s.indice = static_cast<int>(desdeASegunda / 7);
        const int naSemana = static_cast<int>(desdeASegunda % 7);
        s.faltam = 7 - naSemana;

        // A data da segunda, voltando os dias da semana.
        long z = DiasDesde1970(ano, mes, dia) - naSemana + 719468L;
        const long era = (z >= 0 ? z : z - 146096) / 146097;
        const long daEra = z - era * 146097;
        const long doAno = (daEra - daEra / 1460 + daEra / 36524 - daEra / 146096) / 365;
        const long doDia = daEra - (365 * doAno + doAno / 4 - doAno / 100);
        const long mp = (5 * doDia + 2) / 153;
        const int d = static_cast<int>(doDia - (153 * mp + 2) / 5 + 1);
        const int m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
        const int a = static_cast<int>(doAno + era * 400 + (m <= 2 ? 1 : 0));
        char texto[16];
        std::snprintf(texto, sizeof(texto), "%04d-%02d-%02d", a, m, d);
        s.segunda = texto;
        return s;
    }

    std::string DiaDaSegunda(const Semana& s) {
        if (!s.Valida() || s.segunda.size() < 10) return "";
        return s.segunda.substr(8, 2) + "/" + s.segunda.substr(5, 2);
    }

    std::uint32_t Semente(const Semana& s) {
        return Variacao::Misturar(static_cast<std::uint32_t>(std::max(0, s.indice)), Variacao::Rotulo("prova-da-semana"));
    }

    int MateriaDa(const Semana& s, const Materias::Lista& materias) {
        if (!s.Valida()) return -1;
        std::vector<int> servem;
        for (int i = 0; i < materias.Quantas(); ++i) {
            const Materias::Materia* m = materias.Por(i);
            // Nem o TCC (pede orientador) nem as optativas: so as obrigatorias com batalha.
            if (m && !m->chefe.empty() && !m->orientacao && !m->optativa) servem.push_back(i);
        }
        if (servem.empty()) return -1;
        return servem[static_cast<size_t>(s.indice) % servem.size()];
    }

    std::string Cabecalho() {
        return "semana;materia;modo;dificuldade;matricula;parceiro;nota;quando";
    }

    std::string Linha(const Entrada& e) {
        char nota[24];
        std::snprintf(nota, sizeof(nota), "%.2f", static_cast<double>(e.nota));
        return e.semana + ";" + e.materia + ";" + (e.dupla ? "dupla" : "solo") + ";" +
               (e.dificil ? "dificil" : "normal") + ";" + e.matricula + ";" + e.parceiro + ";" + nota + ";" + e.quando;
    }

    std::vector<Entrada> Ler(const std::string& texto) {
        std::vector<Entrada> entradas;
        std::istringstream s(texto);
        std::string linha;
        while (std::getline(s, linha)) {
            if (!linha.empty() && linha.back() == '\r') linha.pop_back();
            const std::vector<std::string> c = Campos(linha);
            if (c.size() < 7 || c[0] == "semana" || c[4].empty()) continue;
            Entrada e;
            e.semana = c[0];
            e.materia = c[1];
            e.dupla = c[2] == "dupla";
            e.dificil = c[3] == "dificil";
            e.matricula = c[4];
            e.parceiro = c[5];
            char* fim = nullptr;
            e.nota = std::strtof(c[6].c_str(), &fim);
            if (fim == c[6].c_str() || e.nota < 0.0f) continue;
            if (c.size() > 7) e.quando = c[7];
            entradas.push_back(e);
        }
        return entradas;
    }

    std::string Quem(const Entrada& e) {
        if (!e.dupla || e.parceiro.empty()) return e.matricula;
        return std::min(e.matricula, e.parceiro) + " + " + std::max(e.matricula, e.parceiro);
    }

    std::vector<Colocado> Ranking(const std::vector<Entrada>& entradas, const Semana& s, const bool dupla,
                                  const bool dificil) {
        // A melhor de cada um.
        std::map<std::string, float> melhor;
        for (const Entrada& e : entradas) {
            if (e.semana != s.segunda || e.dupla != dupla || e.dificil != dificil) continue;
            float& nota = melhor[Quem(e)];
            nota = std::max(nota, e.nota);
        }
        std::vector<Colocado> ranking;
        for (const auto& [quem, nota] : melhor) ranking.push_back({1, quem, nota});
        std::stable_sort(ranking.begin(), ranking.end(),
                         [](const Colocado& a, const Colocado& b) { return a.nota > b.nota; });
        for (size_t i = 1; i < ranking.size(); ++i) {
            ranking[i].posicao = ranking[i].nota == ranking[i - 1].nota ? ranking[i - 1].posicao
                                                                        : static_cast<int>(i) + 1;
        }
        return ranking;
    }

    int PosicaoDe(const std::vector<Colocado>& ranking, const std::string& quem) {
        for (const Colocado& c : ranking) if (c.quem == quem) return c.posicao;
        return 0;
    }
}
