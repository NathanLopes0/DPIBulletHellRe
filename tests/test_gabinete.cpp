// O gabinete.

#include "doctest.h"

#include "../Source/Motor/Gabinete.h"

// A linha de comando.

TEST_CASE("Gabinete: sem argumentos o jogo abre em janela, sem ociosidade") {

    // O padrao e a janela.
    const Gabinete::Configuracao c = Gabinete::Ler({});

    CHECK(c.arcade == false);
    CHECK(c.ociosidade == doctest::Approx(0.0f));
}

TEST_CASE("Gabinete: --arcade liga o modo e traz a ociosidade padrao") {

    const Gabinete::Configuracao c = Gabinete::Ler({"--arcade"});

    CHECK(c.arcade == true);
    CHECK(c.ociosidade == doctest::Approx(Gabinete::kOciosidadePadrao));
}

TEST_CASE("Gabinete: --ociosidade vale com e sem --arcade") {

    CHECK(Gabinete::Ler({"--ociosidade", "20"}).ociosidade == doctest::Approx(20.0f));
    CHECK(Gabinete::Ler({"--ociosidade", "20"}).arcade == false);

    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "20"}).ociosidade == doctest::Approx(20.0f));
}

TEST_CASE("Gabinete: a ordem dos argumentos nao importa") {

    // ---- Arcade preenche a ociosidade padrao, entao se ele fosse processado ----
    const Gabinete::Configuracao antes = Gabinete::Ler({"--ociosidade", "20", "--arcade"});
    const Gabinete::Configuracao depois = Gabinete::Ler({"--arcade", "--ociosidade", "20"});

    CHECK(antes.arcade == true);
    CHECK(depois.arcade == true);
    CHECK(antes.ociosidade == doctest::Approx(20.0f));
    CHECK(depois.ociosidade == doctest::Approx(20.0f));
}

TEST_CASE("Gabinete: --ociosidade 0 desliga a volta ao menu, mesmo no arcade") {

    const Gabinete::Configuracao c = Gabinete::Ler({"--arcade", "--ociosidade", "0"});

    CHECK(c.arcade == true);
    CHECK(c.ociosidade == doctest::Approx(0.0f));
}

TEST_CASE("Gabinete: valor invalido cai no padrao e nao engole o resto") {

    // Este e o teste que importa no gabinete.

    // Valor que nao e numero.
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "abc"}).arcade == true);
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "abc"}).ociosidade
          == doctest::Approx(Gabinete::kOciosidadePadrao));

    // Numero com sujeira colada.
    CHECK(Gabinete::Ler({"--ociosidade", "20s"}).ociosidade == doctest::Approx(0.0f));

    // Valor faltando no fim da linha.
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade"}).arcade == true);

    // Valor faltando no meio: a flag seguinte nao pode ser consumida como valor.
    CHECK(Gabinete::Ler({"--ociosidade", "--arcade"}).arcade == true);

    // Negativo e desligado, e nao uma contagem que esgota no primeiro quadro.
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "-5"}).ociosidade == doctest::Approx(0.0f));
}

TEST_CASE("Gabinete: argumento desconhecido e ignorado sem atrapalhar") {

    const Gabinete::Configuracao c = Gabinete::Ler({"--xpto", "--arcade", "sobra"});

    CHECK(c.arcade == true);
    CHECK(c.ociosidade == doctest::Approx(Gabinete::kOciosidadePadrao));
}

// A contagem.

TEST_CASE("Contagem: limite zero nunca esgota") {

    // E o estado de quem esta desenvolvendo.
    Gabinete::Contagem c{0.0f};

    CHECK(c.Ligada() == false);
    for (int i = 0; i < 1000; ++i) {
        CHECK(c.Passou(1.0f, true) == false);
    }
}

TEST_CASE("Contagem: limite negativo tambem nunca esgota") {

    Gabinete::Contagem c{-3.0f};

    CHECK(c.Ligada() == false);
    CHECK(c.Passou(10.0f, true) == false);
}

TEST_CASE("Contagem: esgota ao atingir o limite, e nao antes") {

    Gabinete::Contagem c{1.0f};

    CHECK(c.Passou(0.25f, true) == false);
    CHECK(c.Passou(0.25f, true) == false);
    CHECK(c.Passou(0.25f, true) == false);
    CHECK(c.Passou(0.25f, true) == true);
}

TEST_CASE("Contagem: a condicao falsa zera a conta") {

    // A propriedade que define a espera.
    Gabinete::Contagem c{1.0f};

    CHECK(c.Passou(0.9f, true) == false);
    CHECK(c.Passou(0.9f, false) == false);   // alguem mexeu
    CHECK(c.Acumulado() == doctest::Approx(0.0f));
    CHECK(c.Passou(0.9f, true) == false);    // nao esgota: a conta reiniciou
    CHECK(c.Passou(0.2f, true) == true);
}

TEST_CASE("Contagem: esgota UMA vez e recomeca") {

    // Sem isto, quem chama pediria a troca de cena em todo quadro depois do primeiro.
    Gabinete::Contagem c{1.0f};

    CHECK(c.Passou(1.0f, true) == true);
    CHECK(c.Passou(0.5f, true) == false);
    CHECK(c.Passou(0.5f, true) == true);
}

TEST_CASE("Contagem: Reiniciar zera sem esgotar") {

    Gabinete::Contagem c{1.0f};

    CHECK(c.Passou(0.9f, true) == false);
    c.Reiniciar();
    CHECK(c.Acumulado() == doctest::Approx(0.0f));
    CHECK(c.Passou(0.9f, true) == false);
}

TEST_CASE("Contagem: passo de quadro de verdade esgota perto do limite") {

    // Somar 0,016 sessenta vezes nao da 1,0 exato em float.
    const float passo = 1.0f / 60.0f;
    Gabinete::Contagem c{Gabinete::kSegurarParaSair};

    float decorrido = 0.0f;
    int quadros = 0;
    while (!c.Passou(passo, true)) {
        decorrido += passo;
        ++quadros;
        REQUIRE(quadros < 1000);   // nao esgotou nunca: o teste seguinte mentiria
    }
    decorrido += passo;

    CHECK(decorrido >= Gabinete::kSegurarParaSair);
    CHECK(decorrido <= Gabinete::kSegurarParaSair + passo);
}
