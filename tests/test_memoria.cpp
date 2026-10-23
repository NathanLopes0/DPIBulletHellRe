//
// A gerencia de memoria da fase 3 do Luizinho.
//

#include "doctest.h"

#include <cmath>
#include <set>

#include "../Source/Attacks/Memoria.h"

using namespace Memoria;

namespace {
    constexpr Passo kTodos[kPassos] = {Passo::NovosAeB, Passo::NovoC, Passo::LacoJava, Passo::Coleta,
                                       Passo::NovoP, Passo::Libera, Passo::Escreve, Passo::LacoCpp};
}

TEST_CASE("Memoria: o coletor alcanca pelas raizes e pelos prox, e o resto e lixo") {
    Heap h;
    const int a = Novo(h, 2);
    Apontar(h, "a", a);
    const int b = Novo(h, 2);
    ApontarProx(h, a, b);
    CHECK(Alcancaveis(h) == std::set<int>{a, b});
    CHECK(Lixo(h).empty());

    // a = new No(): o A sai das raizes, e o B so era alcancado por ele.
    const int c = Novo(h, 2);
    Apontar(h, "a", c);
    CHECK(Alcancaveis(h) == std::set<int>{c});
    CHECK(Lixo(h) == std::vector<int>{a, b});

    // O laco: seis blocos que ninguem guarda.
    std::vector<int> esperado = {a, b};
    for (int i = 0; i < 6; ++i) esperado.push_back(Novo(h, 3));
    CHECK(Lixo(h) == esperado);

    // A VARREDURA leva o lixo inteiro e deixa o vivo.
    CHECK(Varrer(h) == esperado);
    REQUIRE(h.blocos.size() == 1u);
    CHECK(h.blocos.front().id == c);
    CHECK(Lixo(h).empty());
}

TEST_CASE("Memoria: um ciclo que ninguem alcanca e lixo - o que a contagem de referencias nao veria") {
    Heap h;
    const int x = Novo(h, 2);
    const int y = Novo(h, 2);
    ApontarProx(h, x, y);
    ApontarProx(h, y, x);
    CHECK(Lixo(h) == std::vector<int>{x, y});
    Apontar(h, "a", y);
    CHECK(Alcancaveis(h) == std::set<int>{x, y});
    Apontar(h, "a", -1);    // a = null
    CHECK(Varrer(h) == std::vector<int>{x, y});
    CHECK(h.blocos.empty());
}

TEST_CASE("Memoria: o delete deixa o ponteiro pendente, e sem coletor o lixo vaza") {
    Heap h;
    const int p = Novo(h, 4);
    Apontar(h, "p", p);
    CHECK_FALSE(Pendente(h, "p"));
    Liberar(h, "p");
    CHECK(Pendente(h, "p"));
    // O que o delete levou nao e memoria de ninguem, nem lixo para varrer.
    CHECK(Alcancaveis(h).empty());
    CHECK(Lixo(h).empty());
    CHECK_FALSE(Pendente(h, "q"));

    std::vector<int> vazados;
    for (int i = 0; i < 7; ++i) vazados.push_back(Novo(h, 4));
    CHECK(Vazar(h) == vazados);
    for (const int id : vazados) CHECK(Achar(h, id)->estado == Estado::Vazou);
    // VAZOU E PARA SEMPRE: nao e lixo de varredura, e vazar de novo nao repete.
    CHECK(Lixo(h).empty());
    CHECK(Vazar(h).empty());
    CHECK(Varrer(h).empty());
    CHECK(h.blocos.size() == 8u);
}

TEST_CASE("Memoria: o roteiro roda no heap e da o que a fase mostra") {
    // O ROTEIRO INTEIRO, evento por evento, como o Luizinho o roda.
    Heap h;
    std::set<int> varridos, vazados, estourados;
    bool pendenteNaEscrita = false;
    for (const Passo p : kTodos) {
        if (p == Passo::NovoP) h = Heap{};   // o programa Java acabou
        for (const Evento& e : Eventos(p)) {
            switch (e.acao) {
                case Acao::Alocar: {
                    const int id = Novo(h, e.celulas);
                    if (e.raiz.empty()) break;
                    if (e.noProx) ApontarProx(h, h.raizes.at(e.raiz), id);
                    else Apontar(h, e.raiz, id);
                    break;
                }
                case Acao::Parar: break;
                case Acao::Varrer: for (const int id : Varrer(h)) varridos.insert(id); break;
                case Acao::Liberar: Liberar(h, e.raiz); break;
                case Acao::Escrever: pendenteNaEscrita = Pendente(h, e.raiz); break;
                case Acao::Estourar:
                    for (const Bloco& b : h.blocos) if (b.estado == Estado::Vazou) estourados.insert(b.id);
                    break;
            }
            if (LinguagemDe(p) == Linguagem::Cpp) for (const int id : Vazar(h)) vazados.insert(id);
        }
    }
    CHECK(varridos.size() == 8u);          // os dois nos de antes e os seis do laco
    CHECK(pendenteNaEscrita);              // p[2] = 7 escreve num bloco liberado
    CHECK(vazados.size() == 7u);           // o while (1)
    CHECK(estourados == vazados);          // e o bad_alloc leva todos
}

TEST_CASE("Memoria: o relogio do roteiro") {
    float soma = kPrimeiro;
    for (const Passo p : kTodos) soma += Duracao(p);
    CHECK(soma == doctest::Approx(Fase::kDuracao));

    CHECK(PassoEm(0.0f) == -1);
    for (int i = 0; i < kPassos; ++i) {
        const Passo p = kTodos[i];
        CHECK(PassoEm(Inicio(p) + 0.01f) == i);
        CHECK(PassoEm(Inicio(p) + Duracao(p) - 0.01f) == i);
        for (const Evento& e : Eventos(p)) {
            CHECK(e.quando >= 0.0f);
            CHECK(e.quando < Duracao(p));
            // O bloco acende antes de o passo acabar.
            if (e.acao == Acao::Alocar) CHECK(e.quando + kAvisoDaAlocacao < Duracao(p));
        }
    }
    CHECK(PassoEm(Fase::kDuracao + 1.0f) == kPassos - 1);

    // O BAD_ALLOC VEM DEPOIS DE O ULTIMO BLOCO ACENDER, e ainda tem tempo de
    // voar antes de a fase acabar.
    float ultimo = 0.0f;
    float estouro = -1.0f;
    for (const Evento& e : Eventos(Passo::LacoCpp)) {
        if (e.acao == Acao::Alocar) ultimo = std::max(ultimo, e.quando + kAvisoDaAlocacao);
        if (e.acao == Acao::Estourar) estouro = e.quando;
    }
    CHECK(estouro > ultimo);
    CHECK(Duracao(Passo::LacoCpp) - estouro >= 1.0f);

    // A pausa do coletor: para, e varre depois.
    const auto coleta = Eventos(Passo::Coleta);
    REQUIRE(coleta.size() == 2u);
    CHECK(coleta[0].acao == Acao::Parar);
    CHECK(coleta[1].acao == Acao::Varrer);
    CHECK(coleta[1].quando > coleta[0].quando);
}

TEST_CASE("Memoria: o placar diz a linguagem e o codigo, e depois o que aconteceu") {
    CHECK(Placar(Passo::NovoC, false) == "Java \xc2\xb7 coletor de lixo\na = new No();");
    CHECK(Placar(Passo::NovoP, false).rfind("C++ \xc2\xb7 ger\xc3\xaancia manual\n", 0) == 0);
    CHECK(Placar(Passo::Escreve, true).find("comportamento indefinido") != std::string::npos);
    CHECK(Placar(Passo::LacoCpp, true).find("bad_alloc") != std::string::npos);
    std::set<std::string> vistos;
    for (const Passo p : kTodos) {
        CHECK(Placar(p, true).rfind(Placar(p, false), 0) == 0);
        vistos.insert(Placar(p, false));
    }
    CHECK(vistos.size() == static_cast<size_t>(kPassos));
}

TEST_CASE("Memoria: o alocador poe o bloco no aluno, ou no lugar livre mais perto") {
    const Ponto aluno{600.0f, 682.0f};
    const Ponto c = OndeAlocar(4, aluno, {}, 1200.0f, 746.0f);
    CHECK(c.x == doctest::Approx(aluno.x));
    CHECK(c.y == doctest::Approx(aluno.y));
    const Retangulo r = Area(c, 4);
    CHECK(r.w == doctest::Approx(3.0f * kEntreCelulas + kLadoDaCelula));
    CHECK(r.x <= aluno.x);
    CHECK(r.x + r.w >= aluno.x);

    const auto celulas = Celulas(c, 4);
    REQUIRE(celulas.size() == 4u);
    CHECK(celulas[1].x - celulas[0].x == doctest::Approx(kEntreCelulas));
    CHECK((celulas[0].x + celulas[3].x) / 2.0f == doctest::Approx(c.x));

    // O ROTEIRO INTEIRO NUM ALUNO PARADO: nenhum bloco encosta no outro, todo
    // bloco cabe no campo, e cada um fica o mais perto possivel dele.
    std::vector<Retangulo> ocupados;
    for (const Passo p : kTodos) {
        for (const Evento& e : Eventos(p)) {
            if (e.acao != Acao::Alocar) continue;
            const Ponto onde = OndeAlocar(e.celulas, aluno, ocupados, 1200.0f, 746.0f);
            const Retangulo novo = Area(onde, e.celulas);
            for (const Retangulo& o : ocupados) {
                const Retangulo folgado{o.x - kFolga + 0.01f, o.y - kFolga + 0.01f,
                                        o.w + 2.0f * kFolga - 0.02f, o.h + 2.0f * kFolga - 0.02f};
                CHECK_FALSE(novo.Cruza(folgado));
            }
            CHECK(novo.x >= kMargem - 0.01f);
            CHECK(novo.x + novo.w <= 1200.0f - kMargem + 0.01f);
            CHECK(novo.y >= kAlturaMinima - 0.01f);
            CHECK(novo.y + novo.h <= 746.0f - kMargem + 0.01f);
            // Perto: a menos de 300 px dele, com treze blocos no campo.
            CHECK(std::hypot(onde.x - aluno.x, onde.y - aluno.y) < 300.0f);
            ocupados.push_back(novo);
        }
    }
    CHECK(ocupados.size() == 2u + 1u + 6u + 1u + 7u);

    // No canto, o bloco fica dentro do campo.
    const Ponto canto = OndeAlocar(4, {5.0f, 740.0f}, {}, 1200.0f, 746.0f);
    CHECK(Area(canto, 4).x >= kMargem - 0.01f);
    CHECK(Area(canto, 4).y + kLadoDaCelula <= 746.0f - kMargem + 0.01f);
}
