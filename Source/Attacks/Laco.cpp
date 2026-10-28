//
// Os lacos da fase 2 do Andre.
//

#include "Laco.h"

#include <array>
#include <random>
#include <utility>

namespace Laco {

    namespace {
        constexpr std::array<Forma, kFormas> kOrdem = {Forma::Para, Forma::Decrescente, Forma::Multiplica,
                                                       Forma::Continue, Forma::Aninhado, Forma::ForaDoVetor,
                                                       Forma::Infinito};
        constexpr std::array<std::pair<int, int>, 3> kMultiplicas = {{{1, 2}, {1, 3}, {2, 2}}};
        constexpr std::array<int, 3> kAteOFim = {3, 4, 6};    // os passos que chegam em 12 e em -1
        constexpr std::array<const char*, kBytes> kNomesDosBytes = {"DE", "AD", "BE", "EF", "00", "FF"};

        std::string N(const int v) { return std::to_string(v); }
    }

    std::vector<Visita> Execucao(const Programa& p, const std::uint32_t sorteio) {
        std::vector<Visita> v;
        float t = 0.0f;
        auto visita = [&v, &t](const int i, const Tipo tipo) {
            v.push_back({i, tipo, t});
            t += kPasso;
        };
        const int passo = p.passo < 1 ? 1 : p.passo;
        switch (p.forma) {
            case Forma::Para:
                for (int i = p.inicio; i < kCasas; i += passo) visita(i, Tipo::Balao);
                break;
            case Forma::Decrescente:
                for (int i = p.inicio; i >= 0; i -= passo) visita(i, Tipo::Balao);
                break;
            case Forma::Multiplica:
                for (int i = p.inicio < 1 ? 1 : p.inicio; i < kCasas; i *= (passo < 2 ? 2 : passo)) visita(i, Tipo::Balao);
                break;
            case Forma::Continue:
                for (int i = 0; i < kCasas; ++i) visita(i, i % passo == 0 ? Tipo::Pulado : Tipo::Balao);
                break;
            case Forma::Aninhado:
                for (int k = 0; k < passo; ++k) {
                    if (k > 0) t += kEntrePassadas;
                    for (int i = k; i < kCasas; i += passo) visita(i, Tipo::Balao);
                }
                break;
            case Forma::ForaDoVetor:
                if (p.direita) {
                    for (int i = 0; i <= kCasas; i += passo) visita(i, i >= kCasas ? Tipo::Fora : Tipo::Balao);
                } else {
                    for (int i = kCasas - 1; i >= -1; i -= passo) visita(i, i < 0 ? Tipo::Fora : Tipo::Balao);
                }
                break;
            case Forma::Infinito: {
                std::mt19937 g(sorteio);
                int antes = -1;
                for (float u = 0.0f; u < kLimiteDeTempo - 1.0e-4f; u += kPassoInfinito) {
                    int i = static_cast<int>(g() % static_cast<std::uint32_t>(kCasas));
                    if (i == antes) i = (i + 1 + static_cast<int>(g() % static_cast<std::uint32_t>(kCasas - 1))) % kCasas;
                    v.push_back({i, Tipo::Balao, u});
                    antes = i;
                }
                break;
            }
        }
        return v;
    }

    float Duracao(const Programa& p, const std::vector<Visita>& visitas) {
        if (p.forma == Forma::Infinito) return kLimiteDeTempo;
        if (visitas.empty()) return 0.0f;
        if (p.forma == Forma::ForaDoVetor) return visitas.back().quando + kCapotando;
        return visitas.back().quando + kPasso;
    }

    Programa Proximo(const int vez, const std::uint32_t sorteio) {
        std::mt19937 g(sorteio);
        Programa p;
        p.forma = kOrdem[static_cast<std::size_t>(((vez % kFormas) + kFormas) % kFormas)];
        switch (p.forma) {
            case Forma::Para:
                p.passo = 2 + static_cast<int>(g() % 2u);
                p.inicio = static_cast<int>(g() % static_cast<std::uint32_t>(p.passo));
                break;
            case Forma::Decrescente:
                p.passo = 2 + static_cast<int>(g() % 2u);
                p.inicio = kCasas - 1 - static_cast<int>(g() % static_cast<std::uint32_t>(p.passo));
                break;
            case Forma::Multiplica: {
                const auto [inicio, fator] = kMultiplicas[g() % kMultiplicas.size()];
                p.inicio = inicio;
                p.passo = fator;
                break;
            }
            case Forma::Continue:
                p.passo = 3 + static_cast<int>(g() % 2u);
                break;
            case Forma::Aninhado:
                p.passo = 3 + static_cast<int>(g() % 2u);
                break;
            case Forma::ForaDoVetor:
                p.passo = kAteOFim[g() % kAteOFim.size()];
                p.direita = (g() % 2u) == 0u;
                p.inicio = p.direita ? 0 : kCasas - 1;
                break;
            case Forma::Infinito:
                break;
        }
        return p;
    }

    std::string Placar(const Programa& p) {
        switch (p.forma) {
            case Forma::Para: return "for (i = " + N(p.inicio) + "; i < 12; i += " + N(p.passo) + ")";
            case Forma::Decrescente: return "for (i = " + N(p.inicio) + "; i >= 0; i -= " + N(p.passo) + ")";
            case Forma::Multiplica: return "for (i = " + N(p.inicio) + "; i < 12; i *= " + N(p.passo) + ")";
            // OS DE DUAS LINHAS quebram onde o codigo quebraria: o placar
            // escreve o \n, e cada linha fica maior do que se coubesse tudo
            // numa so.
            case Forma::Continue: return "for (i = 0; i < 12; i++)\n    if (i % " + N(p.passo) + " == 0) continue;";
            case Forma::Aninhado:
                return "for (k = 0; k < " + N(p.passo) + "; k++)\n    for (i = k; i < 12; i += " + N(p.passo) + ")";
            case Forma::ForaDoVetor:
                return p.direita ? "for (i = 0; i <= 12; i += " + N(p.passo) + ")"
                                 : "for (i = 11; i >= -1; i -= " + N(p.passo) + ")";
            case Forma::Infinito: return "while (1) v[rand() % 12] = balao;";
        }
        return {};
    }

    float XDaCasa(const int i, const float largura) {
        return largura * (static_cast<float>(i) + 0.5f) / static_cast<float>(kCasas);
    }

    std::string CaraDoIndice(const int i) { return "V" + N(i); }
    std::string CaraDoIndiceAceso(const int i) { return "V" + N(i) + "Aceso"; }
    std::string CaraDoIndicePulado(const int i) { return "V" + N(i) + "Pulado"; }
    std::string CaraDoIndiceErro(const int i) { return "V" + N(i) + "Erro"; }

    std::string CaraDoByte(const int k) {
        return kNomesDosBytes[static_cast<std::size_t>(((k % kBytes) + kBytes) % kBytes)];
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.aviso = Dificuldade::Escolher(n, kAviso, 0.4f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.15f);
        d.baloesPorCacho = Dificuldade::Escolher(n, kBaloesPorCacho, 5);
        d.ladoDoCacho = Dificuldade::Escolher(n, kLadoDoCacho, 24.0f);
        return d;
    }
}
