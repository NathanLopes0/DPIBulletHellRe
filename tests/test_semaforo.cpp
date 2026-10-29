//
// A fase 1 do Vitor: os semaforos.
//

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "../Source/Attacks/Vitor/Semaforo.h"

using namespace Semaforo;

namespace {
    /// Roda o semaforo a 60 quadros por segundo pela rodada inteira e devolve
    /// os eventos, com o instante de cada um. Confere a cada quadro que nunca
    /// ha mais gente dentro do que permissoes.
    struct Visto { Evento e; float u; };
    std::vector<Visto> Rodar(Escalonador& e, const float ate = kPorRodada) {
        std::vector<Visto> vistos;
        for (float u = 0.0f; u <= ate; u += 1.0f / 60.0f) {
            for (const Evento& ev : Avancar(e, u)) vistos.push_back({ev, u});
            if (e.rodada.tipo != Tipo::Corrida) {
                CHECK(static_cast<int>(e.dentro.size()) <= e.rodada.permissoes);
            }
        }
        return vistos;
    }
}

TEST_CASE("Semaforo: as colunas - uma em cima do aluno, as outras na grade, dentro do campo") {
    for (const float x : {600.0f, 21.0f, 1179.0f, 333.3f, 0.0f, 1200.0f, 845.0f}) {
        CAPTURE(x);
        const auto xs = Colunas(x, 1200.0f);
        REQUIRE(xs.size() >= 4u);
        CHECK(static_cast<int>(xs.size()) <= kMaisThreads);
        const float alvo = std::clamp(x, kMargem, 1200.0f - kMargem);
        bool emCima = false;
        for (const float c : xs) emCima = emCima || std::fabs(c - alvo) < 0.01f;
        CHECK(emCima);
        CHECK(xs.front() >= kMargem - 0.01f);
        CHECK(xs.back() <= 1200.0f - kMargem + 0.01f);
        for (size_t i = 1; i < xs.size(); ++i) CHECK(xs[i] - xs[i - 1] == doctest::Approx(kEntreThreads));
        CHECK(xs[static_cast<size_t>(MaisPerto(xs, x))] == doctest::Approx(alvo));
    }
    CHECK(Nome(0) == "T1");
    CHECK(Nome(4) == "T5");
    CHECK(MaisPerto({}, 100.0f) == -1);
}

TEST_CASE("Semaforo: o mutex deixa uma de cada vez, na ordem da fila, e todas passam a tempo") {
    Escalonador e = Comecar(RodadaDaVez(0), 5, 2, 12345u);
    REQUIRE(e.rodada.tipo == Tipo::Mutex);
    const std::vector<int> ordem = e.fila;
    CHECK(std::set<int>(ordem.begin(), ordem.end()).size() == 5u);

    const auto vistos = Rodar(e);
    std::vector<int> entraram;
    for (const Visto& v : vistos) {
        if (v.e.entra) {
            entraram.push_back(v.e.thread);
            CHECK(v.u >= kAbre - 0.001f);
        }
    }
    CHECK(entraram == ordem);          // FIFO
    CHECK(e.passaram.size() == 5u);    // todas sairam antes do fim da rodada
    CHECK(e.fila.empty());
    CHECK(kAbre + 5.0f * NaSecao(RodadaDaVez(0)) < kPorRodada);
}

TEST_CASE("Semaforo: o semaforo contador deixa duas de cada vez") {
    Escalonador e = Comecar(RodadaDaVez(1), 5, 0, 7u);
    REQUIRE(e.rodada.permissoes == 2);
    CHECK(Valor(e) == 2);
    Avancar(e, kAbre);
    CHECK(e.dentro.size() == 2u);
    CHECK(Valor(e) == 0);
    Rodar(e);
    CHECK(e.passaram.size() == 5u);
    CHECK(kAbre + 3.0f * NaSecao(RodadaDaVez(1)) < kPorRodada);
}

TEST_CASE("Semaforo: a thread do aluno chega entre as tres primeiras") {
    for (std::uint32_t s = 1; s < 200; ++s) {
        for (int doAluno = 0; doAluno < 5; ++doAluno) {
            const Escalonador e = Comecar(RodadaDaVez(0), 5, doAluno, s);
            const auto it = std::find(e.fila.begin(), e.fila.end(), doAluno);
            REQUIRE(it != e.fila.end());
            CHECK(it - e.fila.begin() < 3);
        }
    }
}

TEST_CASE("Semaforo: na espera ocupada entra quem ganha o test-and-set, nao a primeira da fila") {
    bool foraDeOrdem = false;
    for (std::uint32_t s = 1; s < 40; ++s) {
        Escalonador e = Comecar(RodadaDaVez(2), 5, 0, s);
        REQUIRE(e.rodada.tipo == Tipo::EsperaOcupada);
        CHECK(Proxima(e) == -1);   // o professor nao sabe quem e a proxima
        const std::vector<int> chegada = e.fila;
        std::vector<int> entraram;
        for (const Visto& v : Rodar(e)) if (v.e.entra) entraram.push_back(v.e.thread);
        CHECK(entraram.size() == 5u);
        CHECK(std::set<int>(entraram.begin(), entraram.end()).size() == 5u);
        foraDeOrdem = foraDeOrdem || entraram != chegada;
    }
    CHECK(foraDeOrdem);
}

TEST_CASE("Semaforo: na corrida todas entram juntas, e o saldo sai errado") {
    Escalonador e = Comecar(RodadaDaVez(3), 5, 1, 3u);
    REQUIRE(e.rodada.tipo == Tipo::Corrida);
    CHECK(Placar(e, false) == "saldo++ \xc2\xb7 sem trava");
    const auto ev = Avancar(e, kAbre);
    CHECK(ev.size() == 5u);
    CHECK(e.dentro.size() == 5u);
    CHECK(e.fila.empty());
    Avancar(e, kAbre + kSaldoAparece);
    CHECK(Placar(e, false) == "saldo = 1 \xc2\xb7 esperado 5");
    CHECK(SaldoDaCorrida(5) == 1);
}

TEST_CASE("Semaforo: o aluno acorrentado faz a thread em cima dele furar a fila") {
    Escalonador e = Comecar(RodadaDaVez(0), 5, 0, 99u);
    Avancar(e, kAbre);
    REQUIRE(e.fila.size() == 4u);
    const int ultima = e.fila.back();
    CHECK(std::find(e.fila.begin(), e.fila.end(), ultima) != e.fila.end());   // esperando
    CHECK(Furar(e, ultima));
    CHECK(e.fila.front() == ultima);
    CHECK(Proxima(e) == ultima);
    CHECK_FALSE(Furar(e, ultima));                 // ja e a primeira
    CHECK_FALSE(Furar(e, e.dentro.front()));       // quem esta dentro nao fura
    // E E ELA QUEM ENTRA a seguir.
    std::vector<int> entraram;
    for (const Visto& v : Rodar(e)) if (v.e.entra) entraram.push_back(v.e.thread);
    REQUIRE(!entraram.empty());
    CHECK(entraram.front() == ultima);
}

TEST_CASE("Semaforo: a pilha cabe entre os nomes e a faixa, e a rajada abre para os dois lados") {
    CHECK(AlturaNaPilha(0) < kTopoDaSecao);
    CHECK(AlturaNaPilha(kMaiorPilha - 1) > kNasce);
    CHECK(kNasce > kAlturaDosNomes);
    CHECK(kAlturaDosNomes - 14.0f > 58.0f);          // abaixo do placar (Battle: 12 + 46)
    CHECK(kTopoDaSecao + kAlturaDaSecao < 400.0f);   // bem acima do aluno
    CHECK(AnguloNaRajada(0) == doctest::Approx(90.0f));
    for (int i = 1; i + 1 < kMaiorPilha; i += 2) {
        CHECK(AnguloNaRajada(i) - 90.0f == doctest::Approx(90.0f - AnguloNaRajada(i + 1)));
    }
    // O giro: todas no raio, espalhadas por igual.
    for (int n = 1; n <= kMaiorPilha; ++n) {
        for (int i = 0; i < n; ++i) {
            const Ponto p = NoGiro(i, n, 0.37f);
            CHECK(std::sqrt(p.x * p.x + p.y * p.y) == doctest::Approx(kRaioDoGiro));
        }
    }
    CHECK(kCentroDoGiro - kRaioDoGiro > kNasce);
    for (std::uint32_t s = 1; s < 100; ++s) CHECK(std::fabs(DesvioNaCorrida(s)) <= kMaiorDesvio + 0.001f);
}

TEST_CASE("Semaforo: as rodadas cabem na fase, e os textos") {
    CHECK(kPrimeiro + static_cast<float>(kRodadas) * kPorRodada == doctest::Approx(Fase::kDuracao));
    CHECK(RodadaDaVez(kRodadas).tipo == RodadaDaVez(0).tipo);
    // A ultima instrucao de uma thread sai antes de a rodada acabar.
    CHECK(kComecaAMandar + static_cast<float>(kInstrucoesPorThread - 1) * kEntreInstrucoes < kPorRodada);

    Escalonador e = Comecar(RodadaDaVez(0), 3, 0, 5u);
    e.fila = {2, 0, 1};
    CHECK(Placar(e, false) == "s = 1 \xc2\xb7 fila: T3 T1 T2");
    CHECK(Placar(e, true) == "aluno em P(s) \xc2\xb7 s = 1 \xc2\xb7 fila: T3 T1 T2");
    CHECK(CodigoDaSecao(Tipo::Mutex) == "P(s);  saldo++;  V(s);");
    Escalonador g = Comecar(RodadaDaVez(2), 2, 0, 5u);
    g.fila = {1};
    CHECK(Placar(g, false) == "espera ocupada \xc2\xb7 girando: T2");
}

// ---- A VARIACAO E O DEGRAU ---------------------------------------------------

TEST_CASE("Semaforo: o plano de sempre e a fase de sempre") {
    const Plano p = PlanoDeSempre();
    for (int k = 0; k < kRodadas; ++k) {
        CHECK(p.rodadas[static_cast<size_t>(k)].tipo == RodadaDaVez(k).tipo);
        CHECK(p.rodadas[static_cast<size_t>(k)].permissoes == RodadaDaVez(k).permissoes);
    }
    CHECK(p.entreThreads == doctest::Approx(kEntreThreads));
}

TEST_CASE("Semaforo: o mutex abre, as outras tres vem em qualquer ordem, e toda rodada sorteada fecha a tempo") {
    std::set<int> contadores;
    std::set<std::vector<int>> ordens;
    float menor = 1e9f, maior = 0.0f;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        CAPTURE(semente);
        Variacao::Sorteio s(semente);
        const Plano p = Sortear(s);
        CHECK(p.rodadas[0].tipo == Tipo::Mutex);
        CHECK(p.rodadas[0].permissoes == 1);
        std::vector<int> tipos;
        for (const Rodada& r : p.rodadas) tipos.push_back(static_cast<int>(r.tipo));
        ordens.insert(tipos);
        std::vector<int> ordenados = tipos;
        std::sort(ordenados.begin(), ordenados.end());
        CHECK(ordenados == std::vector<int>{0, 1, 2, 3});   // as quatro, uma vez cada

        for (const Rodada& r : p.rodadas) {
            if (r.tipo == Tipo::Contador) {
                CHECK(r.permissoes >= kMenorContador);
                CHECK(r.permissoes <= kMaiorContador);
                contadores.insert(r.permissoes);
            }
            // TODAS PASSAM antes de a rodada acabar (a corrida nao sai: e o ponto dela).
            if (r.tipo == Tipo::Corrida) continue;
            Escalonador e = Comecar(r, kMaisThreads, 0, semente + 1u);
            Rodar(e);
            CHECK(e.passaram.size() == static_cast<size_t>(kMaisThreads));
        }

        CHECK(p.entreThreads >= kMenorEntreThreads);
        CHECK(p.entreThreads < kMaiorEntreThreads);
        menor = std::min(menor, p.entreThreads);
        maior = std::max(maior, p.entreThreads);
        // COM QUALQUER PASSO as colunas ficam no campo, e uma em cima do aluno.
        for (const float x : {600.0f, 0.0f, 1200.0f, 845.0f}) {
            const auto xs = Colunas(x, 1200.0f, p.entreThreads);
            REQUIRE(xs.size() >= 4u);
            CHECK(static_cast<int>(xs.size()) <= kMaisThreads);
            CHECK(xs.front() >= kMargem - 0.01f);
            CHECK(xs.back() <= 1200.0f - kMargem + 0.01f);
            const float alvo = std::clamp(x, kMargem, 1200.0f - kMargem);
            CHECK(xs[static_cast<size_t>(MaisPerto(xs, x))] == doctest::Approx(alvo));
        }
    }
    CHECK(ordens.size() == 6u);                        // 3! ordens para as tres depois do mutex
    CHECK(contadores == std::set<int>{2, 3});
    CHECK(menor < kMenorEntreThreads + 5.0f);
    CHECK(maior > kMaiorEntreThreads - 5.0f);
}

TEST_CASE("Semaforo: o degrau - o Normal e o de sempre, o Dificil manda mais e mais depressa") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.entreInstrucoes == doctest::Approx(kEntreInstrucoes));
    CHECK(n.instrucoesPorThread == kInstrucoesPorThread);
    CHECK(n.rapidez == doctest::Approx(1.0f));
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreInstrucoes < n.entreInstrucoes);
    CHECK(d.instrucoesPorThread > n.instrucoesPorThread);
    CHECK(d.rapidez > n.rapidez);
    // A THREAD AINDA ACABA DE MANDAR dentro da rodada.
    CHECK(kComecaAMandar + static_cast<float>(d.instrucoesPorThread - 1) * d.entreInstrucoes < kPorRodada);
}
