//
// A fase 3 do Arroyo: os limites inferiores e a NP-completude.
//

#include "Intratavel.h"

#include <algorithm>
#include <cmath>

namespace Intratavel {

    const std::vector<std::vector<std::pair<int, int>>>& Camadas() {
        // A rede de 6 fios com 12 comparadores em 5 camadas.
        static const std::vector<std::vector<std::pair<int, int>>> camadas = {
            {{0, 5}, {1, 3}, {2, 4}},
            {{1, 2}, {3, 4}},
            {{0, 3}, {2, 5}},
            {{0, 1}, {2, 3}, {4, 5}},
            {{1, 2}, {3, 4}},
        };
        return camadas;
    }

    int Comparadores() {
        int n = 0;
        for (const auto& c : Camadas()) n += static_cast<int>(c.size());
        return n;
    }

    int LimiteInferior(const int n) {
        double bits = 0.0;
        for (int i = 2; i <= n; ++i) bits += std::log2(static_cast<double>(i));
        return static_cast<int>(std::ceil(bits - 1e-9));
    }

    std::vector<int> Ordenar(std::vector<int> valores) {
        for (const auto& camada : Camadas()) {
            for (const auto& [i, j] : camada) {
                if (i < static_cast<int>(valores.size()) && j < static_cast<int>(valores.size()) &&
                    valores[static_cast<size_t>(i)] > valores[static_cast<size_t>(j)]) {
                    std::swap(valores[static_cast<size_t>(i)], valores[static_cast<size_t>(j)]);
                }
            }
        }
        return valores;
    }

    float XDoFio(const int i, const float esquerda, const float largura) {
        return esquerda + largura * (static_cast<float>(i) + 0.5f) / static_cast<float>(kFios);
    }

    float YDaCamada(const int c, const float topo, const float fundo) {
        const int n = static_cast<int>(Camadas().size());
        return topo + (fundo - topo) * (static_cast<float>(c) + 0.5f) / static_cast<float>(n);
    }

    std::string PlacarDaRede() {
        // log2 n! com uma casa, e o arredondamento para cima pela seta: o sinal de teto nao existe
        // na fonte do quadro.
        double bits = 0.0;
        for (int i = 2; i <= kFios; ++i) bits += std::log2(static_cast<double>(i));
        const long decimos = std::lround(bits * 10.0);
        // \xce\xa9 e o omega maiusculo, \xe2\x82\x82 o 2 subscrito, \xe2\x89\x88 o "quase igual" e
        // \xe2\x86\x92 a seta.
        return "\xce\xa9(n log n): log\xe2\x82\x82 " + std::to_string(kFios) + "! \xe2\x89\x88 " +
               std::to_string(decimos / 10) + "," + std::to_string(decimos % 10) + " \xe2\x86\x92 " +
               std::to_string(LimiteInferior(kFios)) + "  \xc2\xb7  a rede usa " + std::to_string(Comparadores());
    }

    long long Rotas(const int n) {
        if (n < 3) return 1;
        long long f = 1;
        for (int i = 2; i <= n - 1; ++i) f *= i;
        return f / 2;
    }

    std::vector<Vector2> Cidades(Variacao::Sorteio& s, const int n, const float x, const float y,
                                 const float largura, const float altura) {
        std::vector<Vector2> c;
        // Longe umas das outras: ate 40 sorteios por cidade, ficando com o que mais se afasta.
        const float minima = std::min(largura, altura) / 3.2f;
        for (int k = 0; k < n; ++k) {
            Vector2 melhor;
            float folga = -1.0f;
            for (int tentativa = 0; tentativa < 40; ++tentativa) {
                const Vector2 p(x + s.Real(0.0f, largura), y + s.Real(0.0f, altura));
                float perto = 1.0e9f;
                for (const Vector2& q : c) perto = std::min(perto, (p - q).Length());
                if (perto > folga) { folga = perto; melhor = p; }
                if (perto >= minima) break;
            }
            c.push_back(melhor);
        }
        return c;
    }

    float Comprimento(const std::vector<Vector2>& cidades, const std::vector<int>& rota) {
        if (rota.size() < 2) return 0.0f;
        float total = 0.0f;
        for (size_t i = 0; i < rota.size(); ++i) {
            const Vector2& a = cidades[static_cast<size_t>(rota[i])];
            const Vector2& b = cidades[static_cast<size_t>(rota[(i + 1) % rota.size()])];
            total += (b - a).Length();
        }
        return total;
    }

    std::vector<int> Tentativa(Variacao::Sorteio& s, const int n) {
        std::vector<int> resto;
        for (int i = 1; i < n; ++i) resto.push_back(i);
        s.Embaralhar(resto);
        std::vector<int> rota = {0};
        rota.insert(rota.end(), resto.begin(), resto.end());
        return rota;
    }

    std::string PlacarDoCaixeiro(const int cidades, const int tentativa, const float melhor) {
        // \xc2\xb7 e o ponto do meio.
        return "n = " + std::to_string(cidades) + "  \xc2\xb7  " + std::to_string(Rotas(cidades)) + " rotas  \xc2\xb7  tentativa " +
               std::to_string(tentativa) + "  \xc2\xb7  melhor " + std::to_string(static_cast<int>(std::lround(melhor)));
    }

    Vector2 NaRota(const std::vector<Vector2>& cidades, const std::vector<int>& rota, const float u) {
        if (rota.empty()) return {};
        if (rota.size() == 1) return cidades[static_cast<size_t>(rota[0])];
        const float total = Comprimento(cidades, rota);
        float falta = std::clamp(u, 0.0f, 1.0f) * total;
        for (size_t i = 0; i < rota.size(); ++i) {
            const Vector2& a = cidades[static_cast<size_t>(rota[i])];
            const Vector2& b = cidades[static_cast<size_t>(rota[(i + 1) % rota.size()])];
            const float trecho = (b - a).Length();
            if (falta <= trecho || i + 1 == rota.size()) {
                const float t = trecho > 0.0f ? std::clamp(falta / trecho, 0.0f, 1.0f) : 0.0f;
                return a + (b - a) * t;
            }
            falta -= trecho;
        }
        return cidades[static_cast<size_t>(rota[0])];
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.porTentativa = 0.8f;
        d.queda = 175.0f;
        return d;
    }
}
