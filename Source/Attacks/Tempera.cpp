//
// A repescagem do Arroyo: a tempera simulada.
//

#include "Tempera.h"

#include <algorithm>
#include <cmath>

namespace Tempera {

    float Temperatura(const float t) {
        // Esfria de kQuente a kFrio em cada metade da fase: T = kQuente * r^u, com u de 0 a 1.
        const float meia = kReaquecimento;
        const float u = std::clamp(std::fmod(std::max(t, 0.0f), meia) / meia, 0.0f, 1.0f);
        return kQuente * std::pow(kFrio / kQuente, u);
    }

    float Energia(const Vector2& x, const Vector2& aluno, const std::vector<Vector2>& vales) {
        float e = (x - aluno).Length() - kFundoDoAluno;
        for (const Vector2& v : vales) e = std::min(e, (x - v).Length());
        return e;
    }

    bool Aceita(const float delta, const float temperatura, const float sorteio) {
        if (delta <= 0.0f) return true;
        return sorteio < ChanceDePior(delta, temperatura);
    }

    float ChanceDePior(const float delta, const float temperatura) {
        if (delta <= 0.0f) return 1.0f;
        if (temperatura <= 0.0f) return 0.0f;
        return std::exp(-delta / (temperatura * kEscala));
    }

    Vector2 Proposta(const Vector2& x, const float temperatura, Variacao::Sorteio& s) {
        const float u = std::clamp(temperatura / kQuente, 0.0f, 1.0f);
        const float passo = kPassoFrio + (kPassoQuente - kPassoFrio) * std::sqrt(u);
        const float angulo = s.Real(0.0f, 2.0f * Math::Pi);
        const float r = passo * s.Real(0.4f, 1.0f);
        return Vector2(x.x + r * std::cos(angulo), x.y + r * std::sin(angulo));
    }

    std::string Placar(const float temperatura) {
        const int pior = static_cast<int>(std::lround(100.0f * ChanceDePior(kPiorDeReferencia, temperatura)));
        // \xc2\xb7 e o ponto do meio.
        return "T = " + std::to_string(static_cast<int>(std::lround(temperatura))) + "  \xc2\xb7  aceita pior: " +
               std::to_string(pior) + "%";
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        if (n != Dificuldade::Nivel::Dificil) return d;
        d.solucoes = 18;
        d.porPasso = 0.13f;
        d.rapidez = 175.0f;
        return d;
    }
}
