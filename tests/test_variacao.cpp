//
// O sorteio de cada batalha. Camada pura.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include "../Source/Variacao.h"

using namespace Variacao;

TEST_CASE("Variacao: o motor e o mt19937 do padrao, e o rotulo e o FNV-1a") {
    // O 10000o numero do mt19937 com a semente padrao (5489) e fixado pelo
    // proprio padrao do C++ - qualquer biblioteca que der outro nao e mt19937.
    std::mt19937 padrao;
    padrao.discard(9999);
    CHECK(padrao() == 4123659995u);

    CHECK(Rotulo("") == 2166136261u);    // a base do FNV-1a de 32 bits
    CHECK(Rotulo("a") == 3826002220u);   // 0xe40c292c, o valor publicado
    CHECK(Rotulo("StateOne") != Rotulo("StateTwo"));
}

TEST_CASE("Variacao: A MESMA SEMENTE DA A MESMA LUTA, em qualquer compilador") {
    // Os numeros abaixo sairam desta implementacao. Eles so dependem do
    // mt19937 e de aritmetica inteira, entao o MSVC tem de dar os mesmos: se um
    // dia este teste quebrar so num deles, alguem trocou o mapeamento por uma
    // distribuicao da biblioteca.
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
    // 120 ordens possiveis; em 1000 sementes a chance de faltar alguma e de
    // 120 * (119/120)^1000, menos de 3 em 1000.
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

    Sorteio p(5u), q(5u);
    Sorteio f = p.Filho(1);
    Sorteio g = p.Filho(1);
    CHECK(f.Bruto() == g.Bruto());     // o mesmo filho
    CHECK(p.Filho(2).Bruto() != p.Filho(1).Bruto());
    CHECK(p.Bruto() == q.Bruto());     // pedir filhos nao mexeu no pai
}
