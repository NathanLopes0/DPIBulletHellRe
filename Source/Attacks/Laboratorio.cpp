//
// A repescagem do Michel: o MaVIlab - segmentacao, dados no tempo e deteccao.
//

#include "Laboratorio.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace Laboratorio {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    bool Mascara::Em(const int c, const int l) const {
        if (c < 0 || l < 0 || c >= n || l >= n) return false;
        return cel[static_cast<size_t>(l * n + c)];
    }

    int Mascara::Quantas() const {
        return static_cast<int>(std::count(cel.begin(), cel.end(), true));
    }

    std::pair<int, int> CelulaDe(const Vector2 normalizado, const int n) {
        if (n <= 0) return {0, 0};
        const auto um = [n](const float u) {
            return std::clamp(static_cast<int>(std::floor(u * static_cast<float>(n))), 0, n - 1);
        };
        return {um(normalizado.x), um(normalizado.y)};
    }

    Mascara Segmentar(const int n, const std::vector<Vector2>& alunos, const std::vector<Vector2>& atalhos,
                      const int dilatacao) {
        Mascara m;
        m.n = std::max(0, n);
        m.cel.assign(static_cast<size_t>(m.n * m.n), false);
        const auto marcar = [&m, dilatacao](const Vector2 p) {
            const auto [c0, l0] = CelulaDe(p, m.n);
            for (int l = l0 - dilatacao; l <= l0 + dilatacao; ++l) {
                for (int c = c0 - dilatacao; c <= c0 + dilatacao; ++c) {
                    if (c >= 0 && l >= 0 && c < m.n && l < m.n) m.cel[static_cast<size_t>(l * m.n + c)] = true;
                }
            }
        };
        for (const Vector2& p : alunos) marcar(p);
        for (const Vector2& p : atalhos) marcar(p);
        return m;
    }

    std::vector<Vector2> Pixels(const Vector2 de, const Vector2 ate, const float espacamento) {
        std::vector<Vector2> ps;
        if (espacamento <= 0.0f) return ps;
        const float w = ate.x - de.x, h = ate.y - de.y;
        if (w <= 0.0f || h <= 0.0f) return ps;
        const int nx = std::max(1, static_cast<int>(std::floor(w / espacamento)));
        const int ny = std::max(1, static_cast<int>(std::floor(h / espacamento)));
        // A sobra divide-se nas duas bordas: a grade fica no meio do retangulo.
        const float x0 = de.x + 0.5f * (w - static_cast<float>(nx - 1) * espacamento);
        const float y0 = de.y + 0.5f * (h - static_cast<float>(ny - 1) * espacamento);
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                ps.emplace_back(x0 + static_cast<float>(i) * espacamento, y0 + static_cast<float>(j) * espacamento);
            }
        }
        return ps;
    }

    void Lembrar(Memoria& m, const Vector2 onde, const float dt, const float esquecimento) {
        if (!m.comecou || dt <= 0.0f) {
            m.onde = onde;
            m.comecou = true;
            return;
        }
        const Vector2 agora = (onde - m.onde) * (1.0f / dt);
        const float u = 1.0f - std::exp(-esquecimento * dt);
        // O giro de agora: o angulo entre a velocidade do passo anterior e a deste, por segundo.
        // Parado, nao gira.
        float giroAgora = 0.0f;
        if (agora.LengthSq() > 400.0f && m.ultima.LengthSq() > 400.0f) {
            const float cruz = m.ultima.x * agora.y - m.ultima.y * agora.x;
            const float ponto = Vector2::Dot(m.ultima, agora);
            giroAgora = Math::Atan2(cruz, ponto) / dt;
        }
        m.velocidade = m.velocidade + (agora - m.velocidade) * u;
        m.giro = std::clamp(m.giro + (giroAgora - m.giro) * u, -kGiroMaximo, kGiroMaximo);
        m.ultima = agora;
        m.onde = onde;
    }

    Vector2 Prever(const Memoria& m, const float daqui, const bool comGiro) {
        if (daqui <= 0.0f) return m.onde;
        if (!comGiro || std::fabs(m.giro) < 1e-4f) return m.onde + m.velocidade * daqui;
        // Com giro, a velocidade roda enquanto anda: um arco de circulo. A direcao e a do ultimo passo
        // (a lembrada fica para tras na curva); a rapidez, a lembrada.
        const float a = m.giro * daqui;
        const float s = Math::Sin(a), c = Math::Cos(a);
        const float rapidez = m.velocidade.Length(), ultima = m.ultima.Length();
        const Vector2 v = ultima > 1e-3f ? m.ultima * (rapidez / ultima) : m.velocidade;
        // A integral de R(giro * t) v de 0 a daqui.
        const float k = 1.0f / m.giro;
        const Vector2 deslocamento((v.x * s - v.y * (1.0f - c)) * k, (v.y * s + v.x * (1.0f - c)) * k);
        return m.onde + deslocamento;
    }

    float Encontro(const Memoria& m, const Vector2 origem, const float rapidez, const bool comGiro) {
        if (rapidez <= 0.0f) return 0.0f;
        float t = (m.onde - origem).Length() / rapidez;
        for (int k = 0; k < 4; ++k) t = (Prever(m, t, comGiro) - origem).Length() / rapidez;
        return std::min(t, 2.0f);
    }

    Caixa CaixaEm(const Vector2 centro, const float lado) {
        const Vector2 meio(0.5f * lado, 0.5f * lado);
        return Caixa{centro - meio, centro + meio};
    }

    float Iou(const Caixa& a, const Caixa& b) {
        const float ix = std::max(0.0f, std::min(a.ate.x, b.ate.x) - std::max(a.de.x, b.de.x));
        const float iy = std::max(0.0f, std::min(a.ate.y, b.ate.y) - std::max(a.de.y, b.de.y));
        const float inter = ix * iy;
        const float areaA = (a.ate.x - a.de.x) * (a.ate.y - a.de.y);
        const float areaB = (b.ate.x - b.de.x) * (b.ate.y - b.de.y);
        const float uniao = areaA + areaB - inter;
        return uniao > 0.0f ? inter / uniao : 0.0f;
    }

    std::vector<int> Nms(const std::vector<Caixa>& caixas, const std::vector<float>& confiancas, const float iouMaximo,
                         const int quantas) {
        std::vector<int> ordem(std::min(caixas.size(), confiancas.size()));
        std::iota(ordem.begin(), ordem.end(), 0);
        std::stable_sort(ordem.begin(), ordem.end(), [&confiancas](const int a, const int b) {
            return confiancas[static_cast<size_t>(a)] > confiancas[static_cast<size_t>(b)];
        });
        std::vector<int> escolhidas;
        for (const int i : ordem) {
            if (static_cast<int>(escolhidas.size()) >= quantas) break;
            bool cobre = false;
            for (const int j : escolhidas) {
                if (Iou(caixas[static_cast<size_t>(i)], caixas[static_cast<size_t>(j)]) > iouMaximo) {
                    cobre = true;
                    break;
                }
            }
            if (!cobre) escolhidas.push_back(i);
        }
        return escolhidas;
    }

    float Confianca(const float anterior, const float movimento, const float tirosPorSegundo, const float dt) {
        const float alvo = std::clamp(kConfiancaBase + 0.55f * std::min(1.0f, movimento / 260.0f) +
                                          0.4f * std::min(1.0f, tirosPorSegundo / 5.0f),
                                      0.0f, 0.99f);
        const float u = 1.0f - std::exp(-2.4f * std::max(0.0f, dt));
        return anterior + (alvo - anterior) * u;
    }

    std::string Colega(const int k) {
        static const std::array<const char*, kColegas> nomes = {"Doutoranda", "Mestrando", "P\xc3\xb3" "s-doc", "IC",
                                                                "Mestranda"};
        return nomes[static_cast<size_t>(((k % kColegas) + kColegas) % kColegas)];
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.vidaDoPixel = 0.9f;
        d.entrePrevisoes = 0.42f;
        d.tirosDaPrevisao = 4;
        d.velocidadeDaPrevisao = 265.0f;
        d.esquecimento = 4.0f;
        d.rodadaDoDetector = 1.9f;
        d.procura = 1.1f;
        d.avisoDoCerco = 0.48f;
        d.tirosDoCerco = 22;
        d.rastreio = 1.4f;
        d.entreRastreios = 0.24f;
        d.entreRuidos = 0.4f;
        return d;
    }
}
