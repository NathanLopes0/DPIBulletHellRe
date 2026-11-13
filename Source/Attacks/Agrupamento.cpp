//
// A fase 2 do Daniel: os atributos e o aprendizado nao supervisionado.
//

#include "Agrupamento.h"

#include <cmath>
#include <limits>

namespace Agrupamento {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    Eixo ComponentePrincipal(const std::vector<Vector2>& ps) {
        Eixo e{Vector2::Zero, Vector2(1.0f, 0.0f)};
        if (ps.empty()) return e;
        for (const Vector2& p : ps) e.centro += p;
        e.centro *= 1.0f / static_cast<float>(ps.size());
        float sxx = 0.0f, syy = 0.0f, sxy = 0.0f;
        for (const Vector2& p : ps) {
            const Vector2 d = p - e.centro;
            sxx += d.x * d.x;
            syy += d.y * d.y;
            sxy += d.x * d.y;
        }
        if (std::fabs(sxx - syy) < 1e-6f && std::fabs(sxy) < 1e-6f) return e;
        // O autovetor do maior autovalor da covariancia, em duas dimensoes, sai do angulo duplo.
        const float a = 0.5f * Math::Atan2(2.0f * sxy, sxx - syy);
        e.direcao = Vector2(Math::Cos(a), Math::Sin(a));
        return e;
    }

    Vector2 Projetar(const Eixo& e, const Vector2 p) {
        return e.centro + e.direcao * Vector2::Dot(p - e.centro, e.direcao);
    }

    Vector2 Girar(const Vector2 p, const Vector2 centro, const float graus) {
        const float a = Math::ToRadians(graus), c = Math::Cos(a), s = Math::Sin(a);
        const Vector2 d = p - centro;
        return centro + Vector2(d.x * c - d.y * s, d.x * s + d.y * c);
    }

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
                                   const std::vector<Vector2>& centroides, const std::vector<Vector2>& alunos,
                                   const float pesoDoAluno) {
        std::vector<Vector2> soma(centroides.size(), Vector2::Zero);
        std::vector<float> peso(centroides.size(), 0.0f);
        for (size_t k = 0; k < pontos.size() && k < grupos.size(); ++k) {
            const int g = grupos[k];
            if (g < 0 || g >= static_cast<int>(centroides.size())) continue;
            soma[static_cast<size_t>(g)] += pontos[k];
            peso[static_cast<size_t>(g)] += 1.0f;
        }
        for (const Vector2& a : alunos) {
            const int g = MaisPerto(centroides, a);
            if (g < 0 || pesoDoAluno <= 0.0f) continue;
            soma[static_cast<size_t>(g)] += a * pesoDoAluno;
            peso[static_cast<size_t>(g)] += pesoDoAluno;
        }
        std::vector<Vector2> novos = centroides;
        for (size_t i = 0; i < novos.size(); ++i) {
            if (peso[i] > 0.0f) novos[i] = soma[i] * (1.0f / peso[i]);
        }
        return novos;
    }

    float Inercia(const std::vector<Vector2>& pontos, const std::vector<int>& grupos,
                  const std::vector<Vector2>& centroides) {
        float soma = 0.0f;
        for (size_t k = 0; k < pontos.size() && k < grupos.size(); ++k) {
            const int g = grupos[k];
            if (g < 0 || g >= static_cast<int>(centroides.size())) continue;
            soma += (pontos[k] - centroides[static_cast<size_t>(g)]).LengthSq();
        }
        return soma;
    }

    std::vector<Vector2> Celula(const std::vector<Vector2>& centroides, const int i, const Vector2 de, const Vector2 ate) {
        if (i < 0 || i >= static_cast<int>(centroides.size())) return {};
        std::vector<Vector2> poligono = {de, Vector2(ate.x, de.y), ate, Vector2(de.x, ate.y)};
        const Vector2 ci = centroides[static_cast<size_t>(i)];
        for (size_t j = 0; j < centroides.size() && !poligono.empty(); ++j) {
            if (static_cast<int>(j) == i) continue;
            const Vector2 cj = centroides[j];
            const Vector2 normal = cj - ci;
            if (normal.LengthSq() < 1e-8f) continue;
            const Vector2 meio = (ci + cj) * 0.5f;
            // Fica o lado de ci: (p - meio) . normal <= 0. Sutherland-Hodgman, uma aresta por vez.
            const auto dentro = [&](const Vector2& p) { return Vector2::Dot(p - meio, normal) <= 0.0f; };
            std::vector<Vector2> cortado;
            for (size_t k = 0; k < poligono.size(); ++k) {
                const Vector2 a = poligono[k], b = poligono[(k + 1) % poligono.size()];
                const bool da = dentro(a), db = dentro(b);
                if (da) cortado.push_back(a);
                if (da != db) {
                    const float fa = Vector2::Dot(a - meio, normal), fb = Vector2::Dot(b - meio, normal);
                    const float u = fa / (fa - fb);
                    cortado.push_back(a + (b - a) * u);
                }
            }
            poligono = std::move(cortado);
        }
        return poligono;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.pontos = 66;
        d.entreIteracoes = 0.95f;
        d.pesoDoAluno = 13.0f;
        d.giroDoEixo = 270.0f;
        d.tirosDoAnel = 16;
        return d;
    }
}
