//
// A pilha da fase 1 do Salles.
//

#include "../../doctest.h"

#include <algorithm>
#include "../../../Source/Attacks/Engine/Fase.h"

#include <random>
#include <set>
#include <vector>

#include "../../../Source/Attacks/Salles/Pilha.h"

TEST_CASE("Pilha: vazia so empilha, cheia derruba, e no meio ha os dois") {
    for (std::uint32_t s = 0; s < 2000; s += 13) {
        CAPTURE(s);
        CHECK(Pilha::Proxima(0, s * 2654435761u).tipo == Pilha::Tipo::Push);
        CHECK(Pilha::Proxima(Pilha::kCapacidade, s * 2654435761u).tipo == Pilha::Tipo::Overflow);
    }
    int push = 0, pop = 0;
    for (std::uint32_t s = 0; s < 4000; ++s) {
        const auto op = Pilha::Proxima(3, s * 2654435761u);
        CHECK(op.tipo != Pilha::Tipo::Overflow);
        CHECK(op.valor >= 0);
        CHECK(op.valor <= 9);
        (op.tipo == Pilha::Tipo::Push ? push : pop) += 1;
    }
    CHECK(static_cast<float>(push) / 4000.0f == doctest::Approx(Pilha::kChanceDePush).epsilon(0.05));
    CHECK(pop > 0);
}

TEST_CASE("Pilha: o programa sempre chega ao overflow, e nunca da pop na pilha vazia") {
    // Simulando a pilha com o proprio programa: a altura nunca fica negativa,
    // e em 20 operacoes - uma fase - o overflow acontece quase sempre.
    int comOverflow = 0;
    for (std::uint32_t semente = 1; semente <= 200; ++semente) {
        int altura = 0;
        bool transbordou = false;
        std::mt19937 gerador(semente);
        for (int i = 0; i < 20; ++i) {
            const auto op = Pilha::Proxima(altura, static_cast<std::uint32_t>(gerador()));
            if (op.tipo == Pilha::Tipo::Push) ++altura;
            else if (op.tipo == Pilha::Tipo::Pop) --altura;
            else { altura = 0; transbordou = true; }
            REQUIRE(altura >= 0);
            REQUIRE(altura <= Pilha::kCapacidade);
        }
        if (transbordou) ++comOverflow;
    }
    CHECK(comOverflow > 150);
}

TEST_CASE("Pilha: o lugar de cada bloco, e o placar de cada operacao") {
    CHECK(Pilha::AbaixoDoProfessor(0) == doctest::Approx(Pilha::kPrimeiroBloco));
    CHECK(Pilha::AbaixoDoProfessor(2) == doctest::Approx(Pilha::kPrimeiroBloco + 2.0f * Pilha::kEntreBlocos));
    CHECK(Pilha::Placar({Pilha::Tipo::Push, 7}, 0) == "push(7)");
    CHECK(Pilha::Placar({Pilha::Tipo::Pop, 0}, 4) == "pop() \xE2\x86\x92 4");
    CHECK(Pilha::Placar({Pilha::Tipo::Overflow, 0}, 0) == "stack overflow!");
    CHECK(Pilha::CaraDoBloco(3) == "N3");
    CHECK(Pilha::CaraDoBloco(12) == "N0");
}

TEST_CASE("Pilha: a pilha sorteada - de 4 a 6 blocos, e o overflow chega em quase toda fase") {
    std::set<int> capacidades;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s(semente);
        const Pilha::Plano p = Pilha::Sortear(s);
        CAPTURE(p.capacidade);
        CAPTURE(p.chanceDePush);
        CHECK(p.capacidade >= Pilha::kMenorCapacidade);
        CHECK(p.capacidade <= Pilha::kMaiorCapacidade);
        capacidades.insert(p.capacidade);
        CHECK(Pilha::Proxima(p.capacidade, semente * 2654435761u, p).tipo == Pilha::Tipo::Overflow);
        CHECK(Pilha::Proxima(p.capacidade - 1, semente * 2654435761u, p).tipo != Pilha::Tipo::Overflow);
    }
    CHECK(capacidades == std::set<int>{4, 5, 6});
    // A PIOR COMBINACAO de cada capacidade (a menor chance) ainda transborda em
    // 20 operacoes - menos que uma fase - quase sempre.
    for (int capacidade = Pilha::kMenorCapacidade; capacidade <= Pilha::kMaiorCapacidade; ++capacidade) {
        CAPTURE(capacidade);
        const Pilha::Plano p{capacidade, 0.55f + 0.05f * static_cast<float>(capacidade - Pilha::kMenorCapacidade)};
        int comOverflow = 0;
        for (std::uint32_t semente = 1; semente <= 200; ++semente) {
            int altura = 0;
            bool transbordou = false;
            std::mt19937 gerador(semente);
            for (int i = 0; i < 20; ++i) {
                const auto op = Pilha::Proxima(altura, static_cast<std::uint32_t>(gerador()), p);
                if (op.tipo == Pilha::Tipo::Push) ++altura;
                else if (op.tipo == Pilha::Tipo::Pop) --altura;
                else { altura = 0; transbordou = true; }
                REQUIRE(altura >= 0);
                REQUIRE(altura <= capacidade);
            }
            if (transbordou) ++comOverflow;
        }
        CHECK(comOverflow > 130);
    }
    // A MAIOR PILHA ainda fica acima do aluno.
    CHECK(Pilha::AbaixoDoProfessor(Pilha::kMaiorCapacidade - 1) < 400.0f);
}

TEST_CASE("Pilha: o degrau - o Normal e o de sempre, o Dificil opera mais e mais depressa") {
    const Pilha::Degrau n = Pilha::DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.passo == doctest::Approx(Pilha::kPasso));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    const Pilha::Degrau d = Pilha::DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.passo < n.passo);
    CHECK(d.rapidez > n.rapidez);
    CHECK(Pilha::kEstouraEm < d.passo);   // o pop estoura antes da operacao seguinte
}
