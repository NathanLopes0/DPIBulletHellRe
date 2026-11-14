//
// A fase 1 do Michel: a visao computacional - como a imagem se forma.
//

#include "CameraEscura.h"

#include <algorithm>
#include <cmath>

namespace CameraEscura {

    int EtapaNoTempo(const float t) {
        float fim = 0.0f;
        for (size_t k = 0; k < kDuracoes.size(); ++k) {
            fim += kDuracoes[k];
            if (t < fim) return static_cast<int>(k);
        }
        return static_cast<int>(kDuracoes.size()) - 1;
    }

    Vector2 Imagem(const Vector2 objeto, const Vector2 furo, const float aumento) {
        return furo - (objeto - furo) * aumento;
    }

    Vector2 ObjetoDa(const Vector2 imagem, const Vector2 furo, const float aumento) {
        if (aumento <= 1e-6f) return furo;
        return furo - (imagem - furo) * (1.0f / aumento);
    }

    float AumentoNoTempo(const float t, const float periodo, const float menor, const float maior) {
        if (periodo <= 0.0f) return menor;
        // Comeca no menor e sobe: o cosseno de cabeca para baixo.
        const float u = 0.5f - 0.5f * Math::Cos(Math::TwoPi * t / periodo);
        return menor + (maior - menor) * u;
    }

    float RaioDoBorrao(const float y, const float foco, const float ganho, const float maximo) {
        return std::min(maximo, std::fabs(y - foco) * ganho);
    }

    float Focar(const float foco, const float alvo, const float dt, const float rapidez) {
        const float u = 1.0f - std::exp(-rapidez * dt);
        return foco + (alvo - foco) * u;
    }

    std::vector<Vector2> Borrao(const Vector2 centro, const float raio, const int amostras, const float giro) {
        std::vector<Vector2> pontas;
        if (amostras <= 0) return pontas;
        pontas.reserve(static_cast<size_t>(amostras));
        for (int k = 0; k < amostras; ++k) {
            const float a = giro + Math::TwoPi * static_cast<float>(k) / static_cast<float>(amostras);
            pontas.push_back(centro + Vector2(Math::Cos(a), Math::Sin(a)) * raio);
        }
        return pontas;
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.entreArremessos = 0.56f;
        d.leque = 3;
        d.aberturaDoLeque = 30.0f;
        d.velocidadeDoObjeto = 125.0f;
        d.aumentoMaior = 1.5f;
        d.periodoDoFoco = 2.9f;
        d.rapidezDoFoco = 1.8f;
        d.amostrasDoBorrao = 7;
        return d;
    }
}
