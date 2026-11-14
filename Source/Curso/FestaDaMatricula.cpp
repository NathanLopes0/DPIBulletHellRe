//
// A festa da matricula: o que a selecao de fases desenha quando uma optativa acaba de ser comprada.
//

#include "FestaDaMatricula.h"

#include <algorithm>
#include <cmath>

namespace FestaDaMatricula {

    int Fichas(const int preco) {
        return std::clamp((preco + kHorasPorFicha - 1) / kHorasPorFicha, kMenosFichas, kMaisFichas);
    }

    Vector2 NoArco(const Vector2 de, const Vector2 para, const float altura, float u) {
        u = std::clamp(u, 0.0f, 1.0f);
        const Vector2 reta = de + (para - de) * u;
        // A parabola que sai de 'de', passa 'altura' acima do mais alto (y cresce para baixo) e
        // chega em 'para': a reta, mais a corcova 4u(1-u).
        const float topo = std::min(de.y, para.y) - altura;
        const float meio = 0.5f * (de.y + para.y);
        return Vector2(reta.x, reta.y + (topo - meio) * 4.0f * u * (1.0f - u));
    }

    Ficha FichaEm(const int k, const int n, const float t, const Vector2 de, const Vector2 para) {
        Ficha f;
        const float sai = static_cast<float>(k) * kEntreFichas;
        // Cada uma sai de um lugar da fileira de blocos, centrada em 'de'.
        const float lado = 18.0f * (static_cast<float>(k) - 0.5f * static_cast<float>(std::max(0, n - 1)));
        const Vector2 origem = de + Vector2(lado, 0.0f);
        if (t < sai) {
            f.onde = origem;
            return f;
        }
        const float u = (t - sai) / kVooDaFicha;
        if (u >= 1.0f) {
            f.onde = para;
            return f;
        }
        f.voando = true;
        f.onde = NoArco(origem, para, kAlturaDoArco, u);
        // Duas voltas inteiras no voo: a largura vai de 1 a 0 e volta.
        f.largura = std::fabs(Math::Cos(2.0f * Math::TwoPi * u));
        return f;
    }

    float Carimbo(const int fichas) {
        return static_cast<float>(std::max(1, fichas) - 1) * kEntreFichas + kVooDaFicha;
    }

    float Pulo(const float t) {
        if (t <= 0.0f || t >= kDuracaoDoPulo) return 0.0f;
        // O primeiro pulinho leva 60% do tempo; o segundo, menor, o resto.
        const float corte = 0.6f * kDuracaoDoPulo;
        if (t < corte) return kPrimeiroPulo * Math::Sin(Math::Pi * t / corte);
        return 0.4f * kPrimeiroPulo * Math::Sin(Math::Pi * (t - corte) / (kDuracaoDoPulo - corte));
    }

    Anel AnelEm(const float t, const float raioInicial) {
        Anel a;
        if (t < 0.0f || t >= kVidaDoAnel) return a;
        const float u = t / kVidaDoAnel;
        // Abre rapido e desacelera; some no fim.
        a.raio = raioInicial + 110.0f * (1.0f - (1.0f - u) * (1.0f - u));
        a.opacidade = 1.0f - u;
        return a;
    }

    std::vector<Faisca> Faiscas(const Vector2 centro, const float t, const int quantas) {
        std::vector<Faisca> fs;
        if (t < 0.0f || t >= kVidaDaFaisca || quantas <= 0) return fs;
        fs.reserve(static_cast<size_t>(quantas));
        for (int k = 0; k < quantas; ++k) {
            // Para cima e para os lados, cada uma com a sua rapidez (sem sorteio: a mesma festa sempre).
            const float a = Math::Pi * (1.12f + 0.76f * static_cast<float>(k) / static_cast<float>(std::max(1, quantas - 1)));
            const float rapidez = 210.0f + 90.0f * static_cast<float>((k * 7) % 5) / 4.0f;
            const Vector2 v(Math::Cos(a) * rapidez, Math::Sin(a) * rapidez);
            Faisca f;
            f.onde = centro + v * t + Vector2(0.0f, 0.5f * kGravidade * t * t);
            f.opacidade = 1.0f - t / kVidaDaFaisca;
            fs.push_back(f);
        }
        return fs;
    }

    float Duracao(const int fichas) {
        return Carimbo(fichas) + std::max({kDuracaoDoPulo, kVidaDoAnel, kVidaDaFaisca});
    }
}
