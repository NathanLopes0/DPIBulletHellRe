//
// O if, o else if e o switch da fase 1 do Andre.
//

#include "Condicional.h"

#include <array>
#include <cmath>
#include <random>
#include <utility>

namespace Condicional {

    namespace {
        constexpr std::array<int, 4> kMaiores = {3, 4, 5, 6};
        constexpr std::array<std::pair<int, int>, 3> kIntervalos = {{{2, 7}, {3, 8}, {1, 6}}};
        constexpr std::array<std::pair<int, int>, 3> kSenaoSes = {{{3, 6}, {3, 7}, {4, 7}}};
        constexpr std::array<int, 2> kAtribuicoes = {0, 7};

        /// O switch: os cases que tem quem caia depois deles, mais vezes, e um
        /// sem case. Nunca o 0 - ele faria cair todos, e nao haveria faixa
        /// segura.
        constexpr std::array<int, 6> kNoSwitch = {1, 2, 2, 3, 3, 7};

        constexpr std::array<const char*, kCores> kNomesDasCores = {"Vermelho", "Azul", "Amarelo", "Verde"};

        std::string N(const int v) { return std::to_string(v); }
    }

    int Ramos(const Comando& c) {
        switch (c.forma) {
            case Forma::SenaoSe: return 3;
            case Forma::Escolha: return kCases;
            default: return 2;
        }
    }

    int Depois(const Comando& c, const int n) {
        return c.forma == Forma::Atribuicao ? c.a : n;
    }

    std::vector<int> Executados(const Comando& c, const int n) {
        const int m = Depois(c, n);
        switch (c.forma) {
            case Forma::Maior: return {m > c.a ? 0 : 1};
            case Forma::Intervalo: return {(m > c.a && m < c.b) ? 0 : 1};
            case Forma::Par: return {m % 2 == 0 ? 0 : 1};
            case Forma::Impar: return {m % 2 == 1 ? 0 : 1};
            case Forma::SenaoSe: return {m < c.a ? 0 : (m < c.b ? 1 : 2)};
            case Forma::Atribuicao: return {m != 0 ? 0 : 1};
            case Forma::Escolha: {
                std::vector<int> ramos;
                if (m < 0 || m >= kCases) return ramos;
                for (int k = m; k < kCases; ++k) ramos.push_back(k);
                return ramos;
            }
        }
        return {};
    }

    Comando Proximo(const int vez, const std::uint32_t sorteio) {
        std::mt19937 g(sorteio);
        const auto escolhe = [&g](const std::size_t quantos) { return static_cast<std::size_t>(g() % quantos); };
        Comando c;
        switch (((vez % 5) + 5) % 5) {
            case 0:
                if (vez == 0 || g() % 2u == 0u) {
                    c.forma = Forma::Maior;
                    c.a = kMaiores[escolhe(kMaiores.size())];
                } else {
                    c.forma = Forma::Intervalo;
                    const auto [a, b] = kIntervalos[escolhe(kIntervalos.size())];
                    c.a = a;
                    c.b = b;
                }
                break;
            case 1:
                c.forma = (vez == 1 || g() % 2u == 0u) ? Forma::Par : Forma::Impar;
                break;
            case 2: {
                c.forma = Forma::SenaoSe;
                const auto [a, b] = kSenaoSes[escolhe(kSenaoSes.size())];
                c.a = a;
                c.b = b;
                break;
            }
            case 3:
                c.forma = Forma::Escolha;
                break;
            default:
                c.forma = Forma::Atribuicao;
                c.a = kAtribuicoes[escolhe(kAtribuicoes.size())];
                break;
        }
        return c;
    }

    int Sortear(const Comando& c, const std::uint32_t sorteio) {
        std::mt19937 g(sorteio);
        if (c.forma == Forma::Escolha) return kNoSwitch[g() % kNoSwitch.size()];
        return static_cast<int>(g() % static_cast<std::uint32_t>(kMaiorValor + 1));
    }

    std::string Placar(const Comando& c) {
        switch (c.forma) {
            case Forma::Maior: return "if (n > " + N(c.a) + ") { ... } else { ... }";
            case Forma::Intervalo: return "if (n > " + N(c.a) + " && n < " + N(c.b) + ") { ... } else { ... }";
            case Forma::Par: return "if (n % 2 == 0) { ... } else { ... }";
            case Forma::Impar: return "if (n % 2 == 1) { ... } else { ... }";
            case Forma::SenaoSe: return "if (n < " + N(c.a) + ") ... else if (n < " + N(c.b) + ") ... else ...";
            case Forma::Escolha: return "switch (n) - sem break!";
            case Forma::Atribuicao: return "if (n = " + N(c.a) + ") { ... } else { ... }";
        }
        return {};
    }

    std::string Explicacao(const Comando& c, const int n) {
        if (c.forma == Forma::Atribuicao) {
            return "if (n = " + N(c.a) + ") atribui " + N(c.a) + ": sempre " +
                   (c.a != 0 ? "verdadeiro" : "falso");
        }
        if (c.forma != Forma::Escolha) return {};
        if (n < 0 || n >= kCases) return "n = " + N(n) + ": nenhum case, nada executa";
        if (n == kCases - 1) return {};
        return "sem break: do case " + N(n) + " cai at\xC3\xA9 o case " + N(kCases - 1);
    }

    std::string CaraDoRotulo(const Comando& c, const int ramo) {
        const int ultimo = Ramos(c) - 1;
        if (c.forma == Forma::Escolha) return "Caso" + N(ramo);
        if (ramo == ultimo) return "Senao";
        switch (c.forma) {
            case Forma::Maior: return "Maior" + N(c.a);
            case Forma::Intervalo: return "Entre" + N(c.a) + "e" + N(c.b);
            case Forma::Par: return "Par";
            case Forma::Impar: return "Impar";
            case Forma::SenaoSe: return ramo == 0 ? "Menor" + N(c.a) : "SenaoMenor" + N(c.b);
            case Forma::Atribuicao: return "Atribui" + N(c.a);
            case Forma::Escolha: break;
        }
        return {};
    }

    std::string CaraDoRotuloAceso(const Comando& c, const int ramo) {
        return CaraDoRotulo(c, ramo) + "Aceso";
    }

    std::vector<Comando> Todos() {
        std::vector<Comando> todos;
        for (const int a : kMaiores) todos.push_back({Forma::Maior, a, 0});
        for (const auto& [a, b] : kIntervalos) todos.push_back({Forma::Intervalo, a, b});
        todos.push_back({Forma::Par, 0, 0});
        todos.push_back({Forma::Impar, 0, 0});
        for (const auto& [a, b] : kSenaoSes) todos.push_back({Forma::SenaoSe, a, b});
        todos.push_back({Forma::Escolha, 0, 0});
        for (const int a : kAtribuicoes) todos.push_back({Forma::Atribuicao, a, 0});
        return todos;
    }

    std::string CaraDoBalao(const int n, const int cor) {
        const int c = ((cor % kCores) + kCores) % kCores;
        return std::string(kNomesDasCores[static_cast<std::size_t>(c)]) + N(n);
    }

    std::string CaraDoConfete(const int cor) {
        const int c = ((cor % kCores) + kCores) % kCores;
        return kNomesDasCores[static_cast<std::size_t>(c)];
    }

    bool operator==(const Comando& x, const Comando& y) {
        return x.forma == y.forma && x.a == y.a && x.b == y.b;
    }

    float InicioDaFaixa(const int ramo, const int ramos, const float largura) {
        return largura * static_cast<float>(ramo) / static_cast<float>(ramos);
    }

    float FimDaFaixa(const int ramo, const int ramos, const float largura) {
        return largura * static_cast<float>(ramo + 1) / static_cast<float>(ramos);
    }

    std::vector<Confete> Confetes(const float inicio, const float fim) {
        std::vector<Confete> confetes;
        // A PRIMEIRA FILEIRA VAI DE PONTA A PONTA, as duas bordas inclusive: a
        // divisa entre duas faixas e das duas. Sem isso, a divisa era um
        // corredor seguro - e o aluno parado no meio do campo (a divisa de
        // duas e de quatro faixas) nao era acertado nunca.
        for (int f = 0; f < kFileiras; ++f) {
            const float desvio = (f % 2 == 0) ? 0.0f : kEntreConfetes / 2.0f;
            const int quantos = static_cast<int>(std::floor((fim - inicio - desvio) / kEntreConfetes + 1.0e-3f));
            for (int k = 0; k <= quantos; ++k) {
                const float x = inicio + desvio + kEntreConfetes * static_cast<float>(k);
                if (f % 2 != 0 && x >= fim - 1.0e-3f) break;
                confetes.push_back({x, kEntreFileiras * static_cast<float>(f)});
            }
        }
        return confetes;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n == Dificuldade::Nivel::Dificil) {
            d.baloesPorComando = 4;
            d.entreBaloes = 0.6f;
            d.naDecisao = 0.15f;
            d.rapidez = 1.4f;
        }
        return d;
    }
}
