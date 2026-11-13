//
// A fase 1 do Arroyo: a arvore de recursao.
//

#include "Recorrencia.h"

#include <cmath>
#include <cstdio>

namespace Recorrencia {

    namespace {
        // \xce\x98 e o teta maiusculo; \xc2\xb2 e \xc2\xb3, o expoente 2 e o 3.
        constexpr const char* kTeta = "\xce\x98";

        std::string Potencia(const double e) {
            if (std::fabs(e - 1.0) < 1e-6) return "n";
            if (std::fabs(e - 2.0) < 1e-6) return "n\xc2\xb2";
            if (std::fabs(e - 3.0) < 1e-6) return "n\xc2\xb3";
            char s[16];
            std::snprintf(s, sizeof(s), "%.2f", e);
            std::string t = s;
            for (char& ch : t) if (ch == '.') ch = ',';   // a virgula decimal da sala
            return "n^" + t;
        }

        std::string Custo(const int k) {
            if (k == 0) return "1";
            return Potencia(k);
        }
    }

    const std::vector<Caso>& Casos() {
        static const std::vector<Caso> casos = {
            {2, 2, 1, "merge sort"},
            {1, 2, 0, "busca bin\xc3\xa1ria"},
            {3, 2, 1, "Karatsuba"},
            {2, 2, 0, "percurso em \xc3\xa1rvore"},
            {1, 2, 1, "sele\xc3\xa7\xc3\xa3o linear"},
            {4, 2, 1, "multiplica\xc3\xa7\xc3\xa3o ing\xc3\xaanua"},
            {2, 2, 2, "pares mais distantes"},
            {3, 3, 1, "merge sort tern\xc3\xa1rio"},
        };
        return casos;
    }

    std::vector<int> Sortear(Variacao::Sorteio& s) {
        std::vector<int> ordem = Variacao::Ordem(s, static_cast<int>(Casos().size()), 1);
        ordem.resize(kRodadas);
        return ordem;
    }

    std::string Formula(const Caso& c) {
        const std::string sub = "T(n/" + std::to_string(c.b) + ")";
        const std::string filhos = (c.a == 1 ? std::string() : std::to_string(c.a)) + sub;
        return "T(n) = " + filhos + " + " + Custo(c.k);
    }

    std::string Solucao(const Caso& c) {
        // log_b a contra o expoente de f(n) = n^k: os tres casos do Teorema Mestre.
        const double critico = std::log(static_cast<double>(c.a)) / std::log(static_cast<double>(c.b));
        const auto k = static_cast<double>(c.k);
        std::string dentro;
        if (std::fabs(k - critico) < 1e-6) {
            dentro = c.k == 0 ? "log n" : Potencia(k) + " log n";
        } else if (k < critico) {
            dentro = Potencia(critico);
        } else {
            dentro = Custo(c.k);
        }
        return std::string(kTeta) + "(" + dentro + ")";
    }

    std::string Placar(const Caso& c, const bool pronta) {
        return pronta ? Formula(c) + " = " + Solucao(c) : Formula(c);
    }

    int Niveis(const Caso& c) {
        if (c.a >= 3) return 2;
        return 3;
    }

    int Trabalho(const Caso& c, const int nivel) {
        // O tamanho do subproblema, em unidades de kN: n/b^nivel. O trabalho e esse tamanho elevado a
        // k, na mesma escala da raiz (n^k vale kN tiros).
        const double tamanho = static_cast<double>(kN) / std::pow(static_cast<double>(c.b), nivel);
        if (c.k == 0) return 1;
        const double tiros = std::pow(tamanho, c.k) / std::pow(static_cast<double>(kN), c.k - 1);
        const int t = static_cast<int>(std::lround(tiros));
        return t < 1 ? 1 : t;
    }

    std::vector<float> Desvios(const int a) {
        if (a <= 1) return {0.0f};
        // A abertura toda cresce com os filhos, ate um leque de 72 graus.
        const float abertura = a == 2 ? 44.0f : (a == 3 ? 60.0f : 72.0f);
        std::vector<float> d;
        for (int i = 0; i < a; ++i) {
            d.push_back(-abertura / 2.0f + abertura * static_cast<float>(i) / static_cast<float>(a - 1));
        }
        return d;
    }

    std::string Rotulo(const Caso& c, const int nivel) {
        if (nivel >= Niveis(c)) return "1";
        if (nivel == 0) return "n";
        int divisor = 1;
        for (int i = 0; i < nivel; ++i) divisor *= c.b;
        return "n/" + std::to_string(divisor);
    }

    std::string Cara(const Caso& c, const int nivel) {
        if (nivel >= Niveis(c)) return "Um";
        if (nivel == 0) return "N";
        int divisor = 1;
        for (int i = 0; i < nivel; ++i) divisor *= c.b;
        return "N" + std::to_string(divisor);
    }

    int Resistencia(const int nivel) {
        if (nivel == 0) return 4;
        if (nivel == 1) return 2;
        return 1;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.porNivel = 0.64f;
        d.velocidade = 125.0f;
        d.doTrabalho = 200.0f;
        return d;
    }
}
