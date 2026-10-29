//
// A repescagem do Andre: a prova no BOCA.
//

#include "Prova.h"

#include <vector>

namespace Prova {

    int ProblemaEm(const float t, const Lista& prova) {
        for (int k = 0; k < kProblemas; ++k) {
            const auto& p = prova[static_cast<std::size_t>(k)];
            if (t >= p.inicio && t < p.fim) return k;
        }
        return (t < prova[0].inicio) ? 0 : -1;
    }

    bool Aceito(const int k, const Lista& prova) {
        if (k < 0 || k >= kProblemas) return false;
        return std::string(prova[static_cast<std::size_t>(k)].veredito) == kAceito;
    }

    std::string Veredito(const int k, const Lista& prova) {
        if (k < 0 || k >= kProblemas) return {};
        const auto& p = prova[static_cast<std::size_t>(k)];
        return std::string("Problema ") + p.letra + ": " + p.veredito;
    }

    std::string Abertura(const int k, const Lista& prova) {
        if (k < 0 || k >= kProblemas) return {};
        const auto& p = prova[static_cast<std::size_t>(k)];
        std::string texto = std::string("Problema ") + p.letra + ": " + p.nome;
        if (k > 0) {
            const auto& antes = prova[static_cast<std::size_t>(k - 1)];
            texto = std::string(1, antes.letra) + ": " + antes.veredito + (Aceito(k - 1, prova) ? "!  " : "  ") + texto;
        }
        return texto;
    }

    std::string CaraDoTrofeu(const int k, const Lista& prova) {
        if (!Aceito(k, prova)) return {};
        return std::string("Letra") + prova[static_cast<std::size_t>(k)].letra;
    }

    Lista Embaralhada(Variacao::Sorteio& s) {
        std::vector<int> ordem = {0, 1, 2, 3};
        s.Embaralhar(ordem);
        ordem.push_back(kProblemas - 1);
        Lista prova = kProva;
        float t = 0.0f;
        for (int k = 0; k < kProblemas; ++k) {
            Problema p = kProva[static_cast<std::size_t>(ordem[static_cast<std::size_t>(k)])];
            const float dura = p.fim - p.inicio;
            p.letra = static_cast<char>('A' + k);
            p.inicio = t;
            p.fim = t + dura;
            t = p.fim;
            prova[static_cast<std::size_t>(k)] = p;
        }
        return prova;
    }
}
