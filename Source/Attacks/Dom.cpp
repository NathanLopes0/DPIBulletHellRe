//
// A fase 1 do Lucas: um documento HTML sendo escrito, tag por tag.
//

#include "Dom.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace Dom {

    namespace {
        int Novo(Documento& d, const std::string& tag, const int pai, const bool vazio = false) {
            Elemento e;
            e.tag = tag;
            e.pai = pai;
            e.vazio = vazio;
            if (pai >= 0) e.profundidade = d.elementos[static_cast<size_t>(pai)].profundidade + 1;
            d.elementos.push_back(e);
            const int novo = static_cast<int>(d.elementos.size()) - 1;
            if (pai >= 0) d.elementos[static_cast<size_t>(pai)].filhos.push_back(novo);
            return novo;
        }

        /// Um filho da raiz, de um molde de HTML de verdade.
        void Molde(Variacao::Sorteio& s, Documento& d, const int pai) {
            switch (s.Inteiro(0, 4)) {
                case 0: {
                    const int lista = Novo(d, s.Chance(0.5f) ? "ul" : "ol", pai);
                    const int itens = s.Inteiro(2, 3);
                    for (int k = 0; k < itens; ++k) Novo(d, "li", lista);
                    break;
                }
                case 1: {
                    const int p = Novo(d, "p", pai);
                    if (s.Chance(0.6f)) Novo(d, s.Chance(0.5f) ? "a" : "strong", p);
                    break;
                }
                case 2: {
                    const int div = Novo(d, "div", pai);
                    Novo(d, "p", div);
                    if (s.Chance(0.5f)) Novo(d, "img", div, true);
                    else Novo(d, "span", div);
                    break;
                }
                case 3:
                    Novo(d, s.Chance(0.5f) ? "h1" : "h2", pai);
                    break;
                default:
                    Novo(d, s.Chance(0.5f) ? "img" : "input", pai, true);
                    break;
            }
        }

        void Posicionar(Documento& d, const int n, const float largura) {
            Elemento& e = d.elementos[static_cast<size_t>(n)];
            const auto m = static_cast<float>(e.filhos.size());
            const float espaco = kEntreIrmaos / std::pow(2.0f, static_cast<float>(e.profundidade));
            for (size_t k = 0; k < e.filhos.size(); ++k) {
                Elemento& f = d.elementos[static_cast<size_t>(e.filhos[k])];
                const float x = e.centro.x + (static_cast<float>(k) - (m - 1.0f) / 2.0f) * espaco;
                f.centro = {std::clamp(x, kMargem, largura - kMargem), e.centro.y + kEntreGeracoes};
            }
            for (const int f : d.elementos[static_cast<size_t>(n)].filhos) Posicionar(d, f, largura);
        }
    }

    Documento Gerar(Variacao::Sorteio& s, const float largura) {
        Documento d;
        static const std::vector<std::string> kRaizes = {"div", "main", "section", "article"};
        Novo(d, s.Um(kRaizes), -1);
        const int filhos = s.Inteiro(2, 3);
        // Cada molde tem ate quatro elementos: so entra se couber.
        for (int k = 0; k < filhos && static_cast<int>(d.elementos.size()) + 4 <= kMaisElementos; ++k) Molde(s, d, 0);
        d.elementos[0].centro = {largura / 2.0f + s.Real(-kDesvioDaRaiz, kDesvioDaRaiz), kYDaRaiz};

        Posicionar(d, 0, largura);
        return d;
    }

    std::vector<Evento> Eventos(const Documento& d) {
        std::vector<Evento> eventos;
        std::function<void(int)> escrever = [&](const int n) {
            const Elemento& e = d.elementos[static_cast<size_t>(n)];
            if (e.vazio) { eventos.push_back({Tipo::Vazio, n}); return; }
            eventos.push_back({Tipo::Abre, n});
            for (const int f : e.filhos) escrever(f);
            eventos.push_back({Tipo::Fecha, n});
        };
        if (!d.elementos.empty()) escrever(0);
        return eventos;
    }

    std::string Tag(const Documento& d, const Evento& e) {
        const std::string& tag = d.elementos[static_cast<size_t>(e.elemento)].tag;
        return (e.tipo == Tipo::Fecha ? "</" : "<") + tag + ">";
    }

    std::string Caminho(const Documento& d, const int elemento) {
        std::vector<std::string> nomes;
        for (int n = elemento; n >= 0; n = d.elementos[static_cast<size_t>(n)].pai) {
            nomes.push_back(d.elementos[static_cast<size_t>(n)].tag);
        }
        std::string caminho;
        for (auto it = nomes.rbegin(); it != nomes.rend(); ++it) {
            if (!caminho.empty()) caminho += " > ";
            caminho += *it;
        }
        return caminho;
    }

    std::string Placar(const Documento& d, const Evento& e) {
        return Tag(d, e) + "\n" + Caminho(d, e.elemento);
    }

    float Encolher(const int profundidade) {
        return std::pow(0.7f, static_cast<float>(std::max(0, profundidade)));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.porEvento = Dificuldade::Escolher(n, kPorEvento, 0.32f);


        return d;
    }
}
