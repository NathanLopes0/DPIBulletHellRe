//
// O problema da parada: a repescagem do Vladimir.
//

#include "doctest.h"

#include <algorithm>
#include <set>
#include <cstdint>
#include <vector>

#include "../Source/Attacks/Vladimir/Parada.h"
#include "../Source/Attacks/Engine/Fase.h"

namespace {
    /// Quantos passos ate parar, contando no maximo 'limite'.
    int PassosAteParar(std::uint32_t x, const int limite = 1000) {
        int passos = 0;
        while (!Parada::Parou(x) && passos < limite) {
            x = Parada::Proximo(x);
            ++passos;
        }
        return passos;
    }
}

TEST_CASE("Parada: um passo do programa de Collatz, e o 1 fica") {
    CHECK(Parada::Proximo(6) == 3);
    CHECK(Parada::Proximo(3) == 10);
    CHECK(Parada::Proximo(1) == 1);
    CHECK(Parada::Proximo(0) == 0);
    CHECK(Parada::Parou(1));
    CHECK_FALSE(Parada::Parou(2));
    // 7 -> 22 -> 11 -> 34 -> 17 -> 52 -> 26 -> 13 -> 40 -> 20 -> 10 -> 5 -> 16 -> 8 -> 4 -> 2 -> 1
    CHECK(PassosAteParar(7) == 16);
    CHECK(PassosAteParar(27, 50) == 50);   // o limite segura
}

TEST_CASE("Parada: toda semente para em 14 a 29 passos, sem passar de 400 no caminho") {
    for (const auto s : Parada::kSementes) {
        CAPTURE(s);
        const int passos = PassosAteParar(s);
        CHECK(passos >= 14);
        CHECK(passos <= 29);
        std::uint32_t x = s, maior = s;
        while (!Parada::Parou(x)) { x = Parada::Proximo(x); maior = std::max(maior, x); }
        CHECK(maior <= 400u);
    }
}

TEST_CASE("Parada: a altura sobe com x, do 1 embaixo ao teto em cima") {
    CHECK(Parada::Altura(1) == doctest::Approx(Parada::kBaixo));
    CHECK(Parada::Altura(256) == doctest::Approx(Parada::kAlto));
    CHECK(Parada::Altura(10000) == doctest::Approx(Parada::kAlto));   // nao passa do teto
    CHECK(Parada::Altura(16) == doctest::Approx((Parada::kAlto + Parada::kBaixo) / 2.0f));
    CHECK(Parada::Altura(40) < Parada::Altura(5));                    // em cima e y menor
}

TEST_CASE("Parada: o placar diz o passo que vem") {
    CHECK(Parada::Placar(41) == "x = 41: 3x + 1");
    CHECK(Parada::Placar(124) == "x = 124: x / 2");
    CHECK(Parada::Placar(1) == "x = 1: parou");
}

TEST_CASE("Parada: cada luta roda as doze sementes numa ordem propria, sem repetir antes de rodar todas") {
    std::set<std::vector<std::uint32_t>> ordens;
    int paraADireita = 0;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, Parada::kFaseDaParada);
        const Parada::Plano p = Parada::Sortear(s);
        std::vector<std::uint32_t> volta;
        for (int vez = 0; vez < static_cast<int>(Parada::kSementes.size()); ++vez) volta.push_back(Parada::SementeDaVez(p, vez));
        std::vector<std::uint32_t> ordenada = volta;
        std::sort(ordenada.begin(), ordenada.end());
        CHECK(ordenada == std::vector<std::uint32_t>(Parada::kSementes.begin(), Parada::kSementes.end()));
        CHECK(Parada::SementeDaVez(p, static_cast<int>(Parada::kSementes.size())) == volta.front());
        CHECK((p.sentido == 1.0f || p.sentido == -1.0f));
        paraADireita += p.sentido > 0.0f ? 1 : 0;
        ordens.insert(volta);
    }
    CHECK(ordens.size() == 300u);
    CHECK(paraADireita > 100);
    CHECK(paraADireita < 200);
}

TEST_CASE("Parada: o degrau do Dificil anda mais depressa, e todo programa ainda para dentro da fase") {
    const Parada::Degrau n = Parada::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.passo == Parada::kPasso);
    CHECK(n.rapidez == 1.0f);
    const Parada::Degrau d = Parada::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.passo < n.passo);
    CHECK(d.rapidez > 1.0f);
    for (const std::uint32_t x : Parada::kSementes) {
        CAPTURE(x);
        for (const float passo : {n.passo, d.passo}) {
            CHECK(Parada::kPrimeiro + static_cast<float>(PassosAteParar(x) + 1) * passo < Fase::kDuracao);
        }
    }
}
