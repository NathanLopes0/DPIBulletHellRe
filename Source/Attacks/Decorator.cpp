//
// O padrao Decorator da fase 3 da Maria Lucia.
//

#include "Decorator.h"

#include <algorithm>

namespace Decorator {

    const char* Nome(const Camada c) {
        // \xc3\xa1 e o a com acento.
        switch (c) {
            case Camada::Rapida:     return "R\xc3\xa1pida";
            case Camada::Dividida:   return "Dividida";
            case Camada::Teleguiada: return "Teleguiada";
        }
        return "";
    }

    std::string Placar(const int camadas) {
        const int n = std::clamp(camadas, 0, kCamadas);
        std::string s = "bala";
        for (int i = 0; i < n; ++i) s = std::string("new ") + Nome(kOrdem[static_cast<size_t>(i)]) + "(" + s + ")";
        return s;
    }

    std::string Cara(const int camadas) {
        return "D" + std::to_string(std::clamp(camadas, 0, kCamadas));
    }

    float Saida(const int lado) {
        return 90.0f - static_cast<float>(lado < 0 ? -1 : 1) * kSaida;
    }

    float Guiar(const float agora, const Vector2 falta, const float deltaTime) {
        return Math::VirarPara(agora, falta, Math::ToRadians(kGiroDaTeleguiada) * std::max(0.0f, deltaTime));
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n == Dificuldade::Nivel::Dificil) d.entreComponentes = 1.25f;
        return d;
    }
}
