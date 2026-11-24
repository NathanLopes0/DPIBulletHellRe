//
// A fase 2 do Daniel: as k-medias.
//

#include "Agrupamento.h"

#include <array>
#include <cmath>
#include <limits>

#include "Tipos.h"

namespace Agrupamento {

    int MaisPerto(const std::vector<Vector2>& centroides, const Vector2 p) {
        int melhor = -1;
        float menor = std::numeric_limits<float>::max();
        for (size_t i = 0; i < centroides.size(); ++i) {
            const float d = (centroides[i] - p).LengthSq();
            if (d < menor) {
                menor = d;
                melhor = static_cast<int>(i);
            }
        }
        return melhor;
    }

    std::vector<int> Atribuir(const std::vector<Vector2>& pontos, const std::vector<Vector2>& centroides) {
        std::vector<int> grupos;
        grupos.reserve(pontos.size());
        for (const Vector2& p : pontos) grupos.push_back(MaisPerto(centroides, p));
        return grupos;
    }

    std::vector<Vector2> Atualizar(const std::vector<Vector2>& pontos, const std::vector<int>& grupos,
                                   const std::vector<Vector2>& centroides, const std::vector<Vector2>& pesados,
                                   const float peso) {
        std::vector<Vector2> soma(centroides.size(), Vector2::Zero);
        std::vector<float> total(centroides.size(), 0.0f);
        for (size_t k = 0; k < pontos.size() && k < grupos.size(); ++k) {
            const int g = grupos[k];
            if (g < 0 || g >= static_cast<int>(centroides.size())) continue;
            soma[static_cast<size_t>(g)] += pontos[k];
            total[static_cast<size_t>(g)] += 1.0f;
        }
        for (const Vector2& a : pesados) {
            const int g = MaisPerto(centroides, a);
            if (g < 0 || peso <= 0.0f) continue;
            soma[static_cast<size_t>(g)] += a * peso;
            total[static_cast<size_t>(g)] += peso;
        }
        std::vector<Vector2> novos = centroides;
        for (size_t i = 0; i < novos.size(); ++i) {
            if (total[i] > 0.0f) novos[i] = soma[i] * (1.0f / total[i]);
        }
        return novos;
    }

    int TipoDoGrupo(const std::vector<int>& tipos, const std::vector<int>& grupos, const int g) {
        std::array<int, Tipos::kTipos> votos{};
        int algum = 0;
        for (size_t k = 0; k < tipos.size() && k < grupos.size(); ++k) {
            if (grupos[k] != g || tipos[k] < 0 || tipos[k] >= Tipos::kTipos) continue;
            ++votos[static_cast<size_t>(tipos[k])];
            ++algum;
        }
        if (algum == 0) return -1;
        int melhor = 0;
        for (int t = 1; t < Tipos::kTipos; ++t) {
            if (votos[static_cast<size_t>(t)] > votos[static_cast<size_t>(melhor)]) melhor = t;
        }
        return melhor;
    }

    std::vector<Vector2> Sementes(const std::vector<Vector2>& pontos, const int k, Variacao::Sorteio& s) {
        std::vector<Vector2> v;
        if (pontos.empty() || k <= 0) return v;
        v.push_back(pontos[static_cast<size_t>(s.Inteiro(0, static_cast<int>(pontos.size()) - 1))]);
        while (static_cast<int>(v.size()) < k) {
            size_t melhor = 0;
            float maisLonge = -1.0f;
            for (size_t i = 0; i < pontos.size(); ++i) {
                float perto = std::numeric_limits<float>::max();
                for (const Vector2& c : v) perto = std::min(perto, (pontos[i] - c).LengthSq());
                if (perto > maisLonge) {
                    maisLonge = perto;
                    melhor = i;
                }
            }
            v.push_back(pontos[melhor]);
        }
        return v;
    }

    Vector2 LugarDoOutlier(const std::vector<Vector2>& centroides, const Vector2 de, const Vector2 ate,
                           Variacao::Sorteio& s) {
        Vector2 melhor = (de + ate) * 0.5f;
        float maisLonge = -1.0f;
        for (int k = 0; k < 12; ++k) {
            const Vector2 p(s.Real(de.x, ate.x), s.Real(de.y, ate.y));
            float perto = std::numeric_limits<float>::max();
            for (const Vector2& c : centroides) perto = std::min(perto, (p - c).LengthSq());
            if (centroides.empty()) perto = 0.0f;
            if (perto > maisLonge) {
                maisLonge = perto;
                melhor = p;
            }
        }
        return melhor;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreIteracoes = 1.25f;
        d.tirosDoAnel = 28;
        d.velocidadeDoAnel = 135.0f;
        d.entreOutliers = 1.0f;
        d.maisOutliers = 6;
        d.entreLeques = 1.4f;
        d.tirosDoLeque = 8;
        return d;
    }
}
