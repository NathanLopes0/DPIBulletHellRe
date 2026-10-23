//
// A repescagem do Andre: a prova no BOCA.
//

#include "Prova.h"

namespace Prova {

    int ProblemaEm(const float t) {
        for (int k = 0; k < kProblemas; ++k) {
            const auto& p = kProva[static_cast<std::size_t>(k)];
            if (t >= p.inicio && t < p.fim) return k;
        }
        return (t < kProva[0].inicio) ? 0 : -1;
    }

    std::string Veredito(const int k) {
        if (k < 0 || k >= kProblemas) return {};
        return std::string("Problema ") + kProva[static_cast<std::size_t>(k)].letra + ": YES";
    }

    std::string Abertura(const int k) {
        if (k < 0 || k >= kProblemas) return {};
        const auto& p = kProva[static_cast<std::size_t>(k)];
        std::string texto = std::string("Problema ") + p.letra + ": " + p.nome;
        if (k > 0) texto = std::string(1, kProva[static_cast<std::size_t>(k - 1)].letra) + ": YES!  " + texto;
        return texto;
    }

    std::string CaraDoTrofeu(const int k) {
        if (k < 0 || k >= kProblemas) return {};
        return std::string("Letra") + kProva[static_cast<std::size_t>(k)].letra;
    }
}
