//
// A repescagem da Maria Lucia: nao ha bala de prata.
//

#include "BalaDePrata.h"

#include <algorithm>

namespace BalaDePrata {

    const std::vector<std::string>& Promessas() {
        // \xc3\xa7 e \xc3\xa3 sao c cedilha e a til, \xc3\xa9 e e agudo, \xc3\xa1 a agudo.
        static const std::vector<std::string> promessas = {
            "IA", "UML", "Orienta\xc3\xa7\xc3\xa3o a objetos", "M\xc3\xa9todos \xc3\xa1geis",
            "Nuvem", "Low-code", "Microsservi\xc3\xa7os", "Ferramenta CASE"};
        return promessas;
    }

    bool SeDesfaz(const float distancia, const float voo) {
        return distancia <= kPerto || voo >= kVooMaximo;
    }

    std::vector<int> Sortear(Variacao::Sorteio& s) {

        return Variacao::Ordem(s, static_cast<int>(Promessas().size()), 0);
    }

    std::string Desfeita(const std::string& promessa) {
        return promessa + "? N\xc3\xa3o h\xc3\xa1 bala de prata.";
    }

    std::string Testada(const std::string& promessa) {
        return promessa + " \xc2\xb7 teste com usu\xc3\xa1rio: reprovada";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.entreBalas = Dificuldade::Escolher(n, d.entreBalas, 1.4f);
        d.giro = Dificuldade::Escolher(n, d.giro, 125.0f);
        return d;
    }

    float Guiar(const float agora, const Vector2 falta, const float giro, const float deltaTime) {
        return Math::VirarPara(agora, falta, Math::ToRadians(giro) * std::max(0.0f, deltaTime));
    }

    float RapidezDoLobisomem(const float t) {
        return 70.0f + 45.0f * std::clamp(t / 20.0f, 0.0f, 1.0f);
    }

    Vector2 PassoDoLobisomem(const Vector2& lobo, const Vector2& alvo, const float rapidez, const float dt) {
        const Vector2 falta = alvo - lobo;
        const float d = falta.Length();
        const float passo = rapidez * dt;
        if (d <= passo || d <= 0.0f) return alvo;
        return lobo + falta * (passo / d);
    }

    bool Atingiu(const Vector2& bala, const Vector2& lobo, const float escala) {
        const float r = kRaioDoLobisomem * escala;
        return (bala - lobo).LengthSq() <= r * r;
    }

    float EscalaDoLobisomem(const int balas) {
        return std::min(kMaiorEscala, kEscalaInicial + 0.08f * static_cast<float>(std::max(0, balas)));
    }

    std::string NaoMatou(const std::string& promessa) {
        // \xc3\xa3 e o a com til.
        return promessa + "? N\xc3\xa3o matou o lobisomem.";
    }
}
