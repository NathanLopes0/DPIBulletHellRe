//
// A fase tutorial (INF 191).
//

#include "Tutorial.h"

#include <algorithm>
#include <cmath>

namespace Tutorial {

    Etapa EtapaEm(const float t) {
        float fim = 0.0f;
        for (std::size_t k = 0; k < kEtapas; ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<Etapa>(k);
        }
        return Etapa::Fim;
    }

    float ComecoDa(const Etapa e) {
        float comeco = 0.0f;
        for (std::size_t k = 0; k < static_cast<std::size_t>(e) && k < kEtapas; ++k) comeco += kDuracoes[k];
        return comeco;
    }

    float NaEtapa(const float t) {
        return std::max(0.0f, t - ComecoDa(EtapaEm(t)));
    }

    bool PodeAvancar(const Etapa e, const Desempenho& d) {
        if (d.naEtapa < kMinimoNaEtapa) return false;
        const bool desviou = d.semApanhar >= kSemApanharParaPassar;
        const bool atirou = d.acertosDados >= kAcertosParaPassar;
        switch (e) {
            case Etapa::Desviar:    return desviou;
            case Etapa::Atirar:     return atirou;
            case Etapa::SobeEDesce: return desviou && atirou;
            case Etapa::Foco:       return desviou;
            case Etapa::PontoExtra: return d.gastouPontoExtra;
            case Etapa::Chegada:
            case Etapa::Fim:        return false;
        }
        return false;
    }

    const char* GatilhoDe(const Etapa e) {
        switch (e) {
            case Etapa::Desviar:    return "chuva";
            case Etapa::SobeEDesce: return "mirado";
            case Etapa::Foco:       return "anel";
            case Etapa::PontoExtra: return "cheia";
            default:                return nullptr;
        }
    }

    float CadenciaDe(const Etapa e) {
        switch (e) {
            case Etapa::Desviar:    return 1.2f;
            case Etapa::SobeEDesce: return 1.1f;
            case Etapa::Foco:       return 1.5f;
            case Etapa::PontoExtra: return 0.9f;
            default:                return 0.0f;
        }
    }

    bool Acabou(const float t) {
        return t >= ComecoDa(Etapa::Fim) + kAplausos;
    }

    Sinal SinalDaNota(const float antes, const float agora) {
        constexpr float kFolga = 0.01f;
        if (agora > antes + kFolga) return Sinal::Subiu;
        if (agora < antes - kFolga) return Sinal::Desceu;
        return Sinal::Nada;
    }

    Vector2 NoVoo(const Vector2 de, const Vector2 ate, const float idade) {
        const float u = std::clamp(idade / kVooDoSinal, 0.0f, 1.0f);
        const float suave = 1.0f - (1.0f - u) * (1.0f - u);
        return de + (ate - de) * suave;
    }

    Vector2 LadoDoManche(const float t) {
        static const Vector2 kLados[4] = {Vector2(-1.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, -1.0f),
                                          Vector2(0.0f, 1.0f)};
        const int k = static_cast<int>(std::floor(std::max(0.0f, t) / kPorLado)) % 4;
        return kLados[k];
    }

    bool BotaoApertado(const float t) {
        return std::fmod(std::max(0.0f, t), kToque) < kToque / 2.0f;
    }
}
