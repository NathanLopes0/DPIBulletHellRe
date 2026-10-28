//
// O if, o else if e o switch da fase 1 do Andre.
//

#include "doctest.h"

#include <cstdint>
#include <set>
#include <string>

#include "../Source/Variacao.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "../Source/Attacks/Condicional.h"

using namespace Condicional;

TEST_CASE("Condicional: o if manda cada valor para o ramo certo") {
    const Comando maior{Forma::Maior, 5, 0};
    CHECK(Executados(maior, 6) == std::vector<int>{0});
    CHECK(Executados(maior, 5) == std::vector<int>{1});    // 5 > 5 e falso
    CHECK(Executados(maior, 0) == std::vector<int>{1});

    const Comando entre{Forma::Intervalo, 2, 7};
    CHECK(Executados(entre, 3) == std::vector<int>{0});
    CHECK(Executados(entre, 2) == std::vector<int>{1});    // as duas pontas ficam de fora
    CHECK(Executados(entre, 7) == std::vector<int>{1});

    CHECK(Executados({Forma::Par, 0, 0}, 4) == std::vector<int>{0});
    CHECK(Executados({Forma::Par, 0, 0}, 0) == std::vector<int>{0});
    CHECK(Executados({Forma::Par, 0, 0}, 7) == std::vector<int>{1});
    CHECK(Executados({Forma::Impar, 0, 0}, 7) == std::vector<int>{0});
    CHECK(Executados({Forma::Impar, 0, 0}, 4) == std::vector<int>{1});
}

TEST_CASE("Condicional: no else if vale o PRIMEIRO ramo verdadeiro") {
    const Comando c{Forma::SenaoSe, 3, 7};
    CHECK(Ramos(c) == 3);
    CHECK(Executados(c, 1) == std::vector<int>{0});     // 1 < 3 e 1 < 7: so o primeiro
    CHECK(Executados(c, 3) == std::vector<int>{1});
    CHECK(Executados(c, 6) == std::vector<int>{1});
    CHECK(Executados(c, 7) == std::vector<int>{2});
    CHECK(Executados(c, 9) == std::vector<int>{2});
}

TEST_CASE("Condicional: o switch sem break cai do case de n ate o ultimo, e n sem case nao executa nada") {
    const Comando c{Forma::Escolha, 0, 0};
    CHECK(Ramos(c) == kCases);
    CHECK(Executados(c, 1) == std::vector<int>{1, 2, 3});
    CHECK(Executados(c, 3) == std::vector<int>{3});
    CHECK(Executados(c, 0) == std::vector<int>{0, 1, 2, 3});
    CHECK(Executados(c, 7).empty());
    CHECK(Executados(c, -1).empty());

    // O SORTEIO NUNCA DA 0, que faria cair todos e nao deixaria faixa segura -
    // e da todo case que tem quem caia depois dele, e o n sem case.
    std::set<int> vistos;
    for (std::uint32_t s = 0; s < 500; ++s) {
        const int n = Sortear(c, s * 2654435761u);
        CHECK(n != 0);
        CHECK(Executados(c, n).size() < static_cast<std::size_t>(kCases));
        vistos.insert(n);
    }
    CHECK(vistos.count(1) == 1);
    CHECK(vistos.count(2) == 1);
    CHECK(vistos.count(3) == 1);
    CHECK(vistos.count(7) == 1);
}

TEST_CASE("Condicional: if (n = 0) e atribuicao - o numero vira 0 e o if e sempre falso") {
    const Comando zero{Forma::Atribuicao, 0, 0};
    const Comando sete{Forma::Atribuicao, 7, 0};
    for (int n = 0; n <= kMaiorValor; ++n) {
        CAPTURE(n);
        CHECK(Depois(zero, n) == 0);
        CHECK(Depois(sete, n) == 7);
        CHECK(Executados(zero, n) == std::vector<int>{1});
        CHECK(Executados(sete, n) == std::vector<int>{0});
        CHECK(Depois({Forma::Maior, 5, 0}, n) == n);
    }
    CHECK(Explicacao(zero, 4).find("falso") != std::string::npos);
    CHECK(Explicacao(sete, 4).find("verdadeiro") != std::string::npos);
}

TEST_CASE("Condicional: a fase vai do if simples a pegadinha, um comando de cada forma") {
    const Forma esperadas[] = {Forma::Maior, Forma::Par, Forma::SenaoSe, Forma::Escolha, Forma::Atribuicao};
    for (int vez = 0; vez < 5; ++vez) {
        CAPTURE(vez);
        // A PRIMEIRA VOLTA e sempre a mesma, em qualquer sorteio: a fase abre
        // com o if mais simples.
        for (std::uint32_t s = 0; s < 50; ++s) CHECK(Proximo(vez, s * 7919u).forma == esperadas[vez]);
    }
    // Nas voltas seguintes aparecem o && e o impar.
    std::set<int> formas;
    for (std::uint32_t s = 0; s < 200; ++s) {
        formas.insert(static_cast<int>(Proximo(5, s * 7919u).forma));
        formas.insert(static_cast<int>(Proximo(6, s * 7919u).forma));
    }
    CHECK(formas.count(static_cast<int>(Forma::Intervalo)) == 1);
    CHECK(formas.count(static_cast<int>(Forma::Impar)) == 1);
}

TEST_CASE("Condicional: todo comando sorteavel esta em Todos, e todo valor cai em algum ramo que existe") {
    const auto todos = Todos();
    auto igual = [](const Comando& x, const Comando& y) { return x.forma == y.forma && x.a == y.a && x.b == y.b; };
    for (int vez = 0; vez < 10; ++vez) {
        for (std::uint32_t s = 0; s < 100; ++s) {
            const Comando c = Proximo(vez, s * 104729u + static_cast<std::uint32_t>(vez));
            bool esta = false;
            for (const auto& t : todos) esta = esta || igual(t, c);
            CHECK(esta);
        }
    }
    for (const auto& c : todos) {
        CAPTURE(Placar(c));
        CHECK_FALSE(Placar(c).empty());
        // NENHUM RAMO SOBRA: com os valores que o sorteio da, cada faixa e
        // alcancada por algum - fora a pegadinha, que e justamente o ramo
        // que nunca vem.
        std::set<int> alcancados;
        for (int n = 0; n <= kMaiorValor; ++n) {
            for (const int r : Executados(c, n)) {
                CHECK(r >= 0);
                CHECK(r < Ramos(c));
                alcancados.insert(r);
            }
        }
        if (c.forma != Forma::Atribuicao) CHECK(static_cast<int>(alcancados.size()) == Ramos(c));
        // OS CABECALHOS SAO DIFERENTES entre si, e as caras tambem.
        std::set<std::string> rotulos, caras;
        for (int r = 0; r < Ramos(c); ++r) {
            rotulos.insert(Rotulo(c, r));
            caras.insert(CaraDoRotulo(c, r));
            CHECK(CaraDoRotuloAceso(c, r) != CaraDoRotulo(c, r));
        }
        CHECK(static_cast<int>(rotulos.size()) == Ramos(c));
        CHECK(static_cast<int>(caras.size()) == Ramos(c));
        CHECK(Rotulo(c, Ramos(c) - 1) == (c.forma == Forma::Escolha ? "case 3:" : "else"));
    }
}

TEST_CASE("Condicional: as faixas cobrem o campo inteiro, sem buraco e sem sobra") {
    for (const int ramos : {2, 3, 4}) {
        CAPTURE(ramos);
        CHECK(InicioDaFaixa(0, ramos, 1200.0f) == doctest::Approx(0.0f));
        CHECK(FimDaFaixa(ramos - 1, ramos, 1200.0f) == doctest::Approx(1200.0f));
        for (int r = 0; r + 1 < ramos; ++r) CHECK(FimDaFaixa(r, ramos, 1200.0f) == doctest::Approx(InicioDaFaixa(r + 1, ramos, 1200.0f)));

        // O CONFETE COBRE A FAIXA: nenhum vao entre dois vizinhos (contando as
        // duas fileiras) maior que meio passo, e nenhum fora dela.
        const float ini = InicioDaFaixa(0, ramos, 1200.0f), fim = FimDaFaixa(0, ramos, 1200.0f);
        std::vector<float> xs;
        for (const auto& c : Confetes(ini, fim)) {
            CHECK(c.x >= ini);
            CHECK(c.x <= fim);
            CHECK(c.acima >= 0.0f);
            xs.push_back(c.x);
        }
        std::sort(xs.begin(), xs.end());
        REQUIRE(xs.size() >= 2);
        // AS BORDAS SAO DA FAIXA: a divisa entre duas faixas nao e um
        // corredor seguro.
        CHECK(xs.front() == doctest::Approx(ini));
        CHECK(xs.back() == doctest::Approx(fim));
        for (std::size_t k = 1; k < xs.size(); ++k) CHECK(xs[k] - xs[k - 1] <= kEntreConfetes / 2.0f + 0.01f);
    }
}

TEST_CASE("Condicional: o ultimo balao do comando estoura antes de o comando acabar, no pior caminho") {
    // O pior caminho: o professor na ponta esquerda, e o balao indo para o
    // cabecalho da faixa mais longe, a do case 3 na direita.
    const float largura = 1200.0f;
    const float xFaixa = (InicioDaFaixa(kCases - 1, kCases, largura) + FimDaFaixa(kCases - 1, kCases, largura)) / 2.0f;
    const float dx = xFaixa - kXMinimo;
    const float dy = (kYDoProfessor + kAbaixoDoProfessor) - kYDosRotulos;
    // O balao segue a seta, em angulo reto: de lado, e depois para cima.
    const float voo = (dx + dy) / kVelocidadeDoBalao;
    const float ultimo = kPrimeiroBalao + static_cast<float>(kBaloesPorComando - 1) * kEntreBaloes;
    const float estoura = ultimo + kAbaixoDoProfessor / kVelocidadeDoBalao + kNaDecisao + voo;
    CHECK(estoura < kPorComando);
    // E a fase tem kComandosPorFase comandos inteiros.
    CHECK(static_cast<float>(kComandosPorFase) * kPorComando == doctest::Approx(Fase::kDuracao));
}

TEST_CASE("Condicional: cada balao tem uma cara por numero e por cor") {
    std::set<std::string> caras;
    for (int n = 0; n <= kMaiorValor; ++n) {
        for (int c = 0; c < kCores; ++c) caras.insert(CaraDoBalao(n, c));
    }
    CHECK(static_cast<int>(caras.size()) == (kMaiorValor + 1) * kCores);
    CHECK(CaraDoBalao(3, kCores) == CaraDoBalao(3, 0));
}

TEST_CASE("Condicional: o degrau - o Normal e o de sempre, e no Dificil o ultimo balao ainda estoura a tempo") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.baloesPorComando == kBaloesPorComando);
    CHECK(n.entreBaloes == doctest::Approx(kEntreBaloes));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(n.naDecisao == doctest::Approx(kNaDecisao));
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.naDecisao < n.naDecisao);
    CHECK(d.naDecisao > 0.0f);
    CHECK(d.baloesPorComando > n.baloesPorComando);
    CHECK(d.entreBaloes < n.entreBaloes);
    CHECK(d.rapidez > n.rapidez);
    // O MESMO PIOR CAMINHO do teste de cima, com o degrau do Dificil.
    const float largura = 1200.0f;
    const float xFaixa = (InicioDaFaixa(kCases - 1, kCases, largura) + FimDaFaixa(kCases - 1, kCases, largura)) / 2.0f;
    const float voo = ((xFaixa - kXMinimo) + (kYDoProfessor + kAbaixoDoProfessor - kYDosRotulos)) / kVelocidadeDoBalao;
    for (const Degrau& g : {n, d}) {
        const float ultimo = kPrimeiroBalao + static_cast<float>(g.baloesPorComando - 1) * g.entreBaloes;
        CHECK(ultimo + kAbaixoDoProfessor / kVelocidadeDoBalao + g.naDecisao + voo < kPorComando);
    }
}

TEST_CASE("Condicional: em qualquer ordem da luta, a fase tem as cinco formas e abre no if simples") {
    for (std::uint32_t semente = 0; semente < 200; ++semente) {
        Variacao::Sorteio s(semente);
        const auto ordem = Variacao::Ordem(s, kFormas);
        CHECK(Proximo(Variacao::NaOrdem(ordem, 0), semente).forma == Forma::Maior);
        std::set<int> formas;
        for (int vez = 0; vez < kFormas; ++vez) {
            const Forma f = Proximo(Variacao::NaOrdem(ordem, vez), semente + 7u).forma;
            formas.insert(f == Forma::Impar ? static_cast<int>(Forma::Par)
                        : f == Forma::Intervalo ? static_cast<int>(Forma::Maior) : static_cast<int>(f));
        }
        CHECK(formas.size() == static_cast<size_t>(kFormas));
    }
}
