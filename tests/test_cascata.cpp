//
// A Maria Lucia refeita: a cascata, o Scrum, o Observer em orbita, o Decorator e o lobisomem.
//

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Attacks/BalaDePrata.h"
#include "../Source/Attacks/Cascata.h"
#include "../Source/Attacks/Decorator.h"
#include "../Source/Attacks/Observer.h"
#include "../Source/Attacks/Scrum.h"

// ---- A cascata ----

TEST_CASE("Cascata: as etapas vem em ordem, uma a cada 4 s, e cabem na fase") {
    using namespace Cascata;
    CHECK(kDuracaoDaEtapa * static_cast<float>(kEtapas.size()) == doctest::Approx(Fase::kDuracao));
    CHECK(EtapaNoTempo(0.0f) == 0);
    CHECK(EtapaNoTempo(4.1f) == 1);
    CHECK(EtapaNoTempo(19.9f) == 4);
    CHECK(EtapaNoTempo(25.0f) == 5);
    CHECK(EtapaNoTempo(-1.0f) == 0);
    CHECK(std::string(CaraDaEtapa(0)) == "Pedido");
    CHECK(std::string(CaraDaEtapa(4)) == "Debito");
}

TEST_CASE("Cascata: a cachoeira acelera ladeira abaixo, e o Dificil mais") {
    using namespace Cascata;
    for (int k = 0; k + 1 < 5; ++k) {
        CHECK(EntreQuedas(k + 1, Dificuldade::Nivel::Normal) < EntreQuedas(k, Dificuldade::Nivel::Normal));
        CHECK(VelocidadeDaQueda(k + 1) > VelocidadeDaQueda(k));
        CHECK(EntreQuedas(k, Dificuldade::Nivel::Dificil) < EntreQuedas(k, Dificuldade::Nivel::Normal));
    }
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).entreMudancas < DegrauDo(Dificuldade::Nivel::Normal).entreMudancas);
}

TEST_CASE("Cascata: a mudanca custa mais a cada etapa - a curva de Boehm em pontos de poker") {
    using namespace Cascata;
    CHECK(Custo(0) == 1);
    CHECK(Custo(1) == 2);
    CHECK(Custo(2) == 3);
    CHECK(Custo(3) == 5);
    CHECK(Custo(4) == 8);
    CHECK(PlacarDaMudanca(3) == "Mudan\xc3\xa7" "a nos Testes: custo \xc3\x97" "5");
    CHECK(TirosDoCusto(1) == 2);
    CHECK(TirosDoCusto(4) == kMaisNoAnel);   // o anel tem teto
    // As cortinas deixam um vao no meio da caixa.
    CHECK(kBordaDaQueda > 40.0f);
    // A mudanca desce a escada inteira dentro de uma etapa.
    CHECK(kPorCaixa * 4.0f < kDuracaoDaEtapa);
}

TEST_CASE("Cascata: o pedido vem de fora, por um dos lados, da metade de baixo") {
    using namespace Cascata;
    std::set<bool> lados;
    for (std::uint32_t s = 0; s < 40; ++s) {
        const Vector2 o = OrigemDoPedido(s, 1200.0f, 746.0f);
        CHECK((o.x < 0.0f || o.x > 1200.0f));
        CHECK(o.y >= 746.0f * 0.45f);
        CHECK(o.y <= 746.0f);
        lados.insert(o.x < 0.0f);
    }
    CHECK(lados.size() == 2u);
}

// ---- O Scrum ----

TEST_CASE("Scrum: o backlog, cinco sprints de tempo fixo e a retrospectiva fecham a fase") {
    using namespace Scrum;
    CHECK(kBacklog + kSprint * static_cast<float>(kSprints) + kRetrospectiva == doctest::Approx(Fase::kDuracao));
    CHECK(kRetrospectiva > 0.0f);
    CHECK(SprintNoTempo(0.2f) == -1);
    CHECK(SprintNoTempo(kBacklog + 0.1f) == 0);
    CHECK(SprintNoTempo(InicioDaSprint(3) + 0.1f) == 3);
    CHECK(SprintNoTempo(19.5f) == kSprints);
    CHECK(Dailies(0.0f) == 0);
    CHECK(Dailies(kDailies[0]) == 1);
    CHECK(Dailies(kSprint - 0.01f) == 3);
    // A historia chega a FEITO e e entregue dentro da sprint dela.
    CHECK(kDailies.back() < kSprint);
    CHECK(kDailiesAteAEntrega == static_cast<int>(kDailies.size()));
}

TEST_CASE("Scrum: a velocidade sobe a cada sprint, e os pontos sao os do poker") {
    using namespace Scrum;
    for (int k = 0; k + 1 < kSprints; ++k) CHECK(Historias(k + 1, Dificuldade::Nivel::Normal) > Historias(k, Dificuldade::Nivel::Normal));
    CHECK(Historias(0, Dificuldade::Nivel::Dificil) > Historias(0, Dificuldade::Nivel::Normal));
    Variacao::Sorteio s(9u);
    const std::set<int> baralho(kBaralho.begin(), kBaralho.end());
    for (const int p : Pontos(s, 40)) CHECK(baralho.count(p) == 1);
    CHECK(TirosDaHistoria(8) > TirosDaHistoria(1));
    CHECK(Cara(5) == "P5");
    CHECK(Placar(2, 1, 13) == "Sprint 3  \xc2\xb7  daily 1  \xc2\xb7  faltam 13 pontos");
    CHECK(Placar(0, 0, 1) == "Sprint 1  \xc2\xb7  faltam 1 ponto");
}

TEST_CASE("Scrum: as vagas do quadro - tres colunas, a pilha de cima para baixo") {
    using namespace Scrum;
    const Vector2 a = Vaga(0, 0, 100.0f, 900.0f, 100.0f);
    const Vector2 b = Vaga(1, 0, 100.0f, 900.0f, 100.0f);
    const Vector2 c = Vaga(2, 2, 100.0f, 900.0f, 100.0f);
    CHECK(a.x == doctest::Approx(250.0f));
    CHECK(b.x == doctest::Approx(550.0f));
    CHECK(c.y == doctest::Approx(100.0f + kTopoDasVagas + 2.0f * kEntreHistorias));
    // A sprint mais cheia ainda cabe na coluna.
    CHECK(Historias(kSprints - 1, Dificuldade::Nivel::Dificil) <= 7);
}

// ---- Os padroes de projeto ----

TEST_CASE("Observer: os olhos orbitam o aluno, repartidos, e a orbita fecha") {
    using namespace Observer;
    const Vector2 aluno(600.0f, 600.0f);
    const Vector2 o0 = NaOrbita(aluno, 0, 4, 0.0f, 200.0f);
    const Vector2 o2 = NaOrbita(aluno, 2, 4, 0.0f, 200.0f);
    CHECK(o0.x == doctest::Approx(800.0f));
    CHECK(o2.x == doctest::Approx(400.0f));      // do outro lado
    CHECK(NaOrbita(aluno, 0, 4, 1.0f, 200.0f).x != doctest::Approx(800.0f));   // gira
    CHECK(Raio(0.0f) == doctest::Approx(kRaioInicial));
    CHECK(Raio(kFimDoObserver) == doctest::Approx(kRaioFinal));
    CHECK(Raio(5.0f) < Raio(1.0f));
}

TEST_CASE("Decorator: a composicao no placar, de dentro para fora") {
    using namespace Decorator;
    CHECK(Placar(0) == "bala");
    CHECK(Placar(1) == "new R\xc3\xa1pida(bala)");
    CHECK(Placar(3) == "new Teleguiada(new Dividida(new R\xc3\xa1pida(bala)))");
    CHECK(Placar(9) == Placar(3));
    CHECK(Cara(2) == "D2");
    CHECK(kEntreCamadas * static_cast<float>(kCamadas) < 3.0f);   // decorada a tempo de importar
    CHECK(DegrauDo(Dificuldade::Nivel::Dificil).entreComponentes < DegrauDo(Dificuldade::Nivel::Normal).entreComponentes);
}

TEST_CASE("Decorator: o componente sai para fora, e a Teleguiada vira aos poucos pelo lado curto") {
    using namespace Decorator;
    // Nasce ao lado da professora, longe da linha de quem mira nela, e desce para fora.
    CHECK(kAoLado > 60.0f);
    CHECK(Saida(1) < 90.0f);
    CHECK(Saida(-1) > 90.0f);
    CHECK(Saida(1) + Saida(-1) == doctest::Approx(180.0f));
    CHECK(Saida(1) > 0.0f);   // para baixo, e nao para o lado
    // Indo para a direita (0), com o aluno embaixo: vira para baixo, no maximo o giro do quadro.
    const float giro = Math::ToRadians(kGiroDaTeleguiada) * 0.1f;
    CHECK(Guiar(0.0f, Vector2(0.0f, 100.0f), 0.1f) == doctest::Approx(giro));
    CHECK(Guiar(0.0f, Vector2(0.0f, -100.0f), 0.1f) == doctest::Approx(-giro));
    // Quase alinhada: chega ao rumo exato, sem passar dele.
    const float quase = Math::Atan2(100.0f, 100.0f) - 0.01f;
    CHECK(Guiar(quase, Vector2(100.0f, 100.0f), 0.1f) == doctest::Approx(Math::Atan2(100.0f, 100.0f)));
    // Pelo lado curto, atravessando o +-180.
    CHECK(Guiar(Math::Pi - 0.05f, Vector2(-100.0f, -1.0f), 0.1f) > Math::Pi - 0.05f);
    // A guia acaba antes de a bala dar a volta no aluno.
    CHECK(kGiroDaTeleguiada * kGuia < 180.0f);
}

// ---- A repescagem: o lobisomem ----

TEST_CASE("Bala de prata: o lobisomem persegue, cresce e nao passa do alvo") {
    using namespace BalaDePrata;
    CHECK(RapidezDoLobisomem(20.0f) > RapidezDoLobisomem(0.0f));
    const Vector2 lobo(0.0f, 0.0f), aluno(100.0f, 0.0f);
    CHECK(PassoDoLobisomem(lobo, aluno, 50.0f, 1.0f).x == doctest::Approx(50.0f));
    CHECK(PassoDoLobisomem(lobo, aluno, 500.0f, 1.0f).x == doctest::Approx(100.0f));   // nao passa
    CHECK(Atingiu(Vector2(30.0f, 0.0f), lobo, 1.0f));
    CHECK_FALSE(Atingiu(Vector2(60.0f, 0.0f), lobo, 1.0f));
    CHECK(Atingiu(Vector2(60.0f, 0.0f), lobo, 2.0f));                                 // maior, pega mais
    CHECK(EscalaDoLobisomem(0) == doctest::Approx(kEscalaInicial));
    CHECK(EscalaDoLobisomem(3) > EscalaDoLobisomem(0));
    CHECK(EscalaDoLobisomem(100) == doctest::Approx(kMaiorEscala));
    CHECK(NaoMatou("IA") == "IA? N\xc3\xa3o matou o lobisomem.");
}
