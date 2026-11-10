// O sorteio de cada batalha.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include "../Source/Regras/Variacao.h"

using namespace Variacao;

TEST_CASE("Variacao: o motor e o mt19937 do padrao, e o rotulo e o FNV-1a") {
    // O 10000o numero do mt19937 com a semente padrao (5489) e fixado pelo proprio padrao do C++.
    std::mt19937 padrao;
    padrao.discard(9999);
    CHECK(padrao() == 4123659995u);

    CHECK(Rotulo("") == 2166136261u);    // A base do fnv-1a de 32 bits.
    CHECK(Rotulo("a") == 3826002220u);   // 0xe40c292c, o valor publicado
    CHECK(Rotulo("StateOne") != Rotulo("StateTwo"));
}

TEST_CASE("Variacao: A MESMA SEMENTE DA A MESMA LUTA, em qualquer compilador") {
    // Os numeros abaixo sairam desta implementacao.
    CHECK(Misturar(0, 0) == 2570937750u);
    CHECK(Misturar(1, 0) == 1739479842u);
    CHECK(Misturar(0, 1) == 403206445u);

    Sorteio s(20261014u);
    std::vector<int> tirados;
    for (int i = 0; i < 8; ++i) tirados.push_back(s.Inteiro(0, 9));
    CHECK(tirados == std::vector<int>{0, 1, 0, 3, 2, 5, 1, 1});

    Sorteio t(20261014u);
    std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7};
    t.Embaralhar(v);
    CHECK(v == std::vector<int>{4, 1, 5, 6, 3, 7, 2, 0});

    Sorteio u = SorteioDa(1234u, "StateOne");
    std::vector<int> w;
    for (int i = 0; i < 5; ++i) w.push_back(u.Inteiro(-3, 3));
    CHECK(w == std::vector<int>{1, -1, 3, 2, 1});
}

TEST_CASE("Variacao: o inteiro fica na faixa e sai todo valor dela, por igual") {
    Sorteio s(7u);
    std::map<int, int> vezes;
    constexpr int kTiros = 70000;
    for (int i = 0; i < kTiros; ++i) {
        const int x = s.Inteiro(-3, 3);
        REQUIRE(x >= -3);
        REQUIRE(x <= 3);
        ++vezes[x];
    }
    CHECK(vezes.size() == 7u);
    for (const auto& [x, n] : vezes) {
        CAPTURE(x);
        CHECK(n > kTiros / 7 * 0.95);
        CHECK(n < kTiros / 7 * 1.05);
    }
    CHECK(s.Inteiro(5, 5) == 5);
    CHECK(s.Inteiro(5, 2) == 5);   // faixa vazia: o comeco
    // As pontas do int nao estouram.
    for (int i = 0; i < 100; ++i) {
        const int x = s.Inteiro(INT32_MIN, INT32_MAX);
        (void) x;
        CHECK(s.Inteiro(INT32_MAX - 1, INT32_MAX) >= INT32_MAX - 1);
        CHECK(s.Inteiro(INT32_MIN, INT32_MIN + 1) <= INT32_MIN + 1);
    }
}

TEST_CASE("Variacao: o real nunca chega no limite de cima, e a chance respeita as pontas") {
    Sorteio s(11u);
    float menor = 1.0f, maior = 0.0f;
    for (int i = 0; i < 20000; ++i) {
        const float r = s.Real(2.0f, 2.5f);
        REQUIRE(r >= 2.0f);
        REQUIRE(r < 2.5f);
        menor = std::min(menor, r - 2.0f);
        maior = std::max(maior, r - 2.0f);
    }
    CHECK(menor < 0.01f);
    CHECK(maior > 0.49f);
    // Uma faixa tao estreita que o arredondamento cairia no limite.
    for (int i = 0; i < 1000; ++i) CHECK(s.Real(1.0f, std::nextafter(1.0f, 2.0f)) == 1.0f);
    CHECK(s.Real(3.0f, 3.0f) == 3.0f);

    int sim = 0;
    for (int i = 0; i < 10000; ++i) {
        CHECK_FALSE(s.Chance(0.0f));
        CHECK(s.Chance(1.0f));
        sim += s.Chance(0.3f) ? 1 : 0;
    }
    CHECK(sim > 2800);
    CHECK(sim < 3200);
}

TEST_CASE("Variacao: embaralhar da uma permutacao, e sementes diferentes dao ordens diferentes") {
    std::set<std::vector<int>> ordens;
    for (std::uint32_t semente = 0; semente < 1000; ++semente) {
        std::vector<int> v = {0, 1, 2, 3, 4};
        Sorteio s(semente);
        s.Embaralhar(v);
        std::vector<int> ordenado = v;
        std::sort(ordenado.begin(), ordenado.end());
        CHECK(ordenado == std::vector<int>{0, 1, 2, 3, 4});
        ordens.insert(v);
    }
    CHECK(ordens.size() == 120u);
}

TEST_CASE("Variacao: cada fase tem a propria sequencia, e o filho nao gasta o pai") {
    Sorteio a = SorteioDa(99u, "StateOne");
    Sorteio b = SorteioDa(99u, "StateTwo");
    Sorteio c = SorteioDa(100u, "StateOne");
    int iguaisAB = 0, iguaisAC = 0;
    for (int i = 0; i < 64; ++i) {
        const auto x = a.Bruto();
        iguaisAB += (x == b.Bruto()) ? 1 : 0;
        iguaisAC += (x == c.Bruto()) ? 1 : 0;
    }
    CHECK(iguaisAB == 0);
    CHECK(iguaisAC == 0);
}

TEST_CASE("Variacao: a ordem das formas - as fixas no lugar, o resto embaralhado, e a volta seguinte") {
    std::set<std::vector<int>> vistas;
    for (std::uint32_t semente = 0; semente < 400; ++semente) {
        Sorteio s(semente);
        const auto ordem = Ordem(s, 5, 1);
        REQUIRE(ordem.size() == 5u);
        CHECK(ordem[0] == 0);
        std::vector<int> ordenada = ordem;
        std::sort(ordenada.begin(), ordenada.end());
        CHECK(ordenada == std::vector<int>{0, 1, 2, 3, 4});
        vistas.insert(ordem);
        for (int vez = 0; vez < 15; ++vez) {
            CHECK(NaOrdem(ordem, vez) % 5 == ordem[static_cast<size_t>(vez % 5)]);
            CHECK(NaOrdem(ordem, vez) / 5 == vez / 5);   // a volta e a mesma
        }
    }
    CHECK(vistas.size() == 24u);   // 4! ordens para as quatro depois da primeira
    Sorteio s(3u);
    const auto duas = Ordem(s, 4, 2);
    CHECK(duas[0] == 0);
    CHECK(duas[1] == 1);
    CHECK(NaOrdem({}, 7) == 7);
    CHECK(Ordem(s, 3, 3) == std::vector<int>{0, 1, 2});
}

TEST_CASE("Variacao: duas sementes novas seguidas nao saem iguais") {
    // O relogio e a contagem entram junto com o random_device: mesmo um random_device que se repete
    // (MinGW antigo) nao faz duas lutas iguais.
    const std::uint32_t a = Variacao::SementeNova();
    const std::uint32_t b = Variacao::SementeNova();
    CHECK(a != b);
}

// ---- O disparo de cada vez ("variar" em fases.json) ----

TEST_CASE("Variacao: sem variar, o disparo e o do arquivo") {
    Variacao::Sorteio s(7);
    for (int i = 0; i < 50; ++i) {
        const Variacao::Disparo d = Variacao::Sortear(s, Variacao::DoDisparo{});
        CHECK(d.giro == 0.0f);
        CHECK(d.abertura == 1.0f);
        CHECK(d.velocidade == 1.0f);
        CHECK(d.projeteis == 0);
        CHECK(d.espera == 1.0f);
    }
}

TEST_CASE("Variacao: o disparo fica na faixa, sai todo valor dela, e a media e a do arquivo") {
    Variacao::DoDisparo v;
    v.giro = 10.0f;
    v.abertura = 0.2f;
    v.velocidade = 0.1f;
    v.projeteis = 2;
    v.cadencia = 0.25f;
    Variacao::Sorteio s(2026);
    double giro = 0, abertura = 0, velocidade = 0, projeteis = 0, espera = 0;
    int contagem[5] = {};
    const int n = 20000;
    for (int i = 0; i < n; ++i) {
        const Variacao::Disparo d = Variacao::Sortear(s, v);
        REQUIRE(d.giro >= -10.0f);
        REQUIRE(d.giro <= 10.0f);
        REQUIRE(d.abertura >= 0.8f);
        REQUIRE(d.abertura <= 1.2f);
        REQUIRE(d.velocidade >= 0.9f);
        REQUIRE(d.velocidade <= 1.1f);
        REQUIRE(d.projeteis >= -2);
        REQUIRE(d.projeteis <= 2);
        REQUIRE(d.espera >= 0.75f);
        REQUIRE(d.espera <= 1.25f);
        ++contagem[d.projeteis + 2];
        giro += d.giro;
        abertura += d.abertura;
        velocidade += d.velocidade;
        projeteis += d.projeteis;
        espera += d.espera;
    }
    // Simetrico em volta do arquivo: a dificuldade media nao muda.
    CHECK(std::abs(giro / n) < 0.2);
    CHECK(abertura / n == doctest::Approx(1.0).epsilon(0.005));
    CHECK(velocidade / n == doctest::Approx(1.0).epsilon(0.005));
    CHECK(std::abs(projeteis / n) < 0.05);
    CHECK(espera / n == doctest::Approx(1.0).epsilon(0.005));
    for (const int c : contagem) CHECK(c > n / 7);
}

TEST_CASE("Variacao: ligar uma variacao nao muda o que as outras sorteiam") {
    Variacao::DoDisparo so;
    so.giro = 10.0f;
    Variacao::DoDisparo mais = so;
    mais.velocidade = 0.3f;
    mais.projeteis = 1;
    Variacao::Sorteio a(99), b(99);
    for (int i = 0; i < 100; ++i) {
        CHECK(Variacao::Sortear(a, so).giro == Variacao::Sortear(b, mais).giro);
    }
}

TEST_CASE("Variacao: a mesma semente da os mesmos disparos") {
    Variacao::DoDisparo v;
    v.giro = 15.0f;
    v.cadencia = 0.3f;
    Variacao::Sorteio a = Variacao::SorteioDa(1234, "StateOne#0");
    Variacao::Sorteio b = Variacao::SorteioDa(1234, "StateOne#0");
    Variacao::Sorteio outro = Variacao::SorteioDa(1234, "StateOne#1");
    bool algumDiferente = false;
    for (int i = 0; i < 20; ++i) {
        const Variacao::Disparo da = Variacao::Sortear(a, v);
        const Variacao::Disparo db = Variacao::Sortear(b, v);
        CHECK(da.giro == db.giro);
        CHECK(da.espera == db.espera);
        if (Variacao::Sortear(outro, v).giro != da.giro) algumDiferente = true;
    }
    // Cada ataque da fase tem a sua sequencia.
    CHECK(algumDiferente);
}

TEST_CASE("Variacao: a quantidade nunca cai abaixo de um, e zero continua zero") {
    CHECK(Variacao::Projeteis(1, -3) == 1);
    CHECK(Variacao::Projeteis(5, -2) == 3);
    CHECK(Variacao::Projeteis(5, 2) == 7);
    CHECK(Variacao::Projeteis(0, 2) == 0);
}
