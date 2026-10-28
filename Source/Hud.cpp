//
// A HUD da batalha: a moldura do campo, o relogio da fase e os efeitos.
//

#include "Hud.h"

#include <algorithm>
#include <cmath>

namespace Hud {

    Caixa AreaDoAluno(const Caixa& campo, const float larguraDoSprite, const float alturaDoSprite) {
        return {campo.esquerda + larguraDoSprite / 2.0f, campo.topo + alturaDoSprite / 2.0f,
                campo.direita - larguraDoSprite / 2.0f, campo.baixo - alturaDoSprite / 2.0f};
    }

    namespace {
        float Perto(const float distancia) {
            return std::clamp(1.0f - distancia / kAvisoDaParede, 0.0f, 1.0f);
        }
    }

    Encosto Encostado(const Caixa& area, const Vector2& centro, const Vector2& meio) {
        Encosto e;
        e.esquerda = Perto(centro.x - meio.x - area.esquerda);
        e.direita = Perto(area.direita - (centro.x + meio.x));
        e.topo = Perto(centro.y - meio.y - area.topo);
        e.baixo = Perto(area.baixo - (centro.y + meio.y));
        return e;
    }

    float Clarao(const float idade, const float duracao) {
        if (idade < 0.0f || duracao <= 0.0f || idade >= duracao) return 0.0f;
        const float u = 1.0f - idade / duracao;
        return u * u;   // some depressa e termina devagar
    }

    Rgb CorDoRelogio(const float fracao) {
        const float f = std::clamp(fracao, 0.0f, 1.0f);
        const auto mistura = [](const Rgb a, const Rgb b, const float t) {
            const auto m = [t](const unsigned char x, const unsigned char y) {
                return static_cast<unsigned char>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
            };
            return Rgb{m(a.r, b.r), m(a.g, b.g), m(a.b, b.b)};
        };
        constexpr Rgb kVerde{150, 230, 110}, kAmbar{255, 190, 30}, kVermelho{255, 70, 60};
        if (f >= 0.5f) return mistura(kAmbar, kVerde, (f - 0.5f) / 0.5f);
        if (f >= 0.2f) return mistura(kVermelho, kAmbar, (f - 0.2f) / 0.3f);
        return kVermelho;
    }

    bool Urgente(const float restante) { return restante > 0.0f && restante <= kUrgente; }

    std::vector<float> Marcas(const float duracao) {
        std::vector<float> m;
        if (duracao <= 0.0f) return m;
        for (float t = kEntreMarcas; t < duracao - 0.5f; t += kEntreMarcas) m.push_back(t / duracao);
        return m;
    }

    float Vida(const Faisca f) {
        switch (f) {
            case Faisca::Acerto:    return 0.16f;
            case Faisca::Derrubada: return 0.28f;
            case Faisca::Dano:      return 0.45f;
            case Faisca::Coleta:    return 0.5f;
        }
        return 0.2f;
    }

    float Raio(const Faisca f, const float idade) {
        const float u = std::clamp(idade / Vida(f), 0.0f, 1.0f);
        const float abre = 1.0f - (1.0f - u) * (1.0f - u);   // rapido no comeco
        switch (f) {
            case Faisca::Acerto:    return 4.0f + 10.0f * abre;
            case Faisca::Derrubada: return 6.0f + 20.0f * abre;
            case Faisca::Dano:      return 14.0f + 46.0f * abre;
            case Faisca::Coleta:    return 10.0f + 40.0f * abre;
        }
        return 10.0f;
    }

    float Brilho(const Faisca f, const float idade) {
        if (idade < 0.0f || idade >= Vida(f)) return 0.0f;
        return 1.0f - idade / Vida(f);
    }
}
