//
// O jogo rodando sozinho, para medir e fotografar: a leitura da linha de comando e as contas dos pilotos.
//

#include "doctest.h"

#include <string>
#include <vector>

#include "../Source/Simulacao/Simulacao.h"

using namespace Simulacao;

TEST_CASE("Simulacao: sem parametros, o jogo de sempre") {
    const Configuracao c = Ler({});
    CHECK_FALSE(c.Ativa());
    CHECK(c.batalha.empty());
    CHECK(c.passo == 0.0f);
    CHECK(c.piloto == Piloto::Nenhum);
    CHECK_FALSE(c.semente.has_value());
    // Os do gabinete e do experimento nao sao daqui, e nao ligam nada.
    CHECK_FALSE(Ler({"--arcade", "--ociosidade", "30", "--condicao", "sem-brilho"}).Ativa());
}

TEST_CASE("Simulacao: a linha da medicao se le inteira") {
    std::vector<std::string> problemas;
    const Configuracao c = Ler({"--batalha", "10", "--jogadores", "2", "--orientador", "INF213", "--dificil",
                                "--semente", "73", "--variacao", "5", "--passo", "0.016", "--piloto", "segue",
                                "--atira", "--poder", "--relatorio", "--sair-no-fim", "--estado", "StateTwo"},
                               &problemas);
    CHECK(problemas.empty());
    CHECK(c.Ativa());
    CHECK(c.batalha == "10");
    CHECK(c.jogadores == 2);
    CHECK(c.orientador == "INF213");
    CHECK(c.dificil);
    CHECK(c.estado == "StateTwo");
    REQUIRE(c.semente.has_value());
    CHECK(*c.semente == 73u);
    REQUIRE(c.variacao.has_value());
    CHECK(*c.variacao == 5u);
    // O passo e lido como o arnes antigo lia: o mesmo float, bit a bit.
    CHECK(c.passo == static_cast<float>(0.016));
    CHECK(c.piloto == Piloto::Segue);
    CHECK(c.atira);
    CHECK(c.poder);
    CHECK(c.relatorio);
    CHECK(c.sairNoFim);
}

TEST_CASE("Simulacao: o roteiro, as fotos, a janela e o fim forcado") {
    std::vector<std::string> problemas;
    const Configuracao c = Ler({"--teclas", "Space@100,8@200,Return@290", "--fotos", "380,470", "--fotos-em", "/tmp/x",
                                "--janela", "1920x1080", "--fim-em", "600", "--fim-aprovado", "--fim-nota", "74.5",
                                "--pausa-em", "300", "--foco-em", "310", "--imortal", "--piloto", "vaivem"},
                               &problemas);
    CHECK(problemas.empty());
    REQUIRE(c.teclas.size() == 3u);
    CHECK(c.teclas[0].nome == "Space");
    CHECK(c.teclas[0].quadro == 100);
    CHECK(c.teclas[1].nome == "8");
    CHECK(c.teclas[2].nome == "Return");
    CHECK(c.fotos == std::vector<int>{380, 470});
    CHECK(c.pastaDasFotos == "/tmp/x");
    CHECK(c.larguraDaJanela == 1920);
    CHECK(c.alturaDaJanela == 1080);
    CHECK(c.fimNoQuadro == 600);
    CHECK(c.fimAprovado);
    REQUIRE(c.fimComNota.has_value());
    CHECK(*c.fimComNota == doctest::Approx(74.5f));
    CHECK(c.pausaNoQuadro == 300);
    CHECK(c.focoNoQuadro == 310);
    CHECK(c.imortal);
    CHECK(c.piloto == Piloto::VaiVem);
}

TEST_CASE("Simulacao: o que nao se le e dito, e ignorado") {
    std::vector<std::string> problemas;
    const Configuracao c = Ler({"--jogadores", "3", "--passo", "rapido", "--piloto", "voador", "--janela", "grande",
                                "--teclas", "Space,B@x", "--semente"},
                               &problemas);
    CHECK(problemas.size() == 7u);
    CHECK(c.jogadores == 1);
    CHECK(c.passo == 0.0f);
    CHECK(c.piloto == Piloto::Nenhum);
    CHECK(c.larguraDaJanela == 0);
    CHECK(c.teclas.empty());
    CHECK_FALSE(c.semente.has_value());
}

TEST_CASE("Simulacao: cada tecla do roteiro fica apertada tres quadros") {
    const Tecla t{"Space", 100};
    CHECK_FALSE(Apertada(t, 99));
    CHECK(Apertada(t, 100));
    CHECK(Apertada(t, 100 + kQuadrosDeTecla - 1));
    CHECK_FALSE(Apertada(t, 100 + kQuadrosDeTecla));
}

TEST_CASE("Simulacao: o piloto anda para o alvo e para perto dele; o vai e vem vira nas bordas") {
    CHECK(Direcao(100.0f, 300.0f) == 1);
    CHECK(Direcao(300.0f, 100.0f) == -1);
    CHECK(Direcao(100.0f, 101.5f) == 0);    // chegou: a menos de 2 px
    CHECK(Direcao(100.0f, 98.5f) == 0);

    CHECK(SentidoDoVaiVem(600.0f, 1.0f) == 1.0f);
    CHECK(SentidoDoVaiVem(600.0f, -1.0f) == -1.0f);
    CHECK(SentidoDoVaiVem(kVaiVemDireita + 1.0f, 1.0f) == -1.0f);
    CHECK(SentidoDoVaiVem(kVaiVemEsquerda - 1.0f, -1.0f) == 1.0f);
}
