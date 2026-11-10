//
// O mural dos formados.
//

#include "Mural.h"

#include <algorithm>
#include <cctype>

namespace Mural {

    std::vector<Formado> Recentes(const std::vector<Exportacao::FichaDeAluno>& fichas, const int formatura,
                                  const std::size_t quantos) {
        std::vector<Formado> formados;
        if (formatura < 0) return formados;
        for (const auto& f : fichas) {
            if (f.progresso.Aprovado(formatura)) formados.push_back({f.matricula, f.progresso.AprovadaEm(formatura)});
        }
        // A data no formato do Relogio se ordena como texto; sem data, no fim.
        std::sort(formados.begin(), formados.end(), [](const Formado& a, const Formado& b) {
            if (a.quando.empty() != b.quando.empty()) return b.quando.empty();
            if (a.quando != b.quando) return a.quando > b.quando;
            return a.matricula < b.matricula;
        });
        if (formados.size() > quantos) formados.resize(quantos);
        return formados;
    }

    std::string Data(const std::string& quando) {
        // "AAAA-MM-DD", e o resto (a hora) nao importa aqui.
        if (quando.size() < 10 || quando[4] != '-' || quando[7] != '-') return "";
        for (const std::size_t i : {0u, 1u, 2u, 3u, 5u, 6u, 8u, 9u}) {
            if (!std::isdigit(static_cast<unsigned char>(quando[i]))) return "";
        }
        return quando.substr(8, 2) + "/" + quando.substr(5, 2) + "/" + quando.substr(0, 4);
    }
}
