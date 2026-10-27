//
// A fase 1 do Lucas: um documento HTML sendo escrito, tag por tag.
//

#include "doctest.h"

#include <cstdint>
#include <set>
#include <string>

#include "../Source/Attacks/Dom.h"

using namespace Dom;

TEST_CASE("Dom: cada documento e HTML bem formado - toda tag aberta e fechada, na ordem certa") {
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoDom);
        const Documento d = Gerar(s, 1200.0f);
        REQUIRE(d.elementos.size() >= 3u);
        CHECK(static_cast<int>(d.elementos.size()) <= kMaisElementos);
        // A PILHA DO PARSER: toda </x> fecha o ultimo <x> aberto, e no fim
        // nao sobra nada aberto.
        std::vector<int> abertos;
        int vazios = 0;
        for (const Evento& e : Eventos(d)) {
            // O FILHO SO ABRE DENTRO DO PAI: o pai e o ultimo aberto.
            const int pai = d.elementos[static_cast<size_t>(e.elemento)].pai;
            if (e.tipo != Tipo::Fecha && pai >= 0) {
                REQUIRE_FALSE(abertos.empty());
                CHECK(abertos.back() == pai);
            }
            if (e.tipo == Tipo::Abre) abertos.push_back(e.elemento);
            if (e.tipo == Tipo::Fecha) {
                REQUIRE_FALSE(abertos.empty());
                CHECK(abertos.back() == e.elemento);
                abertos.pop_back();
            }
            if (e.tipo == Tipo::Vazio) {
                CHECK(d.elementos[static_cast<size_t>(e.elemento)].vazio);
                ++vazios;
            }
        }
        CHECK(abertos.empty());
        // CADA ELEMENTO SAI UMA VEZ: duas tags se tem conteudo, uma se e vazio.
        CHECK(Eventos(d).size() == 2u * d.elementos.size() - static_cast<size_t>(vazios));
        // E O VAZIO NAO TEM FILHOS.
        for (const Elemento& e : d.elementos) if (e.vazio) CHECK(e.filhos.empty());
    }
}

TEST_CASE("Dom: o filho fica embaixo do pai, dentro do campo, e a caixa dele nasce menor") {
    std::set<std::string> raizes;
    for (std::uint32_t semente = 0; semente < 100; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoDom);
        const Documento d = Gerar(s, 1200.0f);
        raizes.insert(d.elementos[0].tag);
        for (const Elemento& e : d.elementos) {
            CHECK(e.centro.x >= kMargem);
            CHECK(e.centro.x <= 1200.0f - kMargem);
            if (e.pai < 0) continue;
            const Elemento& pai = d.elementos[static_cast<size_t>(e.pai)];
            CHECK(e.centro.y == doctest::Approx(pai.centro.y + kEntreGeracoes));
            CHECK(Encolher(e.profundidade) < Encolher(pai.profundidade));
        }
    }
    CHECK(raizes.size() == 4u);   // div, main, section, article: todas aparecem
}

TEST_CASE("Dom: o placar e a tag e o caminho dela no DOM") {
    Documento d;
    d.elementos.push_back({"main", -1, 0, false, {1}, {}});
    d.elementos.push_back({"ul", 0, 1, false, {2}, {}});
    d.elementos.push_back({"li", 1, 2, false, {}, {}});
    CHECK(Placar(d, {Tipo::Abre, 2}) == "<li>\nmain > ul > li");
    CHECK(Tag(d, {Tipo::Fecha, 1}) == "</ul>");
    CHECK(Caminho(d, 0) == "main");
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).porEvento < DegrauDo(Dificuldade::Nivel::Normal).porEvento);
}
