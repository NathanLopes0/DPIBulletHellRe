//
// A revelacao da materia secreta.
//

#include "Revelacao.h"

#include <algorithm>
#include <cmath>

namespace {

    float Limitar(const float x, const float de, const float ate) {
        return std::max(de, std::min(ate, x));
    }

    /// 0 a 1, suave nas pontas.
    float Suave(const float x) {
        const float u = Limitar(x, 0.0f, 1.0f);
        return u * u * (3.0f - 2.0f * u);
    }

    /// Um numero de 0 a 1 que so depende de 'k' e de 'semente': cada faisca tem o seu, e o mesmo
    /// instante desenha sempre igual.
    float Sorteio(const int k, const int semente) {
        const float x = std::sin(static_cast<float>(k) * 12.9898f + static_cast<float>(semente) * 78.233f) * 43758.5453f;
        return x - std::floor(x);
    }

    constexpr float kVoltasDaCarga = 2.2f;
}

namespace Revelacao {

    float Veu(const float t, const float desdeOBotao) {
        const float chegando = kVeuCheio * Suave(t / kEscuro);
        if (desdeOBotao < 0.0f) return chegando;
        return chegando * (1.0f - Suave(desdeOBotao / kIda));
    }

    std::vector<Pixel> Carga(const Vector2 centro, const float t, const int quantos) {
        std::vector<Pixel> pixels;
        if (t < kComecaACarga || t >= kEstouro || quantos <= 0) return pixels;
        const float duracao = kEstouro - kComecaACarga;
        pixels.reserve(static_cast<size_t>(quantos));
        for (int k = 0; k < quantos; ++k) {
            // Cada um parte um pouco depois, e todos chegam no estouro.
            const float parte = kComecaACarga + 0.45f * duracao * Sorteio(k, 1);
            if (t < parte) continue;
            const float u = (t - parte) / (kEstouro - parte);   // 0 a 1
            const float perto = u * u * u;                      // acelera no fim
            const float raio = kRaioDaCarga * (0.65f + 0.35f * Sorteio(k, 2)) * (1.0f - perto);
            const float angulo = Math::TwoPi * (Sorteio(k, 3) + kVoltasDaCarga * perto);
            Pixel p;
            p.onde = centro + Vector2(std::cos(angulo), std::sin(angulo)) * raio;
            p.lado = 3.0f + 3.0f * Sorteio(k, 4) + 3.0f * perto;
            p.opacidade = Limitar(u * 4.0f, 0.0f, 1.0f);
            p.dourado = k % 2 == 0;
            pixels.push_back(p);
        }
        return pixels;
    }

    Contorno ContornoDaCarga(const float t) {
        Contorno c;
        if (t < kComecaACarga || t >= kEstouro) return c;
        const float u = (t - kComecaACarga) / (kEstouro - kComecaACarga);
        // Pisca cada vez mais rapido: de 3 a 14 vezes por segundo.
        const float fase = (3.0f * u + 5.5f * u * u) * (kEstouro - kComecaACarga);
        c.raio = 46.0f + 26.0f * u;
        c.opacidade = (0.25f + 0.6f * u) * (0.5f + 0.5f * std::cos(Math::TwoPi * fase));
        return c;
    }

    float Tremor(const float t) {
        if (t < kComecaACarga) return 0.0f;
        if (t < kEstouro) return 3.0f * Suave((t - kComecaACarga) / (kEstouro - kComecaACarga));
        const float d = t - kEstouro;
        return d < 0.7f ? 12.0f * (1.0f - d / 0.7f) * (1.0f - d / 0.7f) : 0.0f;
    }

    Vector2 Sacudida(const float t) {
        const float a = Tremor(t);
        if (a <= 0.0f) return Vector2::Zero;
        return Vector2(std::sin(t * 71.0f) + 0.5f * std::sin(t * 113.0f),
                       std::cos(t * 59.0f) + 0.5f * std::sin(t * 97.0f)) * (a / 1.5f);
    }

    float Clarao(const float t) {
        const float d = t - kEstouro;
        if (d < 0.0f || d > 0.45f) return 0.0f;
        return 1.0f - d / 0.45f;
    }

    std::vector<Onda> Ondas(const float t) {
        std::vector<Onda> ondas;
        for (int k = 0; k < kOndas; ++k) {
            const float d = t - kEstouro - 0.14f * static_cast<float>(k);
            if (d < 0.0f || d > kVidaDaOnda) continue;
            const float u = d / kVidaDaOnda;
            Onda o;
            o.raio = 30.0f + kRaioDaOnda * (1.0f - (1.0f - u) * (1.0f - u));
            o.opacidade = 1.0f - u;
            o.cor = k;
            ondas.push_back(o);
        }
        return ondas;
    }

    std::vector<Faisca> Faiscas(const Vector2 centro, const float t, const int quantas) {
        std::vector<Faisca> faiscas;
        const float d = t - kEstouro;
        if (d < 0.0f || d > kVidaDaFaisca || quantas <= 0) return faiscas;
        faiscas.reserve(static_cast<size_t>(quantas));
        for (int k = 0; k < quantas; ++k) {
            const float vida = kVidaDaFaisca * (0.55f + 0.45f * Sorteio(k, 5));
            if (d > vida) continue;
            const float angulo = Math::TwoPi * Sorteio(k, 6);
            const float rapidez = 260.0f + 520.0f * Sorteio(k, 7);
            Faisca f;
            f.onde = centro + Vector2(std::cos(angulo), std::sin(angulo)) * (rapidez * d) +
                     Vector2(0.0f, 0.5f * kGravidade * d * d);
            f.opacidade = 1.0f - d / vida;
            f.lado = 2.0f + 3.0f * Sorteio(k, 8);
            f.cor = k % 3;
            faiscas.push_back(f);
        }
        return faiscas;
    }

    float Escala(const float t) {
        const float d = t - kEstouro;
        if (d < 0.0f) return 0.0f;
        if (d < 0.18f) return kEscalaGrande * 1.3f * (d / 0.18f);                     // cresce de uma vez
        if (d < 0.45f) return kEscalaGrande * (1.3f - 0.3f * Suave((d - 0.18f) / 0.27f));   // e volta
        return kEscalaGrande * (1.0f + 0.03f * std::sin((d - 0.45f) * 3.0f));          // respira
    }

    Contorno Halo(const float t) {
        Contorno c;
        const float d = t - kEstouro - 0.5f;
        if (d < 0.0f) return c;
        const float u = std::fmod(d, kEntreHalos) / kEntreHalos;
        // De fora do losango grande (a meia largura dele e de uns 190 px) para longe.
        c.raio = 205.0f + 140.0f * u;
        c.opacidade = 0.8f * (1.0f - u);
        return c;
    }

    float DescidaDoTitulo(const float t) {
        return Suave((t - kTitulo) / 0.5f);
    }

    float Nome(const float t) {
        return Suave((t - kNome) / 0.5f);
    }

    bool Aviso(const float t) {
        return t >= kPodeSeguir && std::fmod(t - kPodeSeguir, 0.9f) < 0.6f;
    }

    Ida IdaEm(const Vector2 meio, const Vector2 lugar, const float desdeOBotao) {
        Ida i;
        const float u = Suave(desdeOBotao / kIda);
        // Um arco: sobe um pouco antes de descer no lugar.
        const Vector2 reta = meio + (lugar - meio) * u;
        i.onde = reta - Vector2(0.0f, 120.0f * std::sin(Math::Pi * u));
        i.escala = kEscalaGrande + (1.0f - kEscalaGrande) * u;
        i.chegou = desdeOBotao >= kIda;
        return i;
    }

    std::string Parabens(const int materiasDoCurso) {
        return "Parabéns! Você achou a " + std::to_string(materiasDoCurso) + "ª matéria.";
    }
}
