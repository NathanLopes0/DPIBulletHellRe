//
// O problema da parada: a repescagem do Vladimir.
//

#include "Parada.h"

#include <algorithm>
#include <cmath>

namespace Parada {

    std::uint32_t Proximo(const std::uint32_t x) {
        if (Parou(x)) return x;
        return (x % 2u == 0u) ? x / 2u : 3u * x + 1u;
    }

    bool Parou(const std::uint32_t x) {
        return x <= 1u;
    }

    int PassosAteParar(std::uint32_t x, const int limite) {
        int passos = 0;
        while (!Parou(x) && passos < limite) {
            x = Proximo(x);
            ++passos;
        }
        return passos;
    }

    std::uint32_t Sortear(const std::uint32_t sorteio) {
        return kSementes[sorteio % kSementes.size()];
    }

    float Altura(const std::uint32_t x) {
        const float bits = std::log2(static_cast<float>(std::max<std::uint32_t>(x, 1u)));
        const float fracao = std::clamp(bits / kBitsDoTeto, 0.0f, 1.0f);
        return kBaixo + (kAlto - kBaixo) * fracao;
    }

    std::string Placar(const std::uint32_t x) {
        const std::string valor = "x = " + std::to_string(x) + ": ";
        if (Parou(x)) return valor + "parou";
        return valor + ((x % 2u == 0u) ? "x / 2" : "3x + 1");
    }

    Plano Sortear(Variacao::Sorteio& s) {
        Plano p;
        p.sementes.assign(kSementes.begin(), kSementes.end());
        s.Embaralhar(p.sementes);
        p.sentido = s.Chance(0.5f) ? 1.0f : -1.0f;
        return p;
    }

    std::uint32_t SementeDaVez(const Plano& p, const int vez) {
        if (p.sementes.empty()) return kSementes[static_cast<size_t>(std::max(0, vez)) % kSementes.size()];
        return p.sementes[static_cast<size_t>(std::max(0, vez)) % p.sementes.size()];
    }

    Degrau DegrauDo(const Dificuldade::Nivel n) {
        Degrau d;
        d.passo = Dificuldade::Escolher(n, kPasso, 0.38f);
        d.rapidez = Dificuldade::Escolher(n, 1.0f, 1.15f);
        return d;
    }
}
