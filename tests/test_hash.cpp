//
// A tabela hash da fase 2 do Salles.
//

#include "doctest.h"

#include <set>
#include "../Source/Attacks/Fase.h"

#include <algorithm>
#include <random>
#include <vector>

#include "../Source/Attacks/Hash.h"

TEST_CASE("Hash: h(k) = k mod m, e o placar o escreve por extenso") {
    CHECK(Hash::Balde(23, 7) == 2);
    CHECK(Hash::Balde(7, 7) == 0);
    CHECK(Hash::Balde(23, 11) == 1);
    CHECK(Hash::PlacarDaInsercao(23, 7) == "h(23) = 23 mod 7 = 2");
    CHECK(Hash::PlacarDaBusca(23, true) == "busca(23): achou");
    CHECK(Hash::PlacarDaBusca(41, false) == "busca(41): NULL");
    CHECK(Hash::PlacarDoRehash(7, 11) == "rehash: m = 7 \xE2\x86\x92 11");
}

TEST_CASE("Hash: o rehash cresce pelos primos, e para no ultimo") {
    CHECK(Hash::ProximoTamanho(7) == 11);
    CHECK(Hash::ProximoTamanho(11) == 13);
    CHECK(Hash::ProximoTamanho(17) == 17);
    CHECK_FALSE(Hash::PrecisaDeRehash(7, 7));      // fator de carga 1: ainda nao
    CHECK(Hash::PrecisaDeRehash(8, 7));
    CHECK_FALSE(Hash::PrecisaDeRehash(30, 17));    // o ultimo tamanho nao cresce mais
}

TEST_CASE("Hash: os baldes dividem a largura, e a lista desce do balde") {
    CHECK(Hash::CentroDoBalde(0, 7, 1400.0f) == doctest::Approx(100.0f));
    CHECK(Hash::CentroDoBalde(6, 7, 1400.0f) == doctest::Approx(1300.0f));
    CHECK(Hash::AlturaNaLista(0) == doctest::Approx(Hash::kYDosBaldes + Hash::kPrimeiraChave));
    CHECK(Hash::AlturaNaLista(2) - Hash::AlturaNaLista(1) == doctest::Approx(Hash::kEntreChaves));
}

TEST_CASE("Hash: a tabela vazia insere, a chave inserida e nova, e a busca acha ou nao como sorteou") {
    std::mt19937 g(2026);
    std::vector<int> tabela;
    int buscas = 0, achadas = 0;
    for (int i = 0; i < 3000; ++i) {
        const auto op = Hash::Proxima(tabela, static_cast<std::uint32_t>(g()));
        REQUIRE(op.chave >= 0);
        REQUIRE(op.chave <= Hash::kMaiorChave);
        const bool esta = std::find(tabela.begin(), tabela.end(), op.chave) != tabela.end();
        if (tabela.empty()) REQUIRE(op.tipo == Hash::Tipo::Inserir);
        if (op.tipo == Hash::Tipo::Inserir) {
            REQUIRE_FALSE(esta);                    // nao ha duas iguais
            tabela.push_back(op.chave);
            if (tabela.size() > 40) tabela.erase(tabela.begin());
        } else {
            ++buscas;
            if (esta) ++achadas;
        }
    }
    CHECK(static_cast<float>(achadas) / static_cast<float>(buscas) == doctest::Approx(Hash::kChanceDeAchar).epsilon(0.1));
    CHECK(Hash::CaraDaChave(23) == "K23");
    CHECK(Hash::CaraDoBalde(4) == "B4");
}

TEST_CASE("Hash: a tabela sorteada abre com 7 ou 11 baldes, e o rehash segue a lista dali") {
    std::set<int> tamanhos;
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s(semente);
        const Hash::Plano p = Hash::Sortear(s);
        tamanhos.insert(p.tamanhoInicial);
        CHECK(p.chanceDeInserir >= 0.45f);
        CHECK(p.chanceDeInserir < 0.65f);
        CHECK(Hash::ProximoTamanho(p.tamanhoInicial) > p.tamanhoInicial);
        // TODA CHAVE INSERIDA E NOVA, com qualquer plano.
        std::vector<int> naTabela;
        std::mt19937 g(semente);
        for (int i = 0; i < 40; ++i) {
            const auto op = Hash::Proxima(naTabela, static_cast<std::uint32_t>(g()), p);
            if (op.tipo != Hash::Tipo::Inserir) continue;
            CHECK(std::find(naTabela.begin(), naTabela.end(), op.chave) == naTabela.end());
            naTabela.push_back(op.chave);
        }
    }
    CHECK(tamanhos == std::set<int>{Hash::kTamanhos[0], Hash::kTamanhos[1]});
    const Hash::Degrau n = Hash::DegrauDo(Dificuldade::Nivel::Normal);
    const Hash::Degrau d = Hash::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.passo == doctest::Approx(Hash::kPasso));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(d.passo < n.passo);
    CHECK(d.rapidez > n.rapidez);
}
