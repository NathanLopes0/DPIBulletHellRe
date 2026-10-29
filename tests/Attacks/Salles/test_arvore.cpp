//
// A arvore binaria de busca da fase 3 do Salles.
//

#include "../../doctest.h"

#include <algorithm>
#include <set>
#include "../../../Source/Attacks/Engine/Fase.h"

#include <random>
#include <vector>

#include "../../../Source/Attacks/Salles/Arvore.h"

namespace {
    /// Insere como o Salles insere: pelo caminho, no lugar vazio do fim, se couber.
    void Inserir(Arvore::Nos& nos, const int chave) {
        const auto c = Arvore::Caminho(nos, chave);
        if (Arvore::Cabe(c.back()) && !nos.count(c.back())) nos[c.back()] = chave;
    }

    /// Toda chave a esquerda e menor e toda a direita e maior - a propriedade de
    /// busca, conferida em toda a arvore.
    bool EBusca(const Arvore::Nos& nos, const int i, const int menor, const int maior) {
        const auto it = nos.find(i);
        if (it == nos.end()) return true;
        if (it->second <= menor || it->second >= maior) return false;
        return EBusca(nos, 2 * i, menor, it->second) && EBusca(nos, 2 * i + 1, it->second, maior);
    }
}

TEST_CASE("Arvore: o indice de heap diz a profundidade e o lugar") {
    CHECK(Arvore::Profundidade(1) == 0);
    CHECK(Arvore::Profundidade(2) == 1);
    CHECK(Arvore::Profundidade(3) == 1);
    CHECK(Arvore::Profundidade(31) == 4);
    CHECK(Arvore::Cabe(31));
    CHECK_FALSE(Arvore::Cabe(32));
    CHECK_FALSE(Arvore::Cabe(0));
    CHECK(Arvore::X(1) == doctest::Approx(Arvore::kXDaRaiz));
    CHECK(Arvore::X(2) < Arvore::X(1));
    CHECK(Arvore::X(3) > Arvore::X(1));
    CHECK(Arvore::X(3) - Arvore::X(1) == doctest::Approx(Arvore::kLarguraDaArvore / 4.0f));
    CHECK(Arvore::Y(4) == doctest::Approx(Arvore::kYDaRaiz + 2.0f * Arvore::kEntreNiveis));
    // As folhas mais fundas cabem na largura.
    CHECK(Arvore::X(16) > Arvore::kXDaRaiz - Arvore::kLarguraDaArvore / 2.0f);
    CHECK(Arvore::X(31) < Arvore::kXDaRaiz + Arvore::kLarguraDaArvore / 2.0f);
}

TEST_CASE("Arvore: o caminho desce comparando, e inserir por ele mantem a arvore de busca") {
    Arvore::Nos nos;
    for (const int k : {50, 30, 70, 40, 20, 60, 80, 45}) Inserir(nos, k);
    CHECK(nos.at(1) == 50);
    CHECK(nos.at(2) == 30);
    CHECK(nos.at(5) == 40);
    CHECK(nos.at(11) == 45);
    CHECK(Arvore::Caminho(nos, 42) == std::vector<int>{1, 2, 5, 11, 22});   // 50, 30, 40, 45, e o vazio a esquerda do 45
    CHECK(Arvore::Caminho(nos, 35) == std::vector<int>{1, 2, 5, 10});       // 50, 30, 40, e o vazio a esquerda do 40
    CHECK(Arvore::Caminho(nos, 45) == std::vector<int>{1, 2, 5, 11});   // achou o 45

    std::mt19937 g(7);
    Arvore::Nos aleatoria;
    for (int i = 0; i < 200; ++i) Inserir(aleatoria, 1 + static_cast<int>(g() % 99));
    CHECK(EBusca(aleatoria, 1, 0, 100));
    for (const auto& [i, k] : aleatoria) CHECK(Arvore::Cabe(i));
}

TEST_CASE("Arvore: o caminho que passa do fundo termina fora da arvore") {
    Arvore::Nos nos;
    for (const int k : {10, 20, 30, 40, 50}) Inserir(nos, k);            // um galho so, para a direita
    const auto c = Arvore::Caminho(nos, 60);
    CHECK(c.size() == 6);
    CHECK_FALSE(Arvore::Cabe(c.back()));
}

TEST_CASE("Arvore: a subarvore e o no e tudo o que pende dele") {
    Arvore::Nos nos;
    for (const int k : {50, 30, 70, 40, 20, 60}) Inserir(nos, k);
    CHECK(Arvore::Subarvore(nos, 2) == std::vector<int>{2, 4, 5});
    CHECK(Arvore::Subarvore(nos, 1).size() == nos.size());
    CHECK(Arvore::Subarvore(nos, 7).empty());                             // vazio
}

TEST_CASE("Arvore: a proxima operacao, e os placares") {
    std::mt19937 g(2026);
    Arvore::Nos nos;
    for (int i = 0; i < 500; ++i) {
        const auto op = Arvore::Proxima(nos, static_cast<std::uint32_t>(g()));
        REQUIRE(op.chave >= 1);
        REQUIRE(op.chave <= Arvore::kMaiorChave);
        if (nos.empty()) REQUIRE(op.tipo == Arvore::Tipo::Inserir);
        if (op.tipo == Arvore::Tipo::Inserir) {
            for (const auto& [i2, k] : nos) REQUIRE(k != op.chave);         // chave nova
            Inserir(nos, op.chave);
            if (nos.size() > 20) nos.clear();
        }
    }
    CHECK(Arvore::PlacarDaOperacao({Arvore::Tipo::Inserir, 42}) == "inserir(42)");
    CHECK(Arvore::PlacarDaComparacao(42, 50) == "42 < 50: esquerda");
    CHECK(Arvore::PlacarDaComparacao(42, 30) == "42 > 30: direita");
    CHECK(Arvore::PlacarDoFim({Arvore::Tipo::Inserir, 42}, false, true) == "42 entra");
    CHECK(Arvore::PlacarDoFim({Arvore::Tipo::Inserir, 42}, false, false) == "42: altura demais!");
    CHECK(Arvore::PlacarDoFim({Arvore::Tipo::Buscar, 42}, true, true) == "42: achou");
    CHECK(Arvore::PlacarDoFim({Arvore::Tipo::Buscar, 42}, false, true) == "42: NULL");
}

TEST_CASE("Arvore: a arvore sorteada muda a chance de inserir, e a chave nova e sempre nova") {
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s(semente);
        const Arvore::Plano p = Arvore::Sortear(s);
        CHECK(p.chanceDeInserir >= 0.55f);
        CHECK(p.chanceDeInserir < 0.75f);
        Arvore::Nos nos;
        std::mt19937 g(semente);
        for (int i = 0; i < 30; ++i) {
            const auto op = Arvore::Proxima(nos, static_cast<std::uint32_t>(g()), p);
            if (op.tipo != Arvore::Tipo::Inserir) continue;
            for (const auto& [indice, chave] : nos) CHECK(chave != op.chave);
            const auto caminho = Arvore::Caminho(nos, op.chave);
            if (Arvore::Cabe(caminho.back())) nos[caminho.back()] = op.chave;
        }
    }
    const Arvore::Degrau n = Arvore::DegrauDo(Dificuldade::Nivel::Normal);
    const Arvore::Degrau d = Arvore::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.emCadaNo == doctest::Approx(Arvore::kEmCadaNo));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(d.emCadaNo < n.emCadaNo);
    CHECK(d.rapidez > n.rapidez);
    CHECK(d.anelDoAchado > n.anelDoAchado);
}
