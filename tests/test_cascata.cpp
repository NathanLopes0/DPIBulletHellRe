//
// O modelo cascata da fase 1 da Maria Lucia.
//

#include "doctest.h"

#include <cstdint>
#include <set>


#include "../Source/Attacks/MariaLucia/Cascata.h"

TEST_CASE("Cascata: as etapas vem em ordem, uma a cada 4 s, e cabem na fase") {
    CHECK(Cascata::kDuracaoDaEtapa == doctest::Approx(4.0f));
    CHECK(Cascata::kDuracaoDaEtapa * static_cast<float>(Cascata::kEtapas.size()) == doctest::Approx(Fase::kDuracao));
    CHECK(Cascata::EtapaNoTempo(0.0f) == 0);
    CHECK(Cascata::EtapaNoTempo(3.99f) == 0);
    CHECK(Cascata::EtapaNoTempo(4.01f) == 1);
    CHECK(Cascata::EtapaNoTempo(16.1f) == 4);
    CHECK(Cascata::EtapaNoTempo(20.0f) == 5);
    CHECK(Cascata::EtapaNoTempo(100.0f) == 5);
    CHECK(Cascata::EtapaNoTempo(-1.0f) == 0);
    int antes = 0;
    for (float t = 0.0f; t < 20.0f; t += 0.05f) {
        CHECK(Cascata::EtapaNoTempo(t) >= antes);
        antes = Cascata::EtapaNoTempo(t);
    }
}

TEST_CASE("Cascata: o backlog tem um lugar para cada pedido, sem dois no mesmo") {
    std::set<std::pair<int, int>> lugares;
    for (int k = 0; k < Cascata::kMaxPedidos; ++k) {
        const Vector2 l = Cascata::LugarNoBacklog(k);
        lugares.insert({static_cast<int>(l.x), static_cast<int>(l.y)});
        CHECK(l.x > 0.0f);
        CHECK(l.y > 62.0f);    // abaixo do placar
    }
    CHECK(static_cast<int>(lugares.size()) == Cascata::kMaxPedidos);
}

TEST_CASE("Cascata: o pedido vem de fora, por um dos lados, da metade de baixo") {
    bool esquerda = false, direita = false;
    for (std::uint32_t s = 0; s < 400; ++s) {
        const Vector2 o = Cascata::OrigemDoPedido(s * 2654435761u, 1200.0f, 746.0f);
        CHECK((o.x < 0.0f || o.x > 1200.0f));
        CHECK(o.y >= 746.0f * 0.45f);
        CHECK(o.y <= 746.0f - 60.0f);
        esquerda = esquerda || o.x < 0.0f;
        direita = direita || o.x > 1200.0f;
    }
    CHECK(esquerda);
    CHECK(direita);
}

TEST_CASE("Cascata: as classes ficam numa grade, e as setas ligam vizinhas") {
    std::set<std::pair<int, int>> lugares;
    for (int k = 0; k < Cascata::kMaxClasses; ++k) {
        const Vector2 l = Cascata::LugarDaClasse(k, 1200.0f);
        lugares.insert({static_cast<int>(l.x), static_cast<int>(l.y)});
        CHECK(l.x > 0.0f);
        CHECK(l.x < 1200.0f);
    }
    CHECK(static_cast<int>(lugares.size()) == Cascata::kMaxClasses);

    // Oito classes: tres setas em cada fila e quatro descendo.
    const auto setas = Cascata::Associacoes(8);
    CHECK(setas.size() == 10);
    for (const auto& [de, para] : setas) {
        const Vector2 a = Cascata::LugarDaClasse(de, 1200.0f);
        const Vector2 b = Cascata::LugarDaClasse(para, 1200.0f);
        // SEMPRE EM ANGULO RETO: na mesma fila ou na mesma coluna.
        CHECK((a.x == doctest::Approx(b.x) || a.y == doctest::Approx(b.y)));
        CHECK(para < 8);
    }
    CHECK(Cascata::Associacoes(1).empty());
    CHECK(Cascata::Associacoes(2).size() == 1);
    CHECK(Cascata::Associacoes(0).empty());
    // Com cinco, a quinta fica embaixo da primeira e so liga a ela.
    const auto cinco = Cascata::Associacoes(5);
    CHECK(cinco.size() == 4);
}

TEST_CASE("Cascata: a ordem das classes e sorteada por luta, e cada classe que existe tem a vez uma vez por volta") {
    std::set<int> primeiras;
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, Cascata::kFaseDaCascata);
        const Cascata::Plano p = Cascata::Sortear(s);
        REQUIRE(p.ordem.size() == static_cast<size_t>(Cascata::kMaxClasses));
        CHECK(std::set<int>(p.ordem.begin(), p.ordem.end()).size() == static_cast<size_t>(Cascata::kMaxClasses));
        primeiras.insert(Cascata::DaVez(p, 0, Cascata::kMaxClasses));

        // O ALUNO CORTOU ESCOPO: com 1 a 8 classes, uma volta passa por todas,
        // sem repetir, e a volta seguinte repete a ordem.
        for (int prontas = 1; prontas <= Cascata::kMaxClasses; ++prontas) {
            std::set<int> vistas;
            for (int vez = 0; vez < prontas; ++vez) {
                const int k = Cascata::DaVez(p, vez, prontas);
                REQUIRE(k >= 0);
                REQUIRE(k < prontas);
                vistas.insert(k);
                CHECK(Cascata::DaVez(p, vez + prontas, prontas) == k);
            }
            CHECK(vistas.size() == static_cast<size_t>(prontas));
        }
    }
    // A PRIMEIRA A FALAR NAO E SEMPRE A DA ESQUERDA.
    CHECK(primeiras.size() == static_cast<size_t>(Cascata::kMaxClasses));
}

TEST_CASE("Cascata: sem plano, a vez e a de sempre, da esquerda para a direita") {
    const Cascata::Plano vazio;
    for (int vez = 0; vez < 20; ++vez) CHECK(Cascata::DaVez(vazio, vez, 5) == vez % 5);
    CHECK(Cascata::DaVez(vazio, 3, 0) == 0);
}
