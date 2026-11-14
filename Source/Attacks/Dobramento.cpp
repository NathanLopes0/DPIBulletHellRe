//
// A fase 3 da Sabrina: a proteina - a cadeia, o dobramento e o encaixe.
//

#include "Dobramento.h"

#include <algorithm>
#include <cmath>

namespace Dobramento {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    void Arrastar(std::vector<Vector2>& contas, const Vector2 cabeca, const float espacamento) {
        if (contas.empty()) return;
        contas[0] = cabeca;
        for (size_t i = 1; i < contas.size(); ++i) {
            const Vector2 d = contas[i] - contas[i - 1];
            const float l = d.Length();
            // Duas contas no mesmo lugar: a de tras fica embaixo da da frente.
            contas[i] = l > 1e-4f ? contas[i - 1] + d * (espacamento / l) : contas[i - 1] + Vector2(0.0f, espacamento);
        }
    }

    std::vector<Vector2> Cortar(std::vector<Vector2>& contas, const int i) {
        if (i < 0 || i >= static_cast<int>(contas.size())) return {};
        std::vector<Vector2> soltos(contas.begin() + i, contas.end());
        contas.resize(static_cast<size_t>(i));
        return soltos;
    }

    std::vector<Helice::Ponto3> Globulo(const std::vector<bool>& hidrofobico, const float nucleo) {
        // Os dois grupos, cada um numa espiral de Fibonacci (os pontos bem espalhados numa esfera).
        int dentro = 0;
        for (const bool h : hidrofobico) if (h) ++dentro;
        const int fora = static_cast<int>(hidrofobico.size()) - dentro;
        const auto naEsfera = [](const int i, const int n, const float raio) {
            if (n <= 1) return Helice::Ponto3{};
            const float y = 1.0f - 2.0f * (static_cast<float>(i) + 0.5f) / static_cast<float>(n);
            const float r = std::sqrt(std::max(0.0f, 1.0f - y * y));
            const float a = 2.39996323f * static_cast<float>(i);   // o angulo de ouro
            return Helice::Ponto3{raio * r * Math::Cos(a), raio * y, raio * r * Math::Sin(a)};
        };
        std::vector<Helice::Ponto3> pontos;
        pontos.reserve(hidrofobico.size());
        int iDentro = 0, iFora = 0;
        for (const bool h : hidrofobico) {
            if (h) pontos.push_back(naEsfera(iDentro++, dentro, nucleo));
            else pontos.push_back(naEsfera(iFora++, fora, 1.0f));
        }
        return pontos;
    }

    Vector2 Quicar(Vector2 onde, Vector2& velocidade, const float dt, const float raio, const Vector2 deCanto,
                   const Vector2 ateCanto) {
        onde += velocidade * dt;
        const float esquerda = deCanto.x + raio, direita = ateCanto.x - raio;
        const float topo = deCanto.y + raio, chao = ateCanto.y - raio;
        if (onde.x < esquerda) {
            onde.x = esquerda;
            velocidade.x = std::fabs(velocidade.x);
        } else if (onde.x > direita) {
            onde.x = direita;
            velocidade.x = -std::fabs(velocidade.x);
        }
        if (onde.y < topo) {
            onde.y = topo;
            velocidade.y = std::fabs(velocidade.y);
        } else if (onde.y > chao) {
            onde.y = chao;
            velocidade.y = -std::fabs(velocidade.y);
        }
        return onde;
    }

    std::vector<Vector2> Sitio(const Vector2 centro, const float raio, const int n, const float boca,
                               const float abertura) {
        std::vector<Vector2> contas;
        if (n <= 0) return contas;
        contas.reserve(static_cast<size_t>(n));
        // O arco vai da beirada de um lado da boca ate a do outro, pelo lado de tras.
        const float de = boca + 0.5f * abertura;
        const float arco = Math::TwoPi - abertura;
        for (int k = 0; k < n; ++k) {
            const float u = n > 1 ? static_cast<float>(k) / static_cast<float>(n - 1) : 0.5f;
            const float a = de + arco * u;
            contas.push_back(centro + Vector2(Math::Cos(a), Math::Sin(a)) * raio);
        }
        return contas;
    }

    bool Encaixado(const Vector2 p, const Vector2 centro, const float raio, const float folga) {
        return (p - centro).Length() < raio - folga;
    }

    float DeltaG(const float segundos) {
        return -std::min(15.0f, 2.0f + 1.6f * std::max(0.0f, segundos));
    }

    Vector2 Browniano(const Vector2 velocidade, const Vector2 media, const Vector2 chute, const float dt,
                      const float retorno) {
        return velocidade + chute * dt + (media - velocidade) * std::clamp(retorno * dt, 0.0f, 1.0f);
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.residuosIniciais = 12;
        d.residuosMaximos = 34;
        d.entreResiduos = 0.2f;
        d.rapidez = 235.0f;
        d.giro = 3.3f;
        d.resistencia = 2;
        d.voltaDaCadeia = 0.5f;
        d.entreLeques = 0.85f;
        d.tirosDoLeque = 6;
        d.velocidadeDoLeque = 220.0f;
        d.residuosDoGlobulo = 30;
        d.raioDoGlobulo = 128.0f;
        d.giroY = 1.35f;
        d.giroX = 0.7f;
        d.limiar = 0.08f;
        d.rapidezDoGlobulo = 175.0f;
        d.perseguicao = 1.3f;
        d.entreBorrifos = 0.42f;
        d.velocidadeDoBorrifo = 220.0f;
        d.tirosDoBorrifo = 3;
        d.abertura = 1.25f;
        d.giroDaBoca = 0.95f;
        d.entreGotas = 0.034f;
        d.velocidadeDaGota = 170.0f;
        d.chute = 1000.0f;
        d.entreInibidores = 0.55f;
        d.tirosDoInibidor = 5;
        d.velocidadeDoInibidor = 245.0f;
        d.premioPorSegundo = 0.7f;
        return d;
    }
}
