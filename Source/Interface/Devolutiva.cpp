//
// A devolutiva da prova: onde a nota caiu e o que mais acertou o aluno - e, na dupla, a parte de
// cada um.
//

#include "Devolutiva.h"

#include <algorithm>
#include <cstdio>

namespace Devolutiva {

    namespace {
        /// Quantas letras o texto tem, e nao quantos bytes: o acento ocupa dois em UTF-8.
        std::size_t Letras(const std::string& texto) {
            std::size_t n = 0;
            for (const unsigned char c : texto) if ((c & 0xC0u) != 0x80u) ++n;
            return n;
        }

        std::string Contagem(const int n, const char* um, const char* varios) {
            return std::to_string(n) + " " + (n == 1 ? um : varios);
        }
    }

    void Caderno::ComecarFase(const std::string& anuncio, const bool final) {
        Fase f;
        f.ordem = static_cast<int>(mFases.size()) + 1;
        f.anuncio = anuncio;
        f.final = final;
        mFases.push_back(f);
    }

    void Caderno::Perder(const std::string& causa, const float perda) {
        if (perda <= 0.0f) return;
        // Antes da primeira fase anunciada, a perda e da primeira.
        if (mFases.empty()) ComecarFase("");
        Fase& f = mFases.back();
        f.perdido += perda;
        ++f.acertos;
        Causa& c = mCausas[causa];
        c.causa = causa;
        c.perdido += perda;
        ++c.acertos;
    }

    std::optional<Fase> Caderno::PiorFase() const {
        const Fase* pior = nullptr;
        for (const Fase& f : mFases) {
            if (f.perdido > 0.0f && (!pior || f.perdido > pior->perdido)) pior = &f;
        }
        if (!pior) return std::nullopt;
        return *pior;
    }

    std::vector<Causa> Caderno::Causas() const {
        std::vector<Causa> v;
        for (const auto& [nome, c] : mCausas) v.push_back(c);
        std::stable_sort(v.begin(), v.end(), [](const Causa& a, const Causa& b) {
            if (a.perdido != b.perdido) return a.perdido > b.perdido;
            return a.acertos > b.acertos;
        });
        return v;
    }

    std::string LinhaDaFase(const Fase& f) {
        const std::string onde = f.final ? "prova final" : "fase " + std::to_string(f.ordem);
        char perda[24];
        std::snprintf(perda, sizeof(perda), " (-%.2f)", static_cast<double>(f.perdido));
        const std::string curta = "Onde a nota mais caiu: " + onde + perda;
        if (f.anuncio.empty() || f.final) return curta;
        const std::string longa = "Onde a nota mais caiu: " + onde + ", " + f.anuncio + perda;
        return Letras(longa) <= kMaximoDaLinha ? longa : curta;
    }

    std::string LinhaDaParte(const Parte& p) {
        return p.quem + ": " + Contagem(p.acertos, "acerto", "acertos") + ", " +
               Contagem(p.coberturas, "cobertura", "coberturas") + ", " +
               Contagem(p.sofridos, "sofrido", "sofridos");
    }
}
