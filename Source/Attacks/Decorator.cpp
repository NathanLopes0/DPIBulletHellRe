//
// O padrao Decorator da fase 3 da Maria Lucia.
//

#include "Decorator.h"

#include <algorithm>

namespace Decorator {

    const char* Nome(const Camada c) {
        switch (c) {
            case Camada::Rapida:     return "Rápida";
            case Camada::Dividida:   return "Dividida";
            case Camada::Teleguiada: return "Teleguiada";
        }
        return "";
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

    std::string OQueFaz(const Camada c) {
        switch (c) {
            case Camada::Rapida:     return "+Rápida: a bala anda 1,6× mais";
            case Camada::Dividida:   return "+Dividida: a bala vira duas";
            case Camada::Teleguiada: return "+Teleguiada: a bala persegue você";
        }
        return "";
    }

    std::string OQueSai(const Camada c) {
        switch (c) {
            case Camada::Rapida:     return "Você tirou a Rápida: mais devagar";
            case Camada::Dividida:   return "Você tirou a Dividida";
            case Camada::Teleguiada: return "Você tirou a Teleguiada: segue reta";
        }
        return "";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n == Dificuldade::Nivel::Dificil) d.entreComponentes = 0.95f;
        return d;
    }
}
