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

    Rgb CorDaAreia(const float fracao) {
        const float f = std::clamp(fracao, 0.0f, 1.0f);
        constexpr Rgb kAreia{236, 204, 128}, kQuente{255, 150, 70};
        if (f >= 0.25f) return kAreia;
        const float t = 1.0f - f / 0.25f;
        const auto m = [t](const unsigned char x, const unsigned char y) {
            return static_cast<unsigned char>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
        };
        return Rgb{m(kAreia.r, kQuente.r), m(kAreia.g, kQuente.g), m(kAreia.b, kQuente.b)};
    }

    float MeiaLarguraDoVidro(const float v) {
        const float d = std::fabs(2.0f * std::clamp(v, 0.0f, 1.0f) - 1.0f);   // 0 no gargalo, 1 nas pontas
        return std::max(kGargalo, std::sqrt(d));
    }

    namespace {
        /// A area de um bulbo do gargalo ate 'a' (0 a 1 da altura dele), numa soma de fatias.
        float AreaAte(const float a) {
            constexpr int kFatias = 64;
            float soma = 0.0f;
            const float topo = std::clamp(a, 0.0f, 1.0f);
            for (int i = 0; i < kFatias; ++i) {
                const float d = (static_cast<float>(i) + 0.5f) / kFatias * topo;
                soma += std::max(kGargalo, std::sqrt(d)) * topo / kFatias;
            }
            return soma;
        }
        /// A altura (do gargalo para fora) que guarda 'parte' da area do bulbo.
        float AlturaComArea(const float parte) {
            const float alvo = std::clamp(parte, 0.0f, 1.0f) * AreaAte(1.0f);
            float de = 0.0f, ate = 1.0f;
            for (int k = 0; k < 24; ++k) {
                const float meio = (de + ate) / 2.0f;
                (AreaAte(meio) < alvo ? de : ate) = meio;
            }
            return (de + ate) / 2.0f;
        }
    }

    float AreiaDeCima(const float fracao) { return AlturaComArea(fracao); }
    float AreiaDeBaixo(const float fracao) {
        // O monte sobe do fundo (a ponta larga): e o que sobra do bulbo acima da areia que falta cair.
        return 1.0f - AlturaComArea(std::clamp(fracao, 0.0f, 1.0f));
    }

    float EscalaDaVirada(const float desde) {
        if (desde < 0.0f || desde >= kVirada) return 1.0f;
        return std::fabs(std::cos(3.14159265f * desde / kVirada));
    }

    bool JaVirou(const float desde) { return desde >= kVirada * 0.5f; }

    bool Urgente(const float restante) { return restante > 0.0f && restante <= kUrgente; }

    float Vida(const Faisca f) {
        switch (f) {
            case Faisca::Acerto:    return 0.16f;
            case Faisca::Derrubada: return 0.28f;
            case Faisca::Dano:      return 0.45f;
            case Faisca::Coleta:    return 0.5f;
            case Faisca::Dissolvido: return 0.35f;
            case Faisca::Resposta:
            case Faisca::RespostaDePerto: return 0.45f;
            case Faisca::Raspao:    return 0.25f;
            case Faisca::Cobertura: return 0.4f;
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
            case Faisca::Dissolvido: return 3.0f + 13.0f * abre;
            case Faisca::Resposta:          // o quanto o "!" ja subiu
            case Faisca::RespostaDePerto: return 2.0f + 16.0f * abre;
            case Faisca::Raspao:    return 5.0f + 11.0f * abre;
            case Faisca::Cobertura: return 8.0f + 24.0f * abre;
        }
        return 10.0f;
    }

    float Brilho(const Faisca f, const float idade) {
        if (idade < 0.0f || idade >= Vida(f)) return 0.0f;
        return 1.0f - idade / Vida(f);
    }

    Rgb CorDoAluno(const std::size_t quem) {
        if (quem == 0) return {110, 190, 255};
        if (quem == 1) return {255, 160, 80};
        return {255, 255, 255};
    }

    Rgb ArcoIris(const float matiz) {
        const float h = (matiz - std::floor(matiz)) * 6.0f;
        // Saturado mas nao puro: o branco do campo escuro nao estoura.
        constexpr float kAlto = 255.0f, kBaixo = 60.0f;
        const auto canal = [&](const float centro) {
            float d = std::fabs(h - centro);
            d = std::min(d, 6.0f - d);
            const float u = std::clamp(2.0f - d, 0.0f, 1.0f);
            return static_cast<unsigned char>(kBaixo + (kAlto - kBaixo) * u);
        };
        return {canal(0.0f), canal(2.0f), canal(4.0f)};
    }

    float Murcho(const float idade) {
        const float u = std::clamp(idade / kMurcha, 0.0f, 1.0f);
        return 1.0f - 0.7f * (1.0f - (1.0f - u) * (1.0f - u));
    }

    std::uint8_t AlfaDoBrilho(const float segundos) {
        const float pulso = 0.5f + 0.5f * Math::Sin(segundos * kPulsoDoBrilho * Math::TwoPi);
        return static_cast<std::uint8_t>(static_cast<float>(kBrilhoMinimo) +
                                         static_cast<float>(kBrilhoMaximo - kBrilhoMinimo) * pulso);
    }
}
