//
// A fase 1 do Daniel: os dados - a coleta e a preparacao.
//

#include "Preparacao.h"

#include <algorithm>
#include <cmath>

namespace Preparacao {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    std::array<Vector2, kEspecies> Especies(Variacao::Sorteio& s) {
        // Tres lugares de um triangulo largo, girado e mexido: longe das bordas e um do outro.
        std::array<Vector2, kEspecies> e{};
        const float giro = s.Real(0.0f, Math::TwoPi);
        for (int k = 0; k < kEspecies; ++k) {
            const float a = giro + Math::TwoPi * static_cast<float>(k) / static_cast<float>(kEspecies);
            const float r = s.Real(0.24f, 0.3f);
            e[static_cast<size_t>(k)] = Vector2(std::clamp(0.5f + r * Math::Cos(a), 0.16f, 0.84f),
                                                std::clamp(0.42f + r * Math::Sin(a), 0.12f, 0.78f));
        }
        return e;
    }

    Dado Coletar(Variacao::Sorteio& s, const std::array<Vector2, kEspecies>& especies, const float fracaoOutlier,
                 const float fracaoNula) {
        Dado d;
        d.fonte = s.Inteiro(0, static_cast<int>(kFontes.size()) - 1);
        d.especie = s.Inteiro(0, kEspecies - 1);
        const float sorte = s.Real(0.0f, 1.0f);
        if (sorte < fracaoOutlier) {
            // Num canto, longe de tudo: e la que o z-score o acha.
            d.tipo = Tipo::Outlier;
            const int canto = s.Inteiro(0, 3);
            d.onde = Vector2((canto % 2 == 0) ? s.Real(0.0f, 0.06f) : s.Real(0.94f, 1.0f),
                             (canto < 2) ? s.Real(0.0f, 0.06f) : s.Real(0.94f, 1.0f));
            return d;
        }
        d.tipo = sorte < fracaoOutlier + fracaoNula ? Tipo::Nulo : Tipo::Comum;
        // Um dado em volta do centro da especie: a soma de tres sorteios faz um sino.
        const Vector2 c = especies[static_cast<size_t>(d.especie)];
        const auto sino = [&s]() { return (s.Real(-1.0f, 1.0f) + s.Real(-1.0f, 1.0f) + s.Real(-1.0f, 1.0f)) * 0.58f; };
        d.onde = Vector2(std::clamp(c.x + kEspalhamento * sino(), 0.08f, 0.92f),
                         std::clamp(c.y + kEspalhamento * sino(), 0.08f, 0.92f));
        return d;
    }

    Vector2 Media(const std::vector<Vector2>& ps) {
        if (ps.empty()) return Vector2::Zero;
        Vector2 soma = Vector2::Zero;
        for (const Vector2& p : ps) soma += p;
        return soma * (1.0f / static_cast<float>(ps.size()));
    }

    Vector2 Desvio(const std::vector<Vector2>& ps) {
        if (ps.empty()) return Vector2::Zero;
        const Vector2 m = Media(ps);
        float sx = 0.0f, sy = 0.0f;
        for (const Vector2& p : ps) {
            sx += (p.x - m.x) * (p.x - m.x);
            sy += (p.y - m.y) * (p.y - m.y);
        }
        const float n = static_cast<float>(ps.size());
        return Vector2(std::sqrt(sx / n), std::sqrt(sy / n));
    }

    Caixa Limites(const std::vector<Vector2>& ps) {
        if (ps.empty()) return {};
        Caixa c{ps.front(), ps.front()};
        for (const Vector2& p : ps) {
            c.de.x = std::min(c.de.x, p.x);
            c.de.y = std::min(c.de.y, p.y);
            c.ate.x = std::max(c.ate.x, p.x);
            c.ate.y = std::max(c.ate.y, p.y);
        }
        return c;
    }

    std::vector<Vector2> MinMax(const std::vector<Vector2>& ps, const Caixa& destino) {
        const Caixa origem = Limites(ps);
        const float lx = origem.ate.x - origem.de.x, ly = origem.ate.y - origem.de.y;
        std::vector<Vector2> saida;
        saida.reserve(ps.size());
        for (const Vector2& p : ps) {
            const float u = lx > 1e-6f ? (p.x - origem.de.x) / lx : 0.5f;
            const float v = ly > 1e-6f ? (p.y - origem.de.y) / ly : 0.5f;
            saida.emplace_back(destino.de.x + u * (destino.ate.x - destino.de.x),
                               destino.de.y + v * (destino.ate.y - destino.de.y));
        }
        return saida;
    }

    std::vector<Vector2> Padronizar(const std::vector<Vector2>& ps, const Vector2 centro, const Vector2 escala) {
        const Vector2 m = Media(ps), d = Desvio(ps);
        std::vector<Vector2> saida;
        saida.reserve(ps.size());
        for (const Vector2& p : ps) {
            const float zx = d.x > 1e-6f ? (p.x - m.x) / d.x : 0.0f;
            const float zy = d.y > 1e-6f ? (p.y - m.y) / d.y : 0.0f;
            saida.emplace_back(centro.x + zx * escala.x, centro.y + zy * escala.y);
        }
        return saida;
    }

    std::vector<bool> Outliers(const std::vector<Vector2>& ps, const float limite) {
        const Vector2 m = Media(ps), d = Desvio(ps);
        std::vector<bool> saida;
        saida.reserve(ps.size());
        for (const Vector2& p : ps) {
            const float zx = d.x > 1e-6f ? std::fabs(p.x - m.x) / d.x : 0.0f;
            const float zy = d.y > 1e-6f ? std::fabs(p.y - m.y) / d.y : 0.0f;
            saida.push_back(zx > limite || zy > limite);
        }
        return saida;
    }

    Caixa CaixaNoCanto(const int canto, const float lado) {
        const int c = ((canto % 4) + 4) % 4;
        const float l = std::clamp(lado, 0.05f, 1.0f);
        const float x0 = (c % 2 == 0) ? 0.0f : 1.0f - l;
        const float y0 = (c < 2) ? 0.0f : 1.0f - l;
        return {Vector2(x0, y0), Vector2(x0 + l, y0 + l)};
    }

    int CantoDoAluno(const float xNoGrafico) {
        return xNoGrafico < 0.5f ? 0 : 1;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreColetas = 0.09f;
        d.velocidadeDaColeta = 320.0f;
        d.maisDados = 60;
        d.resistencia = 2;
        d.aviso = 0.6f;
        d.transformacao = 0.6f;
        return d;
    }
}
