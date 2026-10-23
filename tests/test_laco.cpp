//
// Os lacos da fase 2 do Andre.
//

#include "doctest.h"

#include <algorithm>
#include <set>

#include "../Source/Attacks/Fase.h"
#include "../Source/Attacks/Laco.h"

using namespace Laco;

namespace {
    std::vector<int> Casas(const std::vector<Visita>& v, const Tipo tipo = Tipo::Balao) {
        std::vector<int> casas;
        for (const auto& x : v) if (x.tipo == tipo) casas.push_back(x.i);
        return casas;
    }
}

TEST_CASE("Laco: o for anda do inicio ao fim, de passo em passo, um passo a cada kPasso") {
    const auto v = Execucao({Forma::Para, 1, 3, true}, 0u);
    CHECK(Casas(v) == std::vector<int>{1, 4, 7, 10});
    for (size_t k = 0; k < v.size(); ++k) CHECK(v[k].quando == doctest::Approx(kPasso * static_cast<float>(k)));
    CHECK(Duracao({Forma::Para, 1, 3, true}, v) == doctest::Approx(4.0f * kPasso));

    CHECK(Casas(Execucao({Forma::Decrescente, 11, 3, true}, 0u)) == std::vector<int>{11, 8, 5, 2});
    CHECK(Casas(Execucao({Forma::Multiplica, 1, 2, true}, 0u)) == std::vector<int>{1, 2, 4, 8});
    CHECK(Casas(Execucao({Forma::Multiplica, 1, 3, true}, 0u)) == std::vector<int>{1, 3, 9});
}

TEST_CASE("Laco: o continue passa por todas as casas, e as do continue ficam sem balao") {
    const auto v = Execucao({Forma::Continue, 0, 3, true}, 0u);
    CHECK(v.size() == static_cast<size_t>(kCasas));
    CHECK(Casas(v, Tipo::Pulado) == std::vector<int>{0, 3, 6, 9});
    CHECK(Casas(v).size() == 8);
}

TEST_CASE("Laco: o aninhado faz uma passada por k, e juntas cobrem o vetor uma vez") {
    for (const int passo : {3, 4}) {
        CAPTURE(passo);
        const auto v = Execucao({Forma::Aninhado, 0, passo, true}, 0u);
        auto casas = Casas(v);
        CHECK(casas.size() == static_cast<size_t>(kCasas));
        // A PASSADA k E A DAS CASAS COM RESTO k: o aluno muda de casa uma vez
        // por passada.
        for (size_t j = 0; j < casas.size(); ++j) CHECK(casas[j] % passo == static_cast<int>(j) / (kCasas / passo));
        std::sort(casas.begin(), casas.end());
        for (int i = 0; i < kCasas; ++i) CHECK(casas[static_cast<size_t>(i)] == i);
        // E HA UMA PAUSA entre as passadas.
        CHECK(v[static_cast<size_t>(kCasas / passo)].quando - v[static_cast<size_t>(kCasas / passo) - 1].quando ==
              doctest::Approx(kPasso + kEntrePassadas));
    }
}

TEST_CASE("Laco: o <= passa do fim - a ultima visita e v[12] (ou v[-1]), fora do vetor") {
    for (const int passo : {3, 4, 6}) {
        CAPTURE(passo);
        const auto d = Execucao({Forma::ForaDoVetor, 0, passo, true}, 0u);
        REQUIRE_FALSE(d.empty());
        CHECK(d.back().i == kCasas);
        CHECK(d.back().tipo == Tipo::Fora);
        CHECK(Casas(d, Tipo::Fora).size() == 1);
        const auto e = Execucao({Forma::ForaDoVetor, kCasas - 1, passo, false}, 0u);
        CHECK(e.back().i == -1);
        CHECK(e.back().tipo == Tipo::Fora);
        CHECK(Duracao({Forma::ForaDoVetor, 0, passo, true}, d) == doctest::Approx(d.back().quando + kCapotando));
    }
    CHECK(Placar({Forma::ForaDoVetor, 0, 4, true}).find("<= 12") != std::string::npos);
}

TEST_CASE("Laco: o while (1) sorteia casas ate o limite de tempo, sem repetir a anterior") {
    for (std::uint32_t s = 0; s < 50; ++s) {
        const auto v = Execucao({Forma::Infinito, 0, 1, true}, s * 2654435761u);
        CHECK(v.size() >= 7);
        for (size_t k = 0; k < v.size(); ++k) {
            CHECK(v[k].i >= 0);
            CHECK(v[k].i < kCasas);
            CHECK(v[k].quando < kLimiteDeTempo);
            if (k > 0) CHECK(v[k].i != v[k - 1].i);
        }
        CHECK(Duracao({Forma::Infinito, 0, 1, true}, v) == doctest::Approx(kLimiteDeTempo));
    }
}

TEST_CASE("Laco: todo laco sorteado fica dentro do vetor - menos o que e para sair -, e deixa casa sem balao") {
    for (int vez = 0; vez < 3 * kFormas; ++vez) {
        for (std::uint32_t s = 0; s < 60; ++s) {
            const Programa p = Proximo(vez, s * 7919u + static_cast<std::uint32_t>(vez));
            const auto v = Execucao(p, s);
            CAPTURE(Placar(p));
            REQUIRE_FALSE(v.empty());
            std::set<int> comBalao;
            for (const auto& x : v) {
                if (x.tipo == Tipo::Fora) {
                    CHECK(p.forma == Forma::ForaDoVetor);
                    continue;
                }
                CHECK(x.i >= 0);
                CHECK(x.i < kCasas);
                if (x.tipo == Tipo::Balao) comBalao.insert(x.i);
            }
            // SEMPRE HA ONDE FICAR - menos no aninhado, cujas passadas cobrem
            // o vetor uma vez cada, e no while (1), que nao tem padrao.
            if (p.forma != Forma::Aninhado && p.forma != Forma::Infinito) {
                CHECK(static_cast<int>(comBalao.size()) < kCasas);
            }
            CHECK(p.forma == [vez] {
                const Forma ordem[] = {Forma::Para, Forma::Decrescente, Forma::Multiplica, Forma::Continue,
                                       Forma::Aninhado, Forma::ForaDoVetor, Forma::Infinito};
                return ordem[vez % kFormas];
            }());
            if (p.forma == Forma::ForaDoVetor) CHECK(v.back().tipo == Tipo::Fora);
        }
    }
}

TEST_CASE("Laco: os sete lacos cabem na fase, ate com os numeros mais compridos") {
    float total = kPrimeiro;
    for (int vez = 0; vez < kFormas; ++vez) {
        float pior = 0.0f;
        for (std::uint32_t s = 0; s < 400; ++s) {
            const Programa p = Proximo(vez, s * 104729u);
            pior = std::max(pior, Duracao(p, Execucao(p, s)));
        }
        total += pior + (vez + 1 < kFormas ? kEntreLacos : 0.0f);
    }
    CHECK(total <= Fase::kDuracao);
}

TEST_CASE("Laco: as casas dividem o campo, e v[-1] e v[12] ficam fora dele") {
    CHECK(XDaCasa(0, 1200.0f) == doctest::Approx(50.0f));
    CHECK(XDaCasa(kCasas - 1, 1200.0f) == doctest::Approx(1150.0f));
    CHECK(XDaCasa(-1, 1200.0f) < 0.0f);
    CHECK(XDaCasa(kCasas, 1200.0f) > 1200.0f);
    std::set<std::string> caras;
    for (int i = 0; i < kCasas; ++i) {
        caras.insert(CaraDoIndice(i));
        caras.insert(CaraDoIndiceAceso(i));
        caras.insert(CaraDoIndicePulado(i));
        caras.insert(CaraDoIndiceErro(i));
    }
    CHECK(static_cast<int>(caras.size()) == 4 * kCasas);
}
