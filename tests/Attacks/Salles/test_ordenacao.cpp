//
// O bubble sort da repescagem do Salles.
//

#include "../../doctest.h"

#include <set>
#include "../../../Source/Attacks/Engine/Fase.h"

#include <algorithm>
#include <random>
#include <vector>

#include "../../../Source/Attacks/Salles/Ordenacao.h"

namespace {
    /// Roda o bubble sort ate o fim e devolve os passos.
    std::vector<Ordenacao::Passo> AteOFim(Ordenacao::Bolha& b, const int limite = 1000) {
        std::vector<Ordenacao::Passo> passos;
        for (int k = 0; k < limite; ++k) {
            passos.push_back(Ordenacao::Avancar(b));
            if (passos.back().tipo == Ordenacao::Tipo::Ordenado) break;
        }
        return passos;
    }

    std::vector<int> Aleatorio(std::mt19937& g, const int n) {
        std::vector<int> v;
        std::uniform_int_distribution<int> d(Ordenacao::kMenorValor, Ordenacao::kMaiorValor);
        while (static_cast<int>(v.size()) < n) {
            const int k = d(g);
            if (std::find(v.begin(), v.end(), k) == v.end()) v.push_back(k);
        }
        return v;
    }
}

TEST_CASE("Ordenacao: o vetor fica centrado, e k + 0,5 e a fronteira entre dois lugares") {
    const float largura = 1200.0f;
    const int n = Ordenacao::kTamanho;
    CHECK((Ordenacao::X(0.0f, n, largura) + Ordenacao::X(static_cast<float>(n - 1), n, largura)) / 2.0f
          == doctest::Approx(largura / 2.0f));
    CHECK(Ordenacao::X(1.0f, n, largura) - Ordenacao::X(0.0f, n, largura) == doctest::Approx(Ordenacao::kEntre));
    CHECK(Ordenacao::X(2.5f, n, largura)
          == doctest::Approx((Ordenacao::X(2.0f, n, largura) + Ordenacao::X(3.0f, n, largura)) / 2.0f));
    // O vetor inteiro, e a fronteira depois do ultimo, cabem no campo.
    CHECK(Ordenacao::X(0.0f, n, largura) > 0.0f);
    CHECK(Ordenacao::X(static_cast<float>(n) - 0.5f, n, largura) < largura);
}

TEST_CASE("Ordenacao: o maior cai mais depressa - e a fileira ordenada vira uma rampa") {
    float antes = 0.0f;
    for (int v = Ordenacao::kMenorValor; v <= Ordenacao::kMaiorValor; ++v) {
        CHECK(Ordenacao::VelocidadeDaQueda(v) > antes);
        antes = Ordenacao::VelocidadeDaQueda(v);
    }
}

TEST_CASE("Ordenacao: o vetor sorteado tem valores diferentes, no intervalo, e nasce fora de ordem") {
    for (std::uint32_t s = 0; s < 2000; ++s) {
        const auto v = Ordenacao::Sortear(s * 2654435761u, Ordenacao::kTamanho);
        CAPTURE(s);
        REQUIRE(static_cast<int>(v.size()) == Ordenacao::kTamanho);
        auto ordenado = v;
        std::sort(ordenado.begin(), ordenado.end());
        CHECK(std::adjacent_find(ordenado.begin(), ordenado.end()) == ordenado.end());
        CHECK(ordenado.front() >= Ordenacao::kMenorValor);
        CHECK(ordenado.back() <= Ordenacao::kMaiorValor);
        CHECK_FALSE(std::is_sorted(v.begin(), v.end()));
    }
    CHECK(Ordenacao::Sortear(7u, 8) == Ordenacao::Sortear(7u, 8));
    CHECK(Ordenacao::Sortear(7u, 8) != Ordenacao::Sortear(8u, 8));
}

TEST_CASE("Ordenacao: uma passada sem troca termina tudo - o vetor ja estava em ordem") {
    auto b = Ordenacao::Comecar({2, 1, 3, 4});
    const auto passos = AteOFim(b);
    using T = Ordenacao::Tipo;
    // Primeira passada: troca o 2 com o 1, e o 4 chega a fronteira.
    REQUIRE(passos.size() == 7);
    CHECK(passos[0].tipo == T::Troca);
    CHECK(passos[0].a == 2);
    CHECK(passos[0].b == 1);
    CHECK(passos[1].tipo == T::Fica);
    CHECK(passos[2].tipo == T::Fica);
    CHECK(passos[3].tipo == T::NoLugar);
    CHECK(passos[3].a == 4);
    CHECK(passos[3].i == 3);
    // Segunda passada: so comparacoes, e nenhuma troca.
    CHECK(passos[4].tipo == T::Fica);
    CHECK(passos[5].tipo == T::Fica);
    CHECK(passos[6].tipo == T::Ordenado);
    CHECK(passos[6].semTroca);
    CHECK(b.v == std::vector<int>{1, 2, 3, 4});
    // E depois de ordenado, ordenado para sempre.
    CHECK(Ordenacao::Avancar(b).tipo == T::Ordenado);
}

TEST_CASE("Ordenacao: o vetor do avesso assenta um por passada, do maior para o menor") {
    auto b = Ordenacao::Comecar({5, 4, 3, 2, 1});
    std::vector<int> assentados;
    for (const auto& p : AteOFim(b)) {
        if (p.tipo == Ordenacao::Tipo::NoLugar) assentados.push_back(p.a);
    }
    CHECK(assentados == std::vector<int>{5, 4, 3, 2});
    CHECK(b.v == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("Ordenacao: todo vetor termina ordenado, e cada passo e o que diz ser") {
    std::mt19937 g(20261023u);
    for (int caso = 0; caso < 500; ++caso) {
        const int n = 2 + caso % 9;
        auto b = Ordenacao::Comecar(Aleatorio(g, n));
        auto original = b.v;
        CAPTURE(caso);
        int passos = 0;
        for (;;) {
            const auto antes = b.v;
            const int fimAntes = b.fim;
            const auto p = Ordenacao::Avancar(b);
            ++passos;
            REQUIRE(passos <= n * n + n);
            if (p.tipo == Ordenacao::Tipo::Ordenado) break;
            if (p.tipo == Ordenacao::Tipo::Troca) {
                CHECK(p.a > p.b);
                CHECK(b.v[static_cast<size_t>(p.i)] == p.b);
                CHECK(b.v[static_cast<size_t>(p.i + 1)] == p.a);
            } else if (p.tipo == Ordenacao::Tipo::Fica) {
                CHECK(p.a < p.b);
                CHECK(b.v == antes);
            } else {
                // QUEM ASSENTA E O MAIOR do que faltava ordenar.
                CHECK(p.i == fimAntes);
                CHECK(p.a == *std::max_element(antes.begin(), antes.begin() + fimAntes + 1));
            }
        }
        CHECK(std::is_sorted(b.v.begin(), b.v.end()));
        std::sort(original.begin(), original.end());
        CHECK(b.v == original);
    }
}

TEST_CASE("Ordenacao: tirar a bolha do meio da passada recomeca a passada, e o vetor ainda termina ordenado") {
    // O CASO QUE QUEBRAVA: a bolha carrega o 9 ate o meio, o aluno o derruba, e
    // continuar a passada dali assentaria o 3 com o 5 ainda antes dele.
    auto b = Ordenacao::Comecar({5, 9, 1, 3});
    CHECK(Ordenacao::Avancar(b).tipo == Ordenacao::Tipo::Fica);    // 5 < 9
    CHECK(Ordenacao::Avancar(b).tipo == Ordenacao::Tipo::Troca);   // 9 > 1
    REQUIRE(b.v == std::vector<int>{5, 1, 9, 3});
    Ordenacao::Remover(b, 2);
    CHECK(b.v == std::vector<int>{5, 1, 3});
    CHECK(b.i == 0);
    for (const auto& p : AteOFim(b)) {
        if (p.tipo == Ordenacao::Tipo::NoLugar) CHECK(p.a == 5);   // so o 5 assenta: e o maior
    }
    CHECK(b.v == std::vector<int>{1, 3, 5});
}

TEST_CASE("Ordenacao: tirar um elemento ja no lugar nao mexe na passada") {
    auto b = Ordenacao::Comecar({3, 1, 4, 2});
    // Uma passada inteira: o 4 assenta.
    for (;;) {
        if (Ordenacao::Avancar(b).tipo == Ordenacao::Tipo::NoLugar) break;
    }
    Ordenacao::Avancar(b);   // a primeira comparacao da passada seguinte
    const int i = b.i;
    const int fim = b.fim;
    Ordenacao::Remover(b, static_cast<int>(b.v.size()) - 1);   // o 4
    CHECK(b.i == i);
    CHECK(b.fim == fim);
    AteOFim(b);
    CHECK(b.v == std::vector<int>{1, 2, 3});
}

TEST_CASE("Ordenacao: com remocoes a qualquer hora, todo vetor termina ordenado com quem sobrou") {
    std::mt19937 g(7u);
    for (int caso = 0; caso < 500; ++caso) {
        auto b = Ordenacao::Comecar(Aleatorio(g, Ordenacao::kTamanho));
        auto sobrou = b.v;
        CAPTURE(caso);
        std::uniform_int_distribution<int> chance(0, 9);
        int passos = 0;
        while (Ordenacao::Avancar(b).tipo != Ordenacao::Tipo::Ordenado) {
            REQUIRE(++passos < 1000);
            if (chance(g) == 0 && !b.v.empty()) {
                std::uniform_int_distribution<int> qual(0, static_cast<int>(b.v.size()) - 1);
                const int k = qual(g);
                sobrou.erase(std::find(sobrou.begin(), sobrou.end(), b.v[static_cast<size_t>(k)]));
                Ordenacao::Remover(b, k);
            }
        }
        CHECK(std::is_sorted(b.v.begin(), b.v.end()));
        std::sort(sobrou.begin(), sobrou.end());
        CHECK(b.v == sobrou);
    }
}

TEST_CASE("Ordenacao: tirar ate sobrar um termina a ordenacao") {
    auto b = Ordenacao::Comecar({4, 2, 7});
    Ordenacao::Remover(b, 0);
    CHECK_FALSE(b.ordenado);
    Ordenacao::Remover(b, 0);
    CHECK(b.ordenado);
    CHECK(Ordenacao::Avancar(b).tipo == Ordenacao::Tipo::Ordenado);
    Ordenacao::Remover(b, 5);   // fora do vetor: nada
    CHECK(b.v == std::vector<int>{7});
}

TEST_CASE("Ordenacao: os placares escrevem a comparacao e o veredito") {
    using P = Ordenacao::Passo;
    using T = Ordenacao::Tipo;
    CHECK(Ordenacao::PlacarDoComeco(8) == "bubble sort, n = 8");
    CHECK(Ordenacao::Placar(P{T::Troca, 2, 57, 42, false}) == "57 > 42: troca");
    CHECK(Ordenacao::Placar(P{T::Fica, 2, 42, 57, false}) == "42 < 57: fica");
    CHECK(Ordenacao::Placar(P{T::NoLugar, 7, 87, 0, false}) == "87 no lugar");
    CHECK(Ordenacao::Placar(P{T::Ordenado, 0, 0, 0, false}) == "ordenado!");
    CHECK(Ordenacao::Placar(P{T::Ordenado, 0, 0, 0, true}) == "nenhuma troca: ordenado!");
    CHECK(Ordenacao::CaraDoElemento(42) == "K42");
    CHECK(Ordenacao::CaraDoElemento(150) == "K99");
}

TEST_CASE("Ordenacao: o vetor chega de tres jeitos - em qualquer ordem, quase ordenado ou invertido") {
    for (std::uint32_t s = 1; s < 300; ++s) {
        CAPTURE(s);
        for (const int n : {7, 8}) {
            const auto quase = Ordenacao::Sortear(s * 2654435761u, n, Ordenacao::Entrada::QuaseOrdenada);
            const auto inv = Ordenacao::Sortear(s * 2654435761u, n, Ordenacao::Entrada::Invertida);
            REQUIRE(static_cast<int>(quase.size()) == n);
            CHECK_FALSE(std::is_sorted(quase.begin(), quase.end()));
            CHECK(std::is_sorted(inv.rbegin(), inv.rend()));
            // QUASE ORDENADO: duas inversoes, e o bubble sort acaba cedo, na
            // passada sem troca. INVERTIDO: o pior caso, toda comparacao troca.
            int inversoes = 0;
            for (int i = 0; i < n; ++i) for (int j = i + 1; j < n; ++j) inversoes += quase[i] > quase[j] ? 1 : 0;
            CHECK(inversoes == 2);
            auto bolha = Ordenacao::Comecar(quase);
            int passos = 0, trocasQ = 0;
            bool semTroca = false;
            while (passos++ < 200) {
                const auto p = Ordenacao::Avancar(bolha);
                if (p.tipo == Ordenacao::Tipo::Troca) ++trocasQ;
                if (p.tipo == Ordenacao::Tipo::Ordenado) { semTroca = p.semTroca; break; }
            }
            CHECK(trocasQ == 2);
            CHECK(semTroca);
            auto pior = Ordenacao::Comecar(inv);
            int trocas = 0;
            for (int k = 0; k < 200; ++k) {
                const auto p = Ordenacao::Avancar(pior);
                if (p.tipo == Ordenacao::Tipo::Troca) ++trocas;
                if (p.tipo == Ordenacao::Tipo::Ordenado) break;
            }
            CHECK(trocas == n * (n - 1) / 2);
        }
    }
    CHECK(Ordenacao::PlacarDoComeco(8, Ordenacao::Entrada::Aleatoria) == "bubble sort, n = 8");
    CHECK(Ordenacao::PlacarDoComeco(7, Ordenacao::Entrada::Invertida) == "bubble sort, n = 7 \xc2\xb7 invertido: o pior caso");
}

TEST_CASE("Ordenacao: a repescagem sorteada passa pelos tres jeitos, e o pior caso cabe na fase") {
    std::set<std::vector<int>> ordens;
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s(semente);
        const Ordenacao::Plano p = Ordenacao::Sortear(s);
        CHECK(p.tamanho >= 7);
        CHECK(p.tamanho <= Ordenacao::kTamanho);
        std::vector<int> e;
        for (int k = 0; k < 3; ++k) e.push_back(static_cast<int>(Ordenacao::EntradaDaVez(p, k)));
        CHECK(std::set<int>(e.begin(), e.end()).size() == 3u);
        CHECK(Ordenacao::EntradaDaVez(p, 3) == Ordenacao::EntradaDaVez(p, 0));
        ordens.insert(e);
    }
    CHECK(ordens.size() == 6u);
    // O PIOR CASO DE 8, nos dois modos: cada comparacao e uma troca, e cada
    // passada termina com um elemento assentando. Tem de caber na fase.
    for (const Dificuldade::Nivel nivel : Dificuldade::kNiveis) {
        const Ordenacao::Degrau d = Ordenacao::DegrauDo(nivel);
        const int n = Ordenacao::kTamanho;
        const float pior = Ordenacao::kPrimeiro + Ordenacao::kAteComparar +
                           static_cast<float>(n * (n - 1) / 2 + n) * d.passo + static_cast<float>(n) * Ordenacao::kNoLugar;
        CHECK(pior < Fase::kDuracao);
    }
    const Ordenacao::Degrau n = Ordenacao::DegrauDo(Dificuldade::Nivel::Normal);
    const Ordenacao::Degrau d = Ordenacao::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.passo == doctest::Approx(Ordenacao::kPasso));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(d.passo < n.passo);
    CHECK(d.anelDoLugar > n.anelDoLugar);
}
