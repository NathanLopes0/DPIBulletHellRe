//
// A fase 3 do Daniel: o overfitting.
//

#include "Sobreajuste.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace Sobreajuste {

    void Registrar(Memoria& m, const Vector2 aluno, const float agora) {
        if (agora < m.proximo) return;
        m.proximo = agora + kEntreRegistros;
        m.pontos.push_back(aluno);
        if (static_cast<int>(m.pontos.size()) > kMaisRegistros) m.pontos.erase(m.pontos.begin());
    }

    Vector2 Previsao(const Memoria& m, const float adiante) {
        if (m.pontos.empty()) return Vector2::Zero;
        const Vector2 ultimo = m.pontos.back();
        if (m.pontos.size() < 2) return ultimo;
        const size_t n = std::min<size_t>(4, m.pontos.size() - 1);
        const Vector2 antes = m.pontos[m.pontos.size() - 1 - n];
        const Vector2 velocidade = (ultimo - antes) * (1.0f / (static_cast<float>(n) * kEntreRegistros));
        return ultimo + velocidade * adiante;
    }

    std::vector<Vector2> Decorados(const Memoria& m, const float lado, const int quantas) {
        const float l = std::max(1.0f, lado);
        std::map<std::pair<int, int>, std::pair<int, Vector2>> celulas;
        for (const Vector2& p : m.pontos) {
            const auto chave = std::make_pair(static_cast<int>(std::floor(p.x / l)), static_cast<int>(std::floor(p.y / l)));
            auto& c = celulas[chave];
            ++c.first;
            c.second += p;
        }
        std::vector<std::pair<int, Vector2>> v;
        for (const auto& [chave, c] : celulas) v.emplace_back(c.first, c.second * (1.0f / static_cast<float>(c.first)));
        std::stable_sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
        std::vector<Vector2> saida;
        for (size_t i = 0; i < v.size() && static_cast<int>(i) < quantas; ++i) saida.push_back(v[i].second);
        return saida;
    }

    std::vector<Vector2> Curva(const std::vector<Vector2>& pontos, const int porTrecho, const float exagero) {
        std::vector<Vector2> v;
        if (pontos.size() < 2) return pontos;
        const int n = std::max(1, porTrecho);
        for (size_t i = 0; i + 1 < pontos.size(); ++i) {
            const Vector2 p0 = pontos[i == 0 ? 0 : i - 1], p1 = pontos[i], p2 = pontos[i + 1];
            const Vector2 p3 = pontos[std::min(i + 2, pontos.size() - 1)];
            Vector2 lado(-(p2 - p1).y, (p2 - p1).x);
            if (lado.Length() > 1e-4f) lado = lado * (1.0f / lado.Length());
            for (int k = 0; k < n; ++k) {
                const float u = static_cast<float>(k) / static_cast<float>(n);
                const float u2 = u * u, u3 = u2 * u;
                // Catmull-Rom: passa por p1 (u = 0) e por p2 (u = 1).
                Vector2 c = (p1 * 2.0f + (p2 - p0) * u + (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * u2 +
                             (p1 * 3.0f - p0 - p2 * 3.0f + p3) * u3) * 0.5f;
                // O balanco entre os pontos: zero nas pontas (a curva ainda passa por eles).
                c = c + lado * (exagero * std::sin(Math::Pi * u) * (i % 2 == 0 ? 1.0f : -1.0f));
                v.push_back(c);
            }
        }
        v.push_back(pontos.back());
        return v;
    }

    Vector2 OndeDoNucleo(const int k, const Vector2 professor, const float t) {
        const float a = Math::ToRadians(kGiroDaOrbita * t + 120.0f * static_cast<float>(k) + 90.0f);
        return professor + Vector2(std::cos(a), std::sin(a) * 0.7f) * kRaioDaOrbita;
    }

    bool Aberto(const int k, const float t) {
        if (t >= kValidacao) return true;
        return k == static_cast<int>(std::floor(std::max(0.0f, t) / kEntreAberturas)) % kNucleos;
    }

    Degrau DegrauDo(const Dificuldade::Nivel nivel) {
        Degrau g;
        if (nivel == Dificuldade::Nivel::Dificil) {
            g.entreTiros = 0.45f;
            g.velocidade = 310.0f;
            g.adiante = 1.0f;
            g.entreDecorados = 0.9f;
            g.tirosNoDecorado = 9;
            g.avisoDoDecorado = 0.5f;
            g.vidaDoNucleo = 40.0f;
            g.premioDoNucleo = 2.0f;
            g.entreLeques = 1.5f;
            g.tirosDoLeque = 8;
        }
        return g;
    }
}
