//
// A repescagem do Lucas: o event loop do JavaScript.
//

#include "../../doctest.h"

#include <cstdint>
#include <map>
#include <set>

#include "../../../Source/Attacks/Lucas/EventLoop.h"

using namespace EventLoop;

TEST_CASE("EventLoop: o setTimeout de 0 ms escrito antes do .then roda depois dele") {
    const std::vector<Linha> script = {
        {Tipo::Log, 0.0f, "oi"},
        {Tipo::Timeout, 0.0f, "cb1"},
        {Tipo::Then, 0.0f, "p1"},
    };
    const Agenda a = Simular(script, 0.5f, 0.5f);
    REQUIRE(a.itens.size() == 2u);
    const Item& cb = a.itens[0];
    const Item& p = a.itens[1];
    CHECK(cb.fila == Fila::Tarefa);
    CHECK(p.fila == Fila::Micro);
    CHECK(p.rodou < cb.rodou);
    // NADA RODA ANTES DE O SCRIPT ACABAR: a pilha so esvazia no fim dele.
    CHECK(p.rodou >= 1.5f - 1e-4f);
}

TEST_CASE("EventLoop: depois de cada tarefa, todas as microtarefas prontas antes da proxima tarefa") {
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoLoop);
        const auto script = Gerar(s);
        CAPTURE(semente);
        REQUIRE(script.size() >= 5u);
        CHECK(script.size() <= 6u);
        const Agenda a = Simular(script, kPorLinha, kPorCallback);
        // TODO ITEM RODA, uma vez, depois de entrar na fila.
        for (const Item& it : a.itens) {
            CHECK(it.rodou >= it.naFila - 1e-4f);
            CHECK(it.naFila >= it.registrado - 1e-4f);
        }
        // UMA COISA POR VEZ NA PILHA.
        for (size_t k = 1; k < a.execucoes.size(); ++k) CHECK(a.execucoes[k].inicio >= a.execucoes[k - 1].inicio);
        // A REGRA DO LOOP: quando uma tarefa roda, nenhuma microtarefa pronta
        // esta esperando.
        for (const Item& tarefa : a.itens) {
            if (tarefa.fila != Fila::Tarefa) continue;
            for (const Item& micro : a.itens) {
                if (micro.fila != Fila::Micro) continue;
                if (micro.naFila <= tarefa.rodou + 1e-4f) CHECK(micro.rodou <= tarefa.rodou + 1e-4f);
            }
        }
        CHECK(a.fim <= 9.0f);   // cabe numa rodada da fase
    }
}

TEST_CASE("EventLoop: o placar e o codigo de verdade, e todo script tem a pegadinha") {
    CHECK(Codigo({Tipo::Timeout, 0.0f, "cb1"}) == "setTimeout(cb1, 0)");
    CHECK(Codigo({Tipo::Then, 0.0f, "p2"}) == "Promise.resolve().then(p2)");
    CHECK(Codigo({Tipo::Fetch, 0.8f, "r1"}) == "fetch(\"/api\").then(r1)");
    CHECK(Codigo({Tipo::Log, 0.0f, "oi"}) == "console.log(\"oi\")");
    CHECK(Placar({Tipo::Timeout, 0.0f, "cb1"}) == "setTimeout(cb1, 0)\ntimer nas Web APIs");
    CHECK(Placar({Tipo::Then, 0.0f, "p1"}) == "Promise.resolve().then(p1)\np1 na fila de microtarefas");
    CHECK(PlacarDoCallback({Tipo::Timeout, 0.0f, "cb1"}) == "cb1()\nda fila de tarefas");
    // A RESPOSTA DO FETCH E UMA PROMESSA: o callback dela e microtarefa.
    CHECK(PlacarDoCallback({Tipo::Fetch, 0.8f, "r1"}) == "r1()\nda fila de microtarefas");
    std::set<std::string> scripts;
    for (std::uint32_t semente = 0; semente < 50; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoLoop);
        const auto script = Gerar(s);
        CHECK(script[1].tipo == Tipo::Timeout);
        CHECK(script[1].atraso == 0.0f);
        CHECK(script[2].tipo == Tipo::Then);
        std::string todo;
        for (const Linha& l : script) todo += Codigo(l);
        scripts.insert(todo);
    }
    CHECK(scripts.size() > 30u);
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).porLinha < kPorLinha);
}

TEST_CASE("EventLoop: a ficha anda pilha, Web APIs, fila e pilha, uma por vez em cada lugar") {
    const std::vector<Linha> pegadinha = {
        {Tipo::Log, 0.0f, "oi"},
        {Tipo::Timeout, 0.0f, "cb1"},
        {Tipo::Then, 0.0f, "p1"},
    };
    const Agenda curta = Simular(pegadinha, 0.5f, 0.5f);
    // Aos 1,2 s o .then ainda roda na pilha, e o callback do timer ja espera
    // na fila de tarefas.
    CHECK(Onde(pegadinha, curta, 2, 1.2f, 0.5f, 0.5f).lugar == Lugar::Pilha);
    CHECK(Onde(pegadinha, curta, 1, 1.2f, 0.5f, 0.5f).lugar == Lugar::Tarefa);
    CHECK(Onde(pegadinha, curta, 0, 1.2f, 0.5f, 0.5f).lugar == Lugar::Nenhum);
    // Aos 1,7 s o p1 roda, e o cb1 ainda espera.
    CHECK(Onde(pegadinha, curta, 2, 1.7f, 0.5f, 0.5f).lugar == Lugar::Pilha);
    CHECK(Onde(pegadinha, curta, 1, 1.7f, 0.5f, 0.5f).lugar == Lugar::Tarefa);

    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDoLoop);
        const auto script = Gerar(s);
        const Agenda a = Simular(script, kPorLinha, kPorCallback);
        CAPTURE(semente);
        for (float t = 0.0f; t < a.fim + 1.0f; t += 0.05f) {
            CAPTURE(t);
            int naPilha = 0;
            std::set<std::pair<int, int>> ocupados;
            std::map<int, int> naFila;
            for (int i = 0; i < static_cast<int>(script.size()); ++i) {
                const Estado e = Onde(script, a, i, t, kPorLinha, kPorCallback);
                if (e.lugar == Lugar::Nenhum) continue;
                if (e.lugar == Lugar::Pilha) { ++naPilha; continue; }
                // DUAS FICHAS NUNCA DIVIDEM UMA VAGA.
                CHECK(ocupados.insert({static_cast<int>(e.lugar), e.posicao}).second);
                if (e.lugar == Lugar::Apis) {
                    CHECK(e.falta > 0.0f);
                    CHECK(e.falta <= 1.0f);
                    CHECK(e.posicao < 4);
                } else {
                    ++naFila[static_cast<int>(e.lugar)];
                }
            }
            CHECK(naPilha <= 1);
            // A FILA NAO TEM BURACO: as posicoes vao de 0 a n - 1.
            for (const auto& [fila, n] : naFila) {
                for (int p = 0; p < n; ++p) CHECK(ocupados.count({fila, p}) == 1u);
            }
            if (t > a.fim + kPorCallback) CHECK(ocupados.empty());
        }
    }
    // As vagas cabem nas caixas.
    const auto caixas = Caixas(1200.0f);
    const Ponto ultima = Posicao(Lugar::Apis, 3, 1200.0f);
    CHECK(ultima.x < caixas[1].x + caixas[1].w);
    const Ponto cauda = Posicao(Lugar::Micro, 5, 1200.0f);
    CHECK(cauda.x > caixas[2].x);
}
