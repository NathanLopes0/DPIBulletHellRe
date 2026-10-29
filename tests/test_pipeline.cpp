//
// O pipeline da fase 3 do Ricardo.
//

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <set>
#include <vector>
#include "../Source/Attacks/Engine/Fase.h"

#include "../Source/Attacks/Ricardo/Pipeline.h"

using Pipeline::Tipo;

namespace {
    Tipo TipoNo(const Pipeline::Estado& e, const int s) {
        return Pipeline::Programa()[static_cast<size_t>(e.estagio[s])].tipo;
    }
}

TEST_CASE("Pipeline: cada ciclo todo mundo anda um estagio, e uma instrucao entra no IF") {
    Pipeline::Estado e;
    Pipeline::Avancar(e);
    CHECK(e.estagio[Pipeline::IF] == 0);
    CHECK(e.estagio[Pipeline::ID] == Pipeline::kBolha);
    Pipeline::Avancar(e);
    CHECK(e.estagio[Pipeline::IF] == 1);
    CHECK(e.estagio[Pipeline::ID] == 0);
    Pipeline::Avancar(e);
    Pipeline::Avancar(e);
    const auto c = Pipeline::Avancar(e);
    CHECK(c.saiu == Pipeline::kBolha);           // o pipeline ainda enchia
    CHECK(e.estagio[Pipeline::WB] == 0);         // o lw chegou ao fim
    CHECK(Pipeline::Avancar(e).saiu == 0);       // e sai
}

TEST_CASE("Pipeline: o bne e previsto tomado - a busca volta ao comeco do laco sem esperar") {
    Pipeline::Estado e;
    for (int k = 0; k < 4; ++k) Pipeline::Avancar(e);   // lw add addi bne
    CHECK(TipoNo(e, Pipeline::IF) == Tipo::Bne);
    Pipeline::Avancar(e);
    CHECK(TipoNo(e, Pipeline::IF) == Tipo::Lw);         // previsto: desvia
}

TEST_CASE("Pipeline: na ultima volta a previsao erra - flush, duas bolhas, e a busca segue do sw") {
    Pipeline::Estado e;
    int flushes = 0, ciclo = 0, flushNo = -1;
    Pipeline::Estado antes;
    for (; ciclo < 40 && flushes == 0; ++ciclo) {
        antes = e;
        if (Pipeline::Avancar(e).flush) { ++flushes; flushNo = ciclo; }
    }
    REQUIRE(flushes == 1);
    CHECK(TipoNo(e, Pipeline::EX) == Tipo::Bne);
    CHECK(e.estagio[Pipeline::ID] == Pipeline::kBolha);
    CHECK(e.estagio[Pipeline::IF] == Pipeline::kBolha);
    // tres voltas de quatro instrucoes, e o bne da terceira chega ao EX dois
    // ciclos depois de buscado
    CHECK(flushNo == 3 * 4 - 1 + 2);

    Pipeline::Avancar(e);
    CHECK(TipoNo(e, Pipeline::IF) == Tipo::Sw);
    Pipeline::Avancar(e);
    CHECK(TipoNo(e, Pipeline::IF) == Tipo::J);
    Pipeline::Avancar(e);
    CHECK(TipoNo(e, Pipeline::IF) == Tipo::Lw);         // o j recomeca o programa
}

TEST_CASE("Pipeline: o programa recomeca e erra a previsao de novo, uma vez por execucao") {
    Pipeline::Estado e;
    int flushes = 0;
    for (int k = 0; k < 60; ++k) flushes += Pipeline::Avancar(e).flush ? 1 : 0;
    // uma execucao: 3 voltas de 4, o sw e o j, e as 2 bolhas do flush = 16 ciclos
    CHECK(flushes == 60 / 16);
}

TEST_CASE("Pipeline: a instrucao derrubada vira bolha, e a bolha anda como as outras") {
    Pipeline::Estado e;
    for (int k = 0; k < 3; ++k) Pipeline::Avancar(e);   // lw no EX
    CHECK(TipoNo(e, Pipeline::EX) == Tipo::Lw);
    Pipeline::Estourar(e, Pipeline::ID);                // o add
    CHECK(e.estagio[Pipeline::ID] == Pipeline::kBolha);
    Pipeline::Avancar(e);
    CHECK(e.estagio[Pipeline::EX] == Pipeline::kBolha);
    CHECK(TipoNo(e, Pipeline::MEM) == Tipo::Lw);
    Pipeline::Estourar(e, 9);                           // estagio que nao ha: nada
}

TEST_CASE("Pipeline: cada instrucao ataca no estagio dela") {
    CHECK(Pipeline::Gatilho(Tipo::Add, Pipeline::EX) == "add");
    CHECK(Pipeline::Gatilho(Tipo::Addi, Pipeline::EX) == "addi");
    CHECK(Pipeline::Gatilho(Tipo::Lw, Pipeline::MEM) == "lw");
    CHECK(Pipeline::Gatilho(Tipo::Sw, Pipeline::MEM) == "sw");
    CHECK(Pipeline::Gatilho(Tipo::Lw, Pipeline::EX).empty());     // o load calcula o endereco no EX
    CHECK(Pipeline::Gatilho(Tipo::Add, Pipeline::MEM).empty());
    CHECK(Pipeline::Gatilho(Tipo::Bne, Pipeline::EX).empty());
    CHECK(Pipeline::Gatilho(Tipo::J, Pipeline::ID).empty());
    CHECK(std::string(Pipeline::NomeDoEstagio(Pipeline::MEM)) == "MEM");
}

TEST_CASE("Pipeline: com 2, 3 ou 4 voltas o flush vem depois da ultima, e o programa recomeca") {
    std::set<int> vistas;
    for (std::uint32_t semente = 0; semente < 100; ++semente) {
        Variacao::Sorteio s(semente);
        const Pipeline::Plano p = Pipeline::Sortear(s);
        CHECK(p.voltas >= Pipeline::kMenosVoltas);
        CHECK(p.voltas <= Pipeline::kMaisVoltas);
        vistas.insert(p.voltas);
    }
    CHECK(vistas == std::set<int>{2, 3, 4});
    for (int voltas = Pipeline::kMenosVoltas; voltas <= Pipeline::kMaisVoltas; ++voltas) {
        CAPTURE(voltas);
        Pipeline::Estado e;
        e.voltas = voltas;
        int ciclo = 0, flushNo = -1;
        for (; ciclo < 60; ++ciclo) {
            if (Pipeline::Avancar(e).flush) { flushNo = ciclo; break; }
        }
        // 'voltas' voltas de quatro instrucoes, e o bne da ultima chega ao EX
        // dois ciclos depois de buscado.
        CHECK(flushNo == voltas * 4 - 1 + 2);
        CHECK(TipoNo(e, Pipeline::EX) == Tipo::Bne);
        // UMA EXECUCAO INTEIRA cabe na fase, com o ciclo de qualquer modo.
        const float execucao = static_cast<float>(voltas * 4 + 2 + 2) *
                               Pipeline::DegrauDo(Dificuldade::Nivel::Normal).ciclo;
        CHECK(Pipeline::kPrimeiroCiclo + execucao < Fase::kDuracao);
    }
    const auto n = Pipeline::DegrauDo(Dificuldade::Nivel::Normal);
    const auto d = Pipeline::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.ciclo == doctest::Approx(Pipeline::kCiclo));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(d.ciclo < n.ciclo);
    CHECK(d.rapidez > n.rapidez);
}
