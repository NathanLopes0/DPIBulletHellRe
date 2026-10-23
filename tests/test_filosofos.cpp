//
// A final do Vitor: o jantar dos filosofos.
//

#include "doctest.h"

#include <cmath>
#include <set>

#include "../Source/Attacks/Filosofos.h"

using namespace Filosofos;

namespace {
    /// Roda o roteiro inteiro numa mesa posta, conferindo cada passo. Devolve
    /// a mesa do fim, se ela passou por um deadlock no caminho e quantas
    /// refeicoes houve.
    struct Visto { Mesa mesa; bool deadlock = false; int comeram = 0; };
    Visto Rodar(const Rodada r, const int escolhido) {
        Visto v;
        v.mesa = MesaPosta();
        for (const Passo& p : Roteiro(r, escolhido)) {
            CAPTURE(p.quando);
            CAPTURE(p.filosofo);
            CAPTURE(static_cast<int>(p.acao));
            REQUIRE(Aplicar(v.mesa, p));
            CHECK(p.quando < kPorRodada);
            if (p.acao == Acao::Come) ++v.comeram;
            // UM GARFO, UM DONO: nenhum filosofo come sem os dois.
            for (int f = 0; f < kFilosofos; ++f) {
                if (v.mesa.estado[static_cast<size_t>(f)] != Estado::Comendo) continue;
                CHECK(v.mesa.dono[static_cast<size_t>(GarfoDaEsquerda(f))] == f);
                CHECK(v.mesa.dono[static_cast<size_t>(GarfoDaDireita(f))] == f);
            }
            v.deadlock = v.deadlock || EmDeadlock(v.mesa);
        }
        return v;
    }
}

TEST_CASE("Filosofos: os garfos - cada um entre dois filosofos, e os nomes") {
    for (int f = 0; f < kFilosofos; ++f) {
        CHECK(GarfoDaEsquerda(f) == f);
        CHECK(GarfoDaDireita(f) == (f + 1) % kFilosofos);
        // O garfo da direita de um e o da esquerda do seguinte.
        CHECK(GarfoDaDireita(f) == GarfoDaEsquerda((f + 1) % kFilosofos));
    }
    std::set<std::string> nomes;
    for (int f = 0; f < kFilosofos; ++f) nomes.insert(Nome(f));
    CHECK(nomes.size() == 5u);
    CHECK(Nome(3) == "Kant");
}

TEST_CASE("Filosofos: a mesa recusa o que a regra nao deixa") {
    Mesa m = MesaPosta();
    CHECK_FALSE(Aplicar(m, {0.0f, 0, Acao::Pega, 3}));          // nao e do lado dele
    CHECK(Aplicar(m, {0.0f, 0, Acao::Pega, 0}));
    CHECK_FALSE(Aplicar(m, {0.0f, 4, Acao::Pega, 0}));          // ja tem dono
    CHECK_FALSE(Aplicar(m, {0.0f, 0, Acao::Come}));             // so com um garfo
    CHECK_FALSE(Aplicar(m, {0.0f, 1, Acao::Espera, 2}));        // esperar um garfo livre
    CHECK(Aplicar(m, {0.0f, 4, Acao::Espera, 0}));
    CHECK(Esperas(m) == std::vector<std::pair<int, int>>{{4, 0}});
    CHECK_FALSE(EmDeadlock(m));
    CHECK(Aplicar(m, {0.0f, 0, Acao::Morre}));
    CHECK(m.dono[0] == -1);
    CHECK_FALSE(Aplicar(m, {0.0f, 0, Acao::Pega, 1}));          // morto nao age
    CHECK_FALSE(Aplicar(m, {0.0f, 0, Acao::Solta}));
}

TEST_CASE("Filosofos: o jantar - dois comem de cada vez, e ninguem espera ninguem") {
    const Visto v = Rodar(Rodada::Jantar, 0);
    CHECK_FALSE(v.deadlock);
    CHECK(v.comeram == 5);
}

TEST_CASE("Filosofos: o deadlock - todos com o garfo da esquerda, a espera circular, e o kill -9 que a quebra") {
    for (int vitima = 0; vitima < kFilosofos; ++vitima) {
        CAPTURE(vitima);
        Mesa m = MesaPosta();
        bool viu = false;
        bool quebrou = false;
        for (const Passo& p : Roteiro(Rodada::Deadlock, vitima)) {
            REQUIRE(Aplicar(m, p));
            if (EmDeadlock(m)) {
                viu = true;
                CHECK(Esperas(m).size() == 5u);   // o pentagono inteiro
                CHECK(Placar(Rodada::Deadlock, m) == "deadlock \xc2\xb7 espera circular");
            }
            if (p.acao == Acao::Morre) {
                CHECK(p.filosofo == vitima);
                CHECK_FALSE(EmDeadlock(m));
                quebrou = viu;
            }
        }
        CHECK(viu);
        CHECK(quebrou);
        CHECK(m.estado[static_cast<size_t>(vitima)] == Estado::Morto);
    }
}

TEST_CASE("Filosofos: a ordem e o garcom - a espera vira fila, e nunca circulo") {
    const Visto ordem = Rodar(Rodada::Ordem, 0);
    CHECK_FALSE(ordem.deadlock);
    CHECK(ordem.comeram == 3);
    for (int fora = 0; fora < kFilosofos; ++fora) {
        CAPTURE(fora);
        const Visto garcom = Rodar(Rodada::Garcom, fora);
        CHECK_FALSE(garcom.deadlock);
        CHECK(garcom.comeram == 3);
        CHECK(garcom.mesa.estado[static_cast<size_t>(fora)] == Estado::DeFora);
    }

    // NA ORDEM, quando o penultimo come, os outros quatro estao numa fila que
    // termina nele: o ultimo espera o primeiro, que espera o segundo...
    Mesa m = MesaPosta();
    for (const Passo& p : Roteiro(Rodada::Ordem, 0)) {
        REQUIRE(Aplicar(m, p));
        if (p.acao == Acao::Come && p.filosofo == kFilosofos - 2) break;
    }
    CHECK(Esperas(m).size() == 4u);
    CHECK_FALSE(EmDeadlock(m));
}

TEST_CASE("Filosofos: o leque tem uma garfada no meio, mirada no aluno") {
    CHECK(kGarfadasPorLeque % 2 == 1);
}

TEST_CASE("Filosofos: as rodadas cabem na fase") {
    CHECK(kPrimeiro + static_cast<float>(kRodadas) * kPorRodada == doctest::Approx(Fase::kDuracao));
    CHECK(RodadaDaVez(kRodadas) == RodadaDaVez(0));
    for (int k = 0; k < kRodadas; ++k) {
        for (const Passo& p : Roteiro(RodadaDaVez(k), 2)) CHECK(p.quando < kPorRodada);
    }
}

TEST_CASE("Filosofos: a mesa - os filosofos em volta, os garfos entre eles, tudo acima do aluno") {
    const Ponto c = Centro(1200.0f);
    CHECK(c.x == doctest::Approx(600.0f));
    for (int f = 0; f < kFilosofos; ++f) {
        CAPTURE(f);
        const Ponto p = Lugar(f, 1200.0f);
        CHECK(std::hypot(p.x - c.x, p.y - c.y) == doctest::Approx(kRaioDosFilosofos));
        CHECK(p.y > 58.0f + 13.0f);                 // abaixo do placar (Battle: 12 + 46)
        CHECK(p.y < 460.0f);                        // bem acima do aluno
        // O garfo f fica entre o filosofo f - 1 e o f, perto da mesa.
        const Ponto g = LugarDoGarfo(f, 1200.0f);
        CHECK(std::hypot(g.x - c.x, g.y - c.y) == doctest::Approx(kRaioDosGarfos));
        const Ponto a = Lugar((f + kFilosofos - 1) % kFilosofos, 1200.0f);
        CHECK(std::hypot(g.x - p.x, g.y - p.y) == doctest::Approx(std::hypot(g.x - a.x, g.y - a.y)).epsilon(0.001));
        // Na mao, o garfo fica mais perto de quem o pegou.
        const Ponto mao = NaMao(f, f, 1200.0f);
        CHECK(std::hypot(mao.x - p.x, mao.y - p.y) < std::hypot(g.x - p.x, g.y - p.y));
    }
    // O cerco: kPorAresta pontos na reta entre os dois, sem as pontas.
    const auto cerco = Cerco(0, 1, 1200.0f);
    REQUIRE(static_cast<int>(cerco.size()) == kPorAresta);
    const Ponto a = Lugar(0, 1200.0f);
    const Ponto b = Lugar(1, 1200.0f);
    for (const Ponto& p : cerco) {
        const float cruz = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
        CHECK(std::fabs(cruz) < 0.5f);
        CHECK(std::hypot(p.x - a.x, p.y - a.y) > 1.0f);
        CHECK(std::hypot(p.x - b.x, p.y - b.y) > 1.0f);
    }
}

TEST_CASE("Filosofos: os textos") {
    Mesa m = MesaPosta();
    CHECK(Placar(Rodada::Jantar, m) == "jantar");
    Aplicar(m, {0.0f, 3, Acao::Pega, 3});
    Aplicar(m, {0.0f, 3, Acao::Pega, 4});
    Aplicar(m, {0.0f, 3, Acao::Come});
    CHECK(Placar(Rodada::Ordem, m) == "menor garfo primeiro \xc2\xb7 comendo: Kant");
    CHECK(Placar(Rodada::Garcom, MesaPosta()) == "gar\xc3\xa7om: 4 \xc3\xa0 mesa");
    Mesa morta = MesaPosta();
    Aplicar(morta, {0.0f, 3, Acao::Morre});
    CHECK(Placar(Rodada::Deadlock, morta) == "kill -9 Kant");
}
