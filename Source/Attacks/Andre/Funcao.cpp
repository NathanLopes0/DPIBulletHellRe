//
// As funcoes da fase 3 do Andre.
//

#include "Funcao.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <utility>

namespace Funcao {

    namespace {
        constexpr std::array<Forma, kFormas> kOrdem = {Forma::Baloes, Forma::Leque, Forma::BaloesDoDobro,
                                                       Forma::LequeDoMeio, Forma::PorValor, Forma::PorReferencia};

        /// Os meios: todos dao entre 3 e 7, e quase todos tem soma impar - a
        /// divisao de inteiros joga fora o meio.
        constexpr std::array<std::pair<int, int>, 7> kMeios = {{{3, 8}, {2, 9}, {4, 7}, {1, 6}, {2, 7}, {5, 10}, {3, 6}}};

        std::string N(const int v) { return std::to_string(v); }
    }

    int Dobro(const int x) { return 2 * x; }
    int Meio(const int a, const int b) { return (a + b) / 2; }

    Nome Externa(const Comando& c) {
        switch (c.forma) {
            case Forma::Leque:
            case Forma::LequeDoMeio: return Nome::Leque;
            case Forma::PorReferencia: return Nome::Segue;
            default: return Nome::Baloes;
        }
    }

    bool TemInterna(const Comando& c) {
        return c.forma == Forma::BaloesDoDobro || c.forma == Forma::LequeDoMeio;
    }

    Nome Interna(const Comando& c) {
        if (c.forma == Forma::BaloesDoDobro) return Nome::Dobro;
        if (c.forma == Forma::LequeDoMeio) return Nome::Meio;
        return Externa(c);
    }

    std::vector<int> ArgumentosDaInterna(const Comando& c) {
        switch (c.forma) {
            case Forma::Baloes: return {c.a, c.b};
            case Forma::Leque: return {c.a};
            case Forma::BaloesDoDobro: return {c.a};
            case Forma::LequeDoMeio: return {c.a, c.b};
            default: return {};
        }
    }

    int Retorno(const Comando& c) {
        if (c.forma == Forma::BaloesDoDobro) return Dobro(c.a);
        if (c.forma == Forma::LequeDoMeio) return Meio(c.a, c.b);
        return 0;
    }

    std::vector<int> ArgumentosDaExterna(const Comando& c, const int casaDoAluno) {
        switch (c.forma) {
            case Forma::Baloes: return {c.a, c.b};
            case Forma::Leque: return {c.a};
            case Forma::BaloesDoDobro: return {Dobro(c.a), c.b};
            case Forma::LequeDoMeio: return {Meio(c.a, c.b)};
            case Forma::PorValor: return {casaDoAluno - 1, casaDoAluno + 1};
            case Forma::PorReferencia: return {};
        }
        return {};
    }

    std::vector<int> CasasDosBaloes(const int de, const int ate) {
        std::vector<int> casas;
        for (int i = std::max(de, 0); i <= std::min(ate, kCasas - 1); ++i) casas.push_back(i);
        return casas;
    }

    Comando Proximo(const int vez, const std::uint32_t sorteio) {
        std::mt19937 g(sorteio);
        Comando c;
        c.forma = kOrdem[static_cast<std::size_t>(((vez % kFormas) + kFormas) % kFormas)];
        switch (c.forma) {
            case Forma::Baloes: {
                const int largura = 2 + static_cast<int>(g() % 4u);
                c.a = static_cast<int>(g() % static_cast<std::uint32_t>(kCasas - largura));
                c.b = c.a + largura;
                break;
            }
            case Forma::Leque:
                c.a = 3 + static_cast<int>(g() % 5u);
                break;
            case Forma::BaloesDoDobro: {
                c.a = 1 + static_cast<int>(g() % 4u);
                const int menor = Dobro(c.a) + 2;
                c.b = menor + static_cast<int>(g() % static_cast<std::uint32_t>(kCasas - menor));
                break;
            }
            case Forma::LequeDoMeio: {
                const auto [a, b] = kMeios[g() % kMeios.size()];
                c.a = a;
                c.b = b;
                break;
            }
            default:
                break;
        }
        return c;
    }

    std::string Placar(const Comando& c) {
        switch (c.forma) {
            case Forma::Baloes: return "baloes(" + N(c.a) + ", " + N(c.b) + ");";
            case Forma::Leque: return "leque(" + N(c.a) + ");";
            case Forma::BaloesDoDobro: return "baloes(dobro(" + N(c.a) + "), " + N(c.b) + ");";
            case Forma::LequeDoMeio: return "leque(meio(" + N(c.a) + ", " + N(c.b) + "));";
            case Forma::PorValor: return "baloes(aluno - 1, aluno + 1);";
            case Forma::PorReferencia: return "segue(&aluno);";
        }
        return {};
    }

    std::string PlacarComValores(const Comando& c, const int casaDoAluno) {
        const auto args = ArgumentosDaExterna(c, casaDoAluno);
        // A COPIA diz de onde veio: a casa do aluno no instante da chamada.
        const std::string copia = c.forma == Forma::PorValor ? "aluno = " + N(casaDoAluno) + ": " : "";
        switch (Externa(c)) {
            case Nome::Baloes: return copia + "baloes(" + N(args[0]) + ", " + N(args[1]) + ");";
            case Nome::Leque: return "leque(" + N(args[0]) + ");";
            default: return Placar(c);
        }
    }

    std::string CaraDoCartao(const Nome f) {
        switch (f) {
            case Nome::Baloes: return "Baloes";
            case Nome::Leque: return "Leque";
            case Nome::Dobro: return "Dobro";
            case Nome::Meio: return "Meio";
            case Nome::Segue: return "Segue";
        }
        return {};
    }

    std::string CaraDoCartaoAceso(const Nome f) { return CaraDoCartao(f) + "Aceso"; }

    std::string CaraDoValor(const int v) {
        return "N" + N(std::clamp(v, kMenorValor, kMaiorValor));
    }

    int CasaDoX(const float x, const float largura) {
        const int casa = static_cast<int>(std::floor(x / (largura / static_cast<float>(kCasas))));
        return std::clamp(casa, 0, kCasas - 1);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreSegue = Dificuldade::Escolher(n, kEntreSegue, 0.3f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.3f);
        return d;
    }
}
