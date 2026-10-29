//
// A fase 2 do Vitor: as nuvens e o controle de congestionamento do TCP.
//

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <set>
#include <vector>

#include "../Source/Attacks/Vitor/Tcp.h"
#include "../Source/Attacks/Engine/Fase.h"

using namespace Tcp;

TEST_CASE("Tcp: o dente de serra - slow start, evitacao, e a janela pela metade na perda") {
    Conexao c;
    std::vector<int> janelas, perdas;
    for (int k = 0; k < 11; ++k) {
        const Janela j = Enviar(c);
        janelas.push_back(j.Total());
        perdas.push_back(j.perdidos);
        Reconhecer(c, j);
    }
    CHECK(janelas == std::vector<int>{1, 2, 4, 8, 16, 8, 9, 10, 11, 5, 6});
    CHECK(perdas == std::vector<int>{0, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0});
    CHECK(c.cwnd == 7);                     // 11 / 2, e mais duas
    CHECK(c.estado == Estado::Evitacao);
    CHECK(c.rtt == 11);
}

TEST_CASE("Tcp: o que se perde e retransmitido primeiro, e o roteador descarta a cauda") {
    Conexao c;
    c.cwnd = 16;
    c.estado = Estado::Evitacao;
    const Janela grande = Enviar(c);
    CHECK(grande.perdidos == 16 - kCapacidade);
    for (int i = 0; i < grande.Total(); ++i) CHECK(grande.Perdido(i) == (i >= kCapacidade));
    CHECK(Reconhecer(c, grande) == Desfecho::Cortou);
    CHECK(c.ssthresh == 8);
    CHECK(c.cwnd == 8);

    const Janela seguinte = Enviar(c);
    CHECK(seguinte.retransmitidos == 6);
    CHECK(seguinte.novos == 2);
    CHECK(seguinte.perdidos == 0);
    for (int i = 0; i < seguinte.Total(); ++i) CHECK(seguinte.Retransmitido(i) == (i < 6));
    CHECK(Reconhecer(c, seguinte) == Desfecho::Cresceu);
    CHECK(c.cwnd == 9);
}

TEST_CASE("Tcp: o timeout volta ao slow start com cwnd = 1, e o ssthresh cai a metade") {
    Conexao c;
    c.cwnd = 10;
    c.estado = Estado::Evitacao;
    c.aRetransmitir = 2;
    Timeout(c);
    CHECK(c.cwnd == 1);
    CHECK(c.ssthresh == 5);
    CHECK(c.estado == Estado::SlowStart);
    CHECK(c.aRetransmitir == 0);
    // e dobra ate o ssthresh, e dali cresce de um
    Reconhecer(c, Enviar(c));
    Reconhecer(c, Enviar(c));
    CHECK(c.cwnd == 4);
    CHECK(Reconhecer(c, Enviar(c)) == Desfecho::Dobrou);
    CHECK(c.cwnd == 5);
    CHECK(c.estado == Estado::Evitacao);
    Conexao pequena;
    pequena.cwnd = 1;
    Timeout(pequena);
    CHECK(pequena.ssthresh == kMenorSsthresh);
}

TEST_CASE("Tcp: os RTTs cabem na fase, e a maior janela sai inteira antes do RTT seguinte") {
    // ONZE JANELAS NA FASE: a primeira sai quando as nuvens acabam de
    // processar pela primeira vez, e a decima primeira antes de a fase acabar.
    CHECK(kFormam + kProcessa + 10.0f * kRtt < Fase::kDuracao);
    // A janela de 16 sai em 0,75 s; o roteador encaminha a capacidade inteira
    // antes de a janela seguinte chegar a ele.
    CHECK(15.0f * kEntreSegmentos <= kDescansa + 0.001f);
    CHECK(static_cast<float>(kCapacidade) * kEntreEncaminhados < kRtt);
}

TEST_CASE("Tcp: o roteador fica no meio, longe do placar e do aluno, e o grafico no canto") {
    const Ponto r = Roteador(1200.0f);
    CHECK(r.x == doctest::Approx(600.0f));
    // ABAIXO DO PROFESSOR (a Varredura o leva a 18% da altura da tela, 144, e
    // o cartao dele tem 64 px de meia altura), e acima do aluno.
    CHECK(r.y - kChegou > 144.0f + 64.0f);
    CHECK(r.y < 450.0f);

    const Retangulo g = Grafico(1200.0f);
    CHECK(g.x + g.w <= 1200.0f);
    CHECK(g.x > 336.0f + 400.0f);             // a direita do placar (Battle: 16 + 320)
    // LONGE DO PROFESSOR E DAS NUVENS: a varredura para a kMargemDaVarredura
    // da borda, o cartao dele tem 64 px de meia largura, e a nuvem da direita
    // (125 px ao lado e 55 acima dele, a 144 de altura, 2,2 x 32 px) acaba
    // em cima do grafico.
    CHECK(1200.0f - kMargemDaVarredura + 64.0f < g.x);
    CHECK(144.0f - 55.0f + 2.2f * 16.0f < g.y);
    CHECK(g.y + g.h < kAlturaDoRoteador - 60.0f);
    const Ponto zero = NoGrafico(0, 0, 1200.0f);
    const Ponto cheio = NoGrafico(kRttsNoGrafico - 1, kMaiorNoGrafico, 1200.0f);
    CHECK(zero.x > g.x);
    CHECK(cheio.x < g.x + g.w);
    CHECK(cheio.y > g.y);
    CHECK(zero.y < g.y + g.h);
    CHECK(NoGrafico(99, 99, 1200.0f).x == doctest::Approx(cheio.x));
    CHECK(NoGrafico(99, 99, 1200.0f).y == doctest::Approx(cheio.y));
    CHECK(NoGrafico(3, 8, 1200.0f).y < NoGrafico(3, 4, 1200.0f).y);   // mais cwnd, mais alto
    CHECK(GiroDoDescarte(0) == doctest::Approx(0.0f));
    CHECK(GiroDoDescarte(1) == doctest::Approx(15.0f));
    CHECK(GiroDoDescarte(3) == doctest::Approx(0.0f));
}

TEST_CASE("Tcp: o roteador mira adiante de quem anda, e em cima de quem para") {
    const Ponto de{600.0f, 400.0f};
    const Ponto aluno{600.0f, 740.0f};      // 340 px: um segundo de voo a 340 px/s
    const Ponto parado = Mira(de, aluno, {0.0f, 0.0f}, 340.0f, 1.0f);
    CHECK(parado.x == doctest::Approx(aluno.x));
    CHECK(parado.y == doctest::Approx(aluno.y));
    const Ponto adiante = Mira(de, aluno, {100.0f, 0.0f}, 340.0f, 1.0f);
    CHECK(adiante.x == doctest::Approx(700.0f));
    const Ponto metade = Mira(de, aluno, {100.0f, 0.0f}, 340.0f, 0.5f);
    CHECK(metade.x == doctest::Approx(650.0f));
    // OS NOVOS SE ALTERNAM entre onde ele esta e onde estara; o retransmitido
    // vai sempre adiante.
    CHECK(Antecipacao(0, false) == doctest::Approx(0.0f));
    CHECK(Antecipacao(1, false) == doctest::Approx(1.0f));
    CHECK(Antecipacao(2, false) == doctest::Approx(0.0f));
    for (int i = 0; i < 4; ++i) CHECK(Antecipacao(i, true) == doctest::Approx(1.0f));
}

TEST_CASE("Tcp: os textos") {
    Conexao c;
    CHECK(Placar(c) == "cwnd = 1 \xc2\xb7 slow start");
    c.cwnd = 9;
    c.estado = Estado::Evitacao;
    CHECK(Placar(c) == "cwnd = 9 \xc2\xb7 evita\xc3\xa7\xc3\xa3o");
    CHECK(PlacarDaPerda(c) == "3 ACKs duplicados \xc2\xb7 cwnd = 9");
    CHECK(PlacarDoTimeout(c) == "timeout \xc2\xb7 cwnd = 9");
    CHECK(Fila(7) == "roteador \xc2\xb7 7/10");
    CHECK(Fila(15) == "roteador \xc2\xb7 10/10");
}

// ---- A VARIACAO E O DEGRAU ---------------------------------------------------

TEST_CASE("Tcp: a rede sorteada - toda combinacao tem slow start, perda e o dente de serra") {
    // QUANTAS JANELAS UMA FASE MANDA: a primeira quando as nuvens se formam,
    // uma por RTT ate a fase acabar.
    const int janelas = static_cast<int>((Fase::kDuracao - kFormam) / kRtt) + 1;
    REQUIRE(janelas >= 10);
    for (const Dificuldade::Nivel nivel : Dificuldade::kNiveis)
    for (const int sorteada : kCapacidades) {
        for (const int ssthresh : kSsthreshs) {
            CAPTURE(static_cast<int>(nivel));
            CAPTURE(sorteada);
            CAPTURE(ssthresh);
            Plano p;
            p.capacidade = sorteada;
            p.ssthreshInicial = ssthresh;
            p = NoDegrau(p, DegrauDo(nivel));
            const int capacidade = p.capacidade;
            CHECK(capacidade >= kMenorCapacidade);
            if (nivel == Dificuldade::Nivel::Normal) CHECK(capacidade == sorteada);
            else CHECK(capacidade < sorteada);
            Conexao c = Abrir(p);
            CHECK(c.cwnd == kJanelaInicial);
            CHECK(c.estado == Estado::SlowStart);
            int cortes = 0, primeiroCorte = -1;
            for (int i = 0; i < janelas; ++i) {
                const Janela j = Enviar(c);
                CHECK(j.Total() <= kMaiorNoGrafico);        // cabe no grafico
                CHECK(j.Total() - j.perdidos <= capacidade); // o roteador nao passa da capacidade
                if (Reconhecer(c, j) == Desfecho::Cortou) {
                    ++cortes;
                    if (primeiroCorte < 0) primeiroCorte = i;
                }
            }
            CHECK(cortes >= 1);                  // a serra tem dente
            CHECK(primeiroCorte < janelas - 2);  // e ele vem com tempo de ver a janela cair e crescer
            CHECK(static_cast<float>(capacidade) * DegrauDo(Dificuldade::Nivel::Dificil).entreEncaminhados < kRtt);
            CHECK(static_cast<float>(capacidade) * kEntreEncaminhados < kRtt);
        }
    }
}

TEST_CASE("Tcp: o sorteio cobre as capacidades, os ssthresh e a faixa do roteador") {
    std::set<int> capacidades, ssthreshs;
    float menor = 1.0f, maior = 0.0f;
    for (std::uint32_t semente = 0; semente < 300; ++semente) {
        Variacao::Sorteio s(semente);
        const Plano p = Sortear(s);
        capacidades.insert(p.capacidade);
        ssthreshs.insert(p.ssthreshInicial);
        CHECK(p.fracaoDoRoteador >= kMenorFracaoDoRoteador);
        CHECK(p.fracaoDoRoteador < kMaiorFracaoDoRoteador);
        menor = std::min(menor, p.fracaoDoRoteador);
        maior = std::max(maior, p.fracaoDoRoteador);
        // O ROTEADOR NAO ENCOSTA NO GRAFICO, onde quer que caia.
        const Ponto r = Roteador(1200.0f, p.fracaoDoRoteador);
        const Retangulo g = Grafico(1200.0f);
        CHECK((r.x < g.x - 80.0f || r.y > g.y + g.h + 40.0f));
    }
    CHECK(capacidades == std::set<int>(std::begin(kCapacidades), std::end(kCapacidades)));
    CHECK(ssthreshs == std::set<int>(std::begin(kSsthreshs), std::end(kSsthreshs)));
    CHECK(menor < 0.42f);
    CHECK(maior > 0.58f);
    CHECK(PlanoDeSempre().capacidade == kCapacidade);
    CHECK(PlanoDeSempre().ssthreshInicial == kSsthreshInicial);
    CHECK(Roteador(1200.0f, PlanoDeSempre().fracaoDoRoteador).x == doctest::Approx(Roteador(1200.0f).x));
    CHECK(Fila(7, 12) == "roteador \xc2\xb7 7/12");
    CHECK(Fila(15, 8) == "roteador \xc2\xb7 8/8");
}

TEST_CASE("Tcp: o degrau - o Normal e o de sempre, o Dificil encaminha mais e mais depressa") {
    const Degrau n = DegrauDo(Dificuldade::Nivel::Normal);
    CHECK(n.entreEncaminhados == doctest::Approx(kEntreEncaminhados));
    CHECK(n.rapidez == doctest::Approx(1.0f));
    CHECK(n.anelDoDescarte == kAnelDoDescarte);
    const Degrau d = DegrauDo(Dificuldade::Nivel::Dificil);
    CHECK(d.entreEncaminhados < n.entreEncaminhados);
    CHECK(d.rapidez > n.rapidez);
    CHECK(d.anelDoDescarte > n.anelDoDescarte);
    // TRES ANEIS SEGUIDOS fecham o circulo com qualquer numero de cacos.
    for (const int anel : {n.anelDoDescarte, d.anelDoDescarte}) {
        CHECK(GiroDoDescarte(3, anel) == doctest::Approx(0.0f));
        CHECK(GiroDoDescarte(1, anel) * 3.0f == doctest::Approx(360.0f / static_cast<float>(anel)));
    }
}
