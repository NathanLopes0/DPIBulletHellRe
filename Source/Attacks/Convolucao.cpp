//
// A fase 2 do Michel: do descritor feito a mao as deep features.
//

#include "Convolucao.h"

#include <algorithm>
#include <cmath>

namespace Convolucao {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    float Grade::Em(const int c, const int l) const {
        if (!Dentro(c, l)) return 0.0f;
        return v[static_cast<size_t>(l * colunas + c)];
    }

    float& Grade::Celula(const int c, const int l) {
        return v[static_cast<size_t>(std::clamp(l, 0, linhas - 1) * colunas + std::clamp(c, 0, colunas - 1))];
    }

    Grade Vazia(const int colunas, const int linhas) {
        Grade g;
        g.colunas = std::max(0, colunas);
        g.linhas = std::max(0, linhas);
        g.v.assign(static_cast<size_t>(g.colunas * g.linhas), 0.0f);
        return g;
    }

    void Pintar(Grade& g, const int c, const int l, const float quanto, const float teto) {
        if (!g.Dentro(c, l)) return;
        float& v = g.Celula(c, l);
        v = std::min(teto, v + quanto);
    }

    void Esmaecer(Grade& g, const float dt, const float meiaVida) {
        if (meiaVida <= 0.0f) {
            std::fill(g.v.begin(), g.v.end(), 0.0f);
            return;
        }
        const float fator = std::pow(0.5f, dt / meiaVida);
        for (float& v : g.v) v *= fator;
    }

    int Bin(const Vector2 direcao) {
        if (direcao.LengthSq() < 1e-12f) return 0;
        const float graus = Math::ToDegrees(Math::Atan2(direcao.y, direcao.x));
        const int b = static_cast<int>(std::lround(graus / 45.0f));
        return ((b % kBins) + kBins) % kBins;
    }

    Vector2 DirecaoDoBin(const int bin) {
        const float a = Math::ToRadians(45.0f * static_cast<float>(((bin % kBins) + kBins) % kBins));
        return Vector2(Math::Cos(a), Math::Sin(a));
    }

    int BinDaCelula(const Vector2 centro, const Vector2 aluno, const int giro) {
        return (((Bin(aluno - centro) + giro) % kBins) + kBins) % kBins;
    }

    float Resposta(const Grade& g, const int c, const int l, const Filtro& f) {
        float soma = 0.0f;
        for (int dl = -1; dl <= 1; ++dl) {
            for (int dc = -1; dc <= 1; ++dc) {
                soma += f[static_cast<size_t>((dl + 1) * 3 + (dc + 1))] * g.Em(c + dc, l + dl);
            }
        }
        return soma;
    }

    float Relu(const float x, const float limiar) {
        return std::max(0.0f, x - limiar);
    }

    std::pair<int, int> Janela(const int passo, const int colunas, const int linhas) {
        const int total = colunas * linhas;
        if (total <= 0) return {0, 0};
        const int p = ((passo % total) + total) % total;
        return {p % colunas, p / colunas};
    }

    Grade MaxPool(const Grade& g) {
        Grade p = Vazia((g.colunas + 1) / 2, (g.linhas + 1) / 2);
        for (int l = 0; l < g.linhas; ++l) {
            for (int c = 0; c < g.colunas; ++c) {
                float& m = p.Celula(c / 2, l / 2);
                m = std::max(m, g.Em(c, l));
            }
        }
        return p;
    }

    float Global(const Grade& g) {
        if (g.v.empty()) return 0.0f;
        float soma = 0.0f;
        for (const float v : g.v) soma += v;
        return soma / static_cast<float>(g.v.size());
    }

    int TirosDaAtivacao(const float a, const int minimo, const int maximo, const float cheia) {
        if (a <= 0.0f) return 0;
        const float u = cheia > 0.0f ? std::clamp(a / cheia, 0.0f, 1.0f) : 1.0f;
        return minimo + static_cast<int>(std::lround(u * static_cast<float>(maximo - minimo)));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreFluxos = 0.13f;
        d.velocidadeDoFluxo = 140.0f;
        d.atualizacaoDoDescritor = 0.65f;
        d.brilhoPorSegundo = 1.5f;
        d.meiaVida = 1.3f;
        d.limiar = 0.15f;
        d.passosPorSegundo = 9.5f;
        d.passosDoPooling = 3.2f;
        d.velocidadeDaAtivacao = 185.0f;
        d.avisoDaAtivacao = 0.28f;
        d.tirosMaximos = 10;
        d.entreLeques = 1.05f;
        d.tirosDoLeque = 6;
        d.tirosDoVetor = 44;
        return d;
    }
}
