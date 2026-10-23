//
// A fase 2 do Vitor: as nuvens e o controle de congestionamento do TCP.
//

#include "doctest.h"

#include <vector>

#include "../Source/Attacks/Tcp.h"
#include "../Source/Attacks/Fase.h"

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
