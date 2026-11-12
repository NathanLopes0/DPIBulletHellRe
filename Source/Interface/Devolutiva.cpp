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
        return p.quem + ": " + NumerosDaParte(p);
    }

    std::string NumerosDaParte(const Parte& p) {
        return Contagem(p.acertos, "acerto", "acertos") + ", " + Contagem(p.coberturas, "cobertura", "coberturas") +
               ", " + Contagem(p.sofridos, "sofrido", "sofridos");
    }

    std::vector<std::string> Titulos(const std::vector<Parte>& partes) {
        if (partes.size() != 2) return {};
        const Parte& a = partes[0];
        const Parte& b = partes[1];

        // O dono de cada papel: quem esta a frente por uma margem do maior dos dois; ninguem, quase no
        // empate - "quem mais" por um acerto e o placar entre parceiros que os titulos evitam.
        const auto dono = [](const int doPrimeiro, const int doSegundo) {
            const auto frente = [](const int x, const int y) {
                return x > y && static_cast<float>(x - y) >= kMargemDoPapel * static_cast<float>(x);
            };
            return frente(doPrimeiro, doSegundo) ? 0 : frente(doSegundo, doPrimeiro) ? 1 : -1;
        };
        struct Papel {
            int dono;
            const char* titulo;
        };
        const Papel papeis[] = {
            {dono(a.coberturas, b.coberturas), "quem mais cobriu"},
            {dono(a.acertos, b.acertos), "quem mais perguntou"},
            {dono(b.sofridos, a.sofridos), "quem mais desviou"},   // desviar e sofrer menos
        };

        std::vector<std::string> titulos(2);
        for (int i = 0; i < 2; ++i) {
            for (const Papel& p : papeis) {
                if (p.dono == i) {
                    titulos[static_cast<std::size_t>(i)] = p.titulo;
                    break;
                }
            }
        }
        // Quem nao e dono de nenhum papel ganha o que fez, sem comparar com o parceiro.
        for (std::size_t i = 0; i < 2; ++i) {
            if (!titulos[i].empty()) continue;
            const Parte& p = partes[i];
            if (p.acertos > 0) titulos[i] = "fez " + Contagem(p.acertos, "pergunta", "perguntas");
            else if (p.coberturas > 0) titulos[i] = "cobriu " + Contagem(p.coberturas, "vez", "vezes");
            else titulos[i] = "esteve junto at\xc3\xa9 o fim";
        }
        return titulos;
    }
}
