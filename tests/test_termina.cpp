//
// A fase 3 do Luizinho: o programa Termina.
//

#include "doctest.h"

#include <set>

#include "../Source/Attacks/Termina.h"

using namespace Termina;

TEST_CASE("Termina: cada laco da uma volta por vez e para quando a condicao falha") {
    CHECK(Passo(Programa::Contagem, 3) == 2);
    CHECK_FALSE(Passo(Programa::Contagem, 0));
    CHECK(Passo(Programa::DeDoisEmDois, 7) == 5);
    CHECK_FALSE(Passo(Programa::DeDoisEmDois, 0));
    CHECK(Passo(Programa::Metade, 24) == 12);
    CHECK(Passo(Programa::Metade, 3) == 1);
    CHECK_FALSE(Passo(Programa::Metade, 1));
    CHECK(Passo(Programa::Crescente, 4) == 5);
    CHECK(Passo(Programa::DeTresEmTres, 7) == 10);
    CHECK_FALSE(Passo(Programa::DeTresEmTres, 10));
    CHECK(Codigo(Programa::DeDoisEmDois) == "enquanto (n != 0) n = n - 2;");
}

TEST_CASE("Termina: os valores do contador sao as voltas do laco, ate parar ou ate o limite") {
    CHECK(Valores(Programa::Contagem, 4, 64) == std::vector<int>{4, 3, 2, 1, 0});
    CHECK(Valores(Programa::Metade, 24, 64) == std::vector<int>{24, 12, 6, 3, 1});
    CHECK(Valores(Programa::DeDoisEmDois, 8, 64) == std::vector<int>{8, 6, 4, 2, 0});
    CHECK(Valores(Programa::DeTresEmTres, 4, 64) == std::vector<int>{4, 7, 10});
    // O que nao para chega ao limite.
    CHECK(Valores(Programa::DeDoisEmDois, 7, 5) == std::vector<int>{7, 5, 3, 1, -1, -3});
    CHECK(Valores(Programa::Crescente, 2, 3).size() == 4);
}

TEST_CASE("Termina: o veredito da conta bate com rodar o laco ate o fim") {
    for (int p = 0; p < kProgramas; ++p) {
        const auto programa = static_cast<Programa>(p);
        for (int n = -3; n <= 40; ++n) {
            CAPTURE(Codigo(programa));
            CAPTURE(n);
            // Rodado com folga: quem para, para muito antes de 200 voltas.
            const auto v = Valores(programa, n, 200);
            const bool parou = static_cast<int>(v.size()) - 1 < 200;
            CHECK(Para(programa, n) == parou);
        }
    }
}

TEST_CASE("Termina: as rodadas variam o programa e a entrada, e umas param e outras nao") {
    std::set<int> programas;
    int param = 0;
    int naoParam = 0;
    for (std::uint32_t s = 0; s < 300; ++s) {
        const int anterior = static_cast<int>(s % kProgramas);
        const Rodada r = Proxima(s, anterior);
        CHECK(static_cast<int>(r.programa) != anterior);
        programas.insert(static_cast<int>(r.programa));
        if (Para(r.programa, r.n)) {
            ++param;
            // Quem para, para em poucas voltas: cabe na rodada.
            CHECK(Valores(r.programa, r.n, kMaisVoltas).size() <= 9);
            CHECK(Valores(r.programa, r.n, kMaisVoltas).size() >= 2);
        } else {
            ++naoParam;
        }
    }
    CHECK(programas.size() == static_cast<size_t>(kProgramas));
    // Nem tudo para, nem tudo repete: cada lado com pelo menos um terco.
    CHECK(param > 100);
    CHECK(naoParam > 100);
}

TEST_CASE("Termina: o placar pergunta ao Termina, e o Paradoxo faz o contrario da resposta") {
    const Rodada r{Programa::DeDoisEmDois, 7};
    CHECK(Placar(r, std::nullopt) == "enquanto (n != 0) n = n - 2;\nTermina(P, 7) = ?");
    CHECK(Placar(r, false) == "enquanto (n != 0) n = n - 2;\nTermina(P, 7) = N\xC3\x83O");
    CHECK(Resposta(true) == "SIM");
    CHECK(Pergunta(r) == "Termina(P, 7)");
    CHECK(PlacarDoParadoxo(true, false) == "Paradoxo: se Termina(Paradoxo)\nSIM \xE2\x86\x92 o Paradoxo repete");
    CHECK(PlacarDoParadoxo(false, false) == "Paradoxo: se Termina(Paradoxo)\nN\xC3\x83O \xE2\x86\x92 o Paradoxo para");
    CHECK(PlacarDoParadoxo(std::nullopt, true) == "Termina n\xC3\xA3o pode existir.\ncontradi\xC3\xA7\xC3\xA3o!");
    // Ele comeca dizendo que para, e muda de ideia a cada virada.
    CHECK(VereditoDaVirada(0));
    for (int k = 1; k < kViradas; ++k) CHECK(VereditoDaVirada(k) != VereditoDaVirada(k - 1));
}

TEST_CASE("Termina: as viradas do Paradoxo ficam mais depressa, e ele cabe no fim da fase") {
    for (int k = 1; k < kViradas; ++k) CHECK(DuracaoDaVirada(k) <= DuracaoDaVirada(k - 1));
    CHECK(DuracaoDaVirada(kViradas) >= 0.3f);
    // Comecando no ultimo instante possivel (a rodada mais longa depois de kInicioDoParadoxo), o
    // Termina ainda estoura antes de a fase acabar.
    const float rodadaMaisLonga = kPensando + 0.15f + kMaiorCtrlC + kDepois;
    CHECK(kInicioDoParadoxo + rodadaMaisLonga + DuracaoDoParadoxo() < 20.0f);
}

TEST_CASE("Termina: o degrau do Normal e o de sempre, e o do Dificil aperta") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(n.bracos == kBracos);
    CHECK(n.porVolta == doctest::Approx(kPorVolta));
    CHECK(n.velocidade == doctest::Approx(kVelocidade));
    CHECK(d.bracos > n.bracos);
    CHECK(d.porVolta < n.porVolta);
    CHECK(d.velocidade > n.velocidade);
}
