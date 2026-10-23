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

    bool Aceito(const int k) {
        if (k < 0 || k >= kProblemas) return false;
        return std::string(kProva[static_cast<std::size_t>(k)].veredito) == kAceito;
    }

    std::string Veredito(const int k) {
        if (k < 0 || k >= kProblemas) return {};
        const auto& p = kProva[static_cast<std::size_t>(k)];
        return std::string("Problema ") + p.letra + ": " + p.veredito;
    }

    std::string Abertura(const int k) {
        if (k < 0 || k >= kProblemas) return {};
        const auto& p = kProva[static_cast<std::size_t>(k)];
        std::string texto = std::string("Problema ") + p.letra + ": " + p.nome;
        if (k > 0) {
            const auto& antes = kProva[static_cast<std::size_t>(k - 1)];
            texto = std::string(1, antes.letra) + ": " + antes.veredito + (Aceito(k - 1) ? "!  " : "  ") + texto;
        }
        return texto;
    }

    std::string CaraDoTrofeu(const int k) {
        if (!Aceito(k)) return {};
        return std::string("Letra") + kProva[static_cast<std::size_t>(k)].letra;
    }
}
