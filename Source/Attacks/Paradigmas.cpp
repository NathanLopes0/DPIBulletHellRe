//
// A fase 2 do Arroyo: os paradigmas de projeto.
//

#include "Paradigmas.h"

#include <algorithm>
#include <climits>

namespace Paradigmas {

    int EtapaNoTempo(const float t) {
        if (t < kInicio[1]) return 0;
        if (t < kInicio[2]) return 1;
        return 2;
    }

    // ---- Guloso ----

    const std::vector<Troco>& Trocos() {
        static const std::vector<Troco> trocos = {
            {385, {100, 50, 25, 10, 5}},   // o real: o guloso acerta
            {31, {25, 10, 1}},             // o guloso da sete moedas; o otimo, quatro (10+10+10+1)
            {12, {9, 6, 1}},               // o guloso da quatro; o otimo, duas de 6
        };
        return trocos;
    }

    std::vector<int> Guloso(const Troco& t) {
        std::vector<int> dadas;
        int falta = t.valor;
        for (const int m : t.moedas) {
            while (m > 0 && falta >= m) {
                dadas.push_back(m);
                falta -= m;
            }
        }
        return dadas;
    }

    int Otimo(const Troco& t) {
        std::vector<int> menor(static_cast<size_t>(t.valor) + 1, INT_MAX);
        menor[0] = 0;
        for (int v = 1; v <= t.valor; ++v) {
            for (const int m : t.moedas) {
                if (m <= v && menor[static_cast<size_t>(v - m)] != INT_MAX) {
                    menor[static_cast<size_t>(v)] = std::min(menor[static_cast<size_t>(v)], menor[static_cast<size_t>(v - m)] + 1);
                }
            }
        }
        return menor[static_cast<size_t>(t.valor)] == INT_MAX ? -1 : menor[static_cast<size_t>(t.valor)];
    }

    std::string PlacarDoTroco(const Troco& t) {
        const std::vector<int> dadas = Guloso(t);
        std::string s = "Troco de " + std::to_string(t.valor) + ": ";
        for (size_t i = 0; i < dadas.size(); ++i) s += (i ? "+" : "") + std::to_string(dadas[i]);
        const int otimo = Otimo(t);
        // \xc3\xb3 e o o com acento.
        if (otimo < static_cast<int>(dadas.size())) s += "  (\xc3\xb3timo: " + std::to_string(otimo) + " moedas)";
        else s += "  (\xc3\xb3timo!)";
        return s;
    }

    std::string CaraDaMoeda(const int valor) {
        return "M" + std::to_string(valor);
    }

    float EscalaDaMoeda(const int valor) {
        if (valor >= 100) return 1.5f;
        if (valor >= 25) return 1.25f;
        if (valor >= 5) return 1.05f;
        return 0.9f;
    }

    // ---- Programacao dinamica ----

    std::vector<std::vector<int>> Tabela(const std::string& a, const std::string& b) {
        std::vector<std::vector<int>> t(a.size() + 1, std::vector<int>(b.size() + 1, 0));
        for (size_t i = 1; i <= a.size(); ++i) {
            for (size_t j = 1; j <= b.size(); ++j) {
                t[i][j] = (a[i - 1] == b[j - 1]) ? t[i - 1][j - 1] + 1 : std::max(t[i - 1][j], t[i][j - 1]);
            }
        }
        return t;
    }

    bool Casou(const std::string& a, const std::string& b, const int i, const int j) {
        if (i < 1 || j < 1 || i > static_cast<int>(a.size()) || j > static_cast<int>(b.size())) return false;
        return a[static_cast<size_t>(i - 1)] == b[static_cast<size_t>(j - 1)];
    }

    std::vector<std::vector<std::pair<int, int>>> Ondas(const int linhas, const int colunas) {
        std::vector<std::vector<std::pair<int, int>>> ondas;
        for (int soma = 2; soma <= linhas + colunas; ++soma) {
            std::vector<std::pair<int, int>> onda;
            for (int i = 1; i <= linhas; ++i) {
                const int j = soma - i;
                if (j >= 1 && j <= colunas) onda.emplace_back(i, j);
            }
            ondas.push_back(std::move(onda));
        }
        return ondas;
    }

    Vector2 LugarDaCasa(const int i, const int j, const int linhas, const int colunas, const float centroX,
                        const float topo) {
        const float x0 = centroX - kLarguraDaCasa * static_cast<float>(colunas - 1) / 2.0f;
        (void)linhas;
        return Vector2(x0 + kLarguraDaCasa * static_cast<float>(j - 1), topo + kAlturaDaCasa * static_cast<float>(i - 1));
    }

    std::string CaraDaCasa(const int valor, const bool casou) {
        const int v = std::clamp(valor, 0, 9);
        return (casou ? "C" : "V") + std::to_string(casou ? std::max(v, 1) : v);
    }

    // ---- Backtracking ----

    std::vector<Passo> Busca(const int n) {
        std::vector<Passo> passos;
        std::vector<int> colunas;
        auto cabe = [&](const int linha, const int coluna) {
            for (int l = 0; l < static_cast<int>(colunas.size()); ++l) {
                const int c = colunas[static_cast<size_t>(l)];
                if (c == coluna || std::abs(c - coluna) == linha - l) return false;
            }
            return true;
        };
        // Em profundidade, sem recursao: a pilha e o proprio vetor de colunas.
        std::vector<int> proxima(static_cast<size_t>(n) + 1, 0);
        int linha = 0;
        while (linha >= 0 && linha < n) {
            int c = proxima[static_cast<size_t>(linha)];
            while (c < n && !cabe(linha, c)) ++c;
            if (c < n) {
                colunas.push_back(c);
                passos.push_back({true, linha, c});
                proxima[static_cast<size_t>(linha)] = c + 1;
                ++linha;
                if (linha < n) proxima[static_cast<size_t>(linha)] = 0;
            } else {
                // Nao ha saida nesta linha: volta e tira a rainha de cima.
                proxima[static_cast<size_t>(linha)] = 0;
                --linha;
                if (linha < 0) break;
                passos.push_back({false, linha, colunas.back()});
                colunas.pop_back();
            }
        }
        return passos;
    }

    std::vector<int> NoTabuleiro(const std::vector<Passo>& busca, const int passos, const int n) {
        std::vector<int> col(static_cast<size_t>(n), -1);
        const int ate = std::min(passos, static_cast<int>(busca.size()));
        for (int i = 0; i < ate; ++i) {
            const Passo& p = busca[static_cast<size_t>(i)];
            col[static_cast<size_t>(p.linha)] = p.coloca ? p.coluna : -1;
        }
        return col;
    }

    Vector2 LugarNoTabuleiro(const int linha, const int coluna, const int n, const Vector2& centro) {
        const float meio = static_cast<float>(n - 1) / 2.0f;
        return Vector2(centro.x + (static_cast<float>(coluna) - meio) * kCasaDoTabuleiro,
                       centro.y + (static_cast<float>(linha) - meio) * kCasaDoTabuleiro);
    }

    std::array<float, 8> DirecoesDaRainha() {
        return {0.0f, 45.0f, 90.0f, 135.0f, 180.0f, 225.0f, 270.0f, 315.0f};
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.porPasso = 0.095f;
        d.porOnda = 0.31f;
        d.velocidadeDaMoeda = 265.0f;
        return d;
    }
}
