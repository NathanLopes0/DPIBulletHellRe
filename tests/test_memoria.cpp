//
// A gerencia de memoria da fase 3 do Luizinho.
//

#include "doctest.h"

#include <cmath>
#include <set>
#include <algorithm>
#include <cstdint>

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

TEST_CASE("Memoria: o roteiro da luta pode por o C++ primeiro, com cada metade inteira") {
    const Roteiro sempre = RoteiroDeSempre();
    int cppPrimeiro = 0;
    for (std::uint32_t semente = 0; semente < 400; ++semente) {
        Variacao::Sorteio s = Variacao::SorteioDa(semente, kFaseDaMemoria);
        const Roteiro r = Sortear(s);
        const bool trocado = r != sempre;
        cppPrimeiro += trocado ? 1 : 0;
        for (int i = 0; i < kPassos; ++i) {
            // CADA METADE NA ORDEM DO PROGRAMA: so a metade muda de lugar.
            const int deSempre = trocado ? (i + kPassos / 2) % kPassos : i;
            CHECK(r[static_cast<size_t>(i)] == sempre[static_cast<size_t>(deSempre)]);
            // A TROCA DE PROGRAMA so no meio, e o heap so esvazia ali.
            CHECK(TrocaDeLinguagem(i, r) == (i == kPassos / 2));
        }
        CHECK(LinguagemDe(r[0]) == (trocado ? Linguagem::Cpp : Linguagem::Java));
        // A FASE DURA O MESMO, e o relogio acha o passo de cada instante.
        CHECK(InicioDaVez(kPassos, r) == doctest::Approx(InicioDaVez(kPassos, sempre)));
        for (int i = 0; i < kPassos; ++i) {
            CHECK(PassoEm(InicioDaVez(i, r) + 0.01f, r) == i);
            CHECK(PassoEm(InicioDaVez(i, r) - 0.01f, r) == i - 1);
        }
    }
    CHECK(cppPrimeiro > 150);
    CHECK(cppPrimeiro < 250);
    // O DE SEMPRE e o de antes, instante por instante.
    for (float t = 0.0f; t < 21.0f; t += 0.05f) CHECK(PassoEm(t, sempre) == PassoEm(t));
    for (int i = 0; i < kPassos; ++i) CHECK(InicioDaVez(i, sempre) == doctest::Approx(Inicio(sempre[static_cast<size_t>(i)])));
}

TEST_CASE("Memoria: o degrau do Normal e o de sempre, e o do Dificil acende antes e estoura mais") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.avisoDaAlocacao == kAvisoDaAlocacao);
    CHECK(n.estilhacosDoPendente == kEstilhacosDoPendente);
    CHECK(n.rapidez == 1.0f);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.avisoDaAlocacao < n.avisoDaAlocacao);
    CHECK(d.estilhacosDoPendente > n.estilhacosDoPendente);
    CHECK(d.rapidez > 1.0f);
    // O ALUNO AINDA SAI DE BAIXO DO BLOCO, ate pelo lado comprido: do meio do
    // maior bloco do roteiro ate a ponta, a 300 px/s, cabe no aviso.
    int maior = 0;
    for (int i = 0; i < kPassos; ++i) {
        for (const Evento& e : Eventos(static_cast<Passo>(i))) maior = std::max(maior, e.celulas);
    }
    CHECK(Area({0.0f, 0.0f}, maior).w / 2.0f / 300.0f < d.avisoDaAlocacao);
}

TEST_CASE("Memoria: no Dificil os lacos alocam mais depressa, o while (1) vai mais longe, e o do Java segue o placar") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    auto alocacoes = [](const std::vector<Evento>& ev) {
        std::vector<float> q;
        for (const Evento& e : ev) if (e.acao == Acao::Alocar) q.push_back(e.quando);
        return q;
    };
    for (int i = 0; i < kPassos; ++i) {
        const auto p = static_cast<Passo>(i);
        CAPTURE(i);
        // O NORMAL E O DE SEMPRE, evento por evento.
        const auto sempre = Eventos(p);
        const auto normal = Eventos(p, n);
        REQUIRE(sempre.size() == normal.size());
        for (size_t k = 0; k < sempre.size(); ++k) {
            CHECK(sempre[k].quando == normal[k].quando);
            CHECK(sempre[k].acao == normal[k].acao);
            CHECK(sempre[k].celulas == normal[k].celulas);
            CHECK(sempre[k].raiz == normal[k].raiz);
        }
        // E TUDO CABE NO PASSO, nos dois modos.
        for (const Evento& e : Eventos(p, d)) CHECK(e.quando + d.avisoDaAlocacao < Duracao(p));
    }
    CHECK(d.entreAlocacoes < n.entreAlocacoes);
    // O DO JAVA ALOCA O QUE O PLACAR DIZ: "i < 6", nos dois modos.
    CHECK(alocacoes(Eventos(Passo::LacoJava, n)).size() == static_cast<size_t>(kNoLacoJava));
    CHECK(alocacoes(Eventos(Passo::LacoJava, d)).size() == static_cast<size_t>(kNoLacoJava));
    CHECK(Placar(Passo::LacoJava, false).find("i < 6") != std::string::npos);
    // O WHILE (1) VAI MAIS LONGE, e todo bloco nasce antes do bad_alloc.
    const auto cpp = alocacoes(Eventos(Passo::LacoCpp, d));
    CHECK(cpp.size() > static_cast<size_t>(kNoLacoCpp));
    for (const float q : cpp) CHECK(q < kEstouro);
}

TEST_CASE("Memoria: todo passo tem legenda, e a do delete fala do ponteiro pendente") {
    for (int k = 0; k < Memoria::kPassos; ++k) {
        CHECK_FALSE(Memoria::Legenda(static_cast<Memoria::Passo>(k)).empty());
    }
    CHECK(Memoria::Legenda(Memoria::Passo::Libera).find("pendente") != std::string::npos);
    CHECK(Memoria::Legenda(Memoria::Passo::Coleta).find("coletor") != std::string::npos);
    CHECK(Memoria::Legenda(Memoria::Passo::LacoCpp).find("bad_alloc") != std::string::npos);
}
