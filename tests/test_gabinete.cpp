// O gabinete. Camada pura.
//
// Duas coisas vivem aqui, e as duas sao perigosas pelo mesmo motivo: so se
// manifestam numa maquina sem teclado, no corredor do departamento, onde
// ninguem esta olhando o terminal.
//
// A LEITURA DA LINHA DE COMANDO decide se a tela e cheia e se ha saida. Um erro
// de digitacao no script que abre o jogo nao pode impedi-lo de abrir, nem
// ligar o modo errado em silencio.
//
// A CONTAGEM decide quando o jogo volta ao menu sozinho. Errar para menos tira
// o aluno do meio da escolha; errar para mais deixa o perfil de um aluno aberto
// para o seguinte.

#include "doctest.h"

#include "../Source/Gabinete.h"

// ---------------------------------------------------------------------------
// A linha de comando
// ---------------------------------------------------------------------------

TEST_CASE("Gabinete: sem argumentos o jogo abre em janela, sem ociosidade") {

    // O PADRAO E A JANELA. Este e o caso do CLion e o de rodar o executavel na
    // mao: tela cheia e volta-ao-menu atrapalhariam quem esta desenvolvendo.
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

    // Sem --arcade de proposito: e assim que se testa a volta ao menu numa
    // janela, sem tela cheia e sem esperar um minuto.
    CHECK(Gabinete::Ler({"--ociosidade", "20"}).ociosidade == doctest::Approx(20.0f));
    CHECK(Gabinete::Ler({"--ociosidade", "20"}).arcade == false);

    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "20"}).ociosidade == doctest::Approx(20.0f));
}

TEST_CASE("Gabinete: a ordem dos argumentos nao importa") {

    // --arcade preenche a ociosidade padrao, entao se ele fosse processado
    // DEPOIS de --ociosidade ele apagaria o valor pedido. Por isso o padrao
    // entra fora do laco.
    const Gabinete::Configuracao antes = Gabinete::Ler({"--ociosidade", "20", "--arcade"});
    const Gabinete::Configuracao depois = Gabinete::Ler({"--arcade", "--ociosidade", "20"});

    CHECK(antes.arcade == true);
    CHECK(depois.arcade == true);
    CHECK(antes.ociosidade == doctest::Approx(20.0f));
    CHECK(depois.ociosidade == doctest::Approx(20.0f));
}

TEST_CASE("Gabinete: --ociosidade 0 desliga a volta ao menu, mesmo no arcade") {

    // E uma escolha de quem monta: um gabinete em sala fechada pode preferir
    // que a tela fique onde o aluno deixou.
    const Gabinete::Configuracao c = Gabinete::Ler({"--arcade", "--ociosidade", "0"});

    CHECK(c.arcade == true);
    CHECK(c.ociosidade == doctest::Approx(0.0f));
}

TEST_CASE("Gabinete: valor invalido cai no padrao e nao engole o resto") {

    // ESTE E O TESTE QUE IMPORTA NO GABINETE. O jogo tem de abrir de qualquer
    // jeito, e as outras flags continuam valendo.

    // Valor que nao e numero.
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "abc"}).arcade == true);
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "abc"}).ociosidade
          == doctest::Approx(Gabinete::kOciosidadePadrao));

    // Numero com sujeira colada. "20s" nao vale 20: quem escreveu isso quis
    // dizer outra coisa, e adivinhar seria pior que usar o padrao.
    CHECK(Gabinete::Ler({"--ociosidade", "20s"}).ociosidade == doctest::Approx(0.0f));

    // Valor faltando no fim da linha.
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade"}).arcade == true);

    // Valor faltando NO MEIO: a flag seguinte nao pode ser consumida como valor.
    CHECK(Gabinete::Ler({"--ociosidade", "--arcade"}).arcade == true);

    // Negativo e desligado, e nao uma contagem que esgota no primeiro quadro.
    CHECK(Gabinete::Ler({"--arcade", "--ociosidade", "-5"}).ociosidade == doctest::Approx(0.0f));
}

TEST_CASE("Gabinete: argumento desconhecido e ignorado sem atrapalhar") {

    const Gabinete::Configuracao c = Gabinete::Ler({"--xpto", "--arcade", "sobra"});

    CHECK(c.arcade == true);
    CHECK(c.ociosidade == doctest::Approx(Gabinete::kOciosidadePadrao));
}

// ---------------------------------------------------------------------------
// A contagem
// ---------------------------------------------------------------------------

TEST_CASE("Contagem: limite zero nunca esgota") {

    // E o estado de quem esta desenvolvendo. Se esgotasse, a janela voltaria ao
    // menu sozinha no meio de um teste.
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

    // A PROPRIEDADE QUE DEFINE A ESPERA. "Parado por um minuto" nao e "um
    // minuto de jogo somado": qualquer toque recomeca do zero.
    Gabinete::Contagem c{1.0f};

    CHECK(c.Passou(0.9f, true) == false);
    CHECK(c.Passou(0.9f, false) == false);   // alguem mexeu
    CHECK(c.Acumulado() == doctest::Approx(0.0f));
    CHECK(c.Passou(0.9f, true) == false);    // nao esgota: a conta reiniciou
    CHECK(c.Passou(0.2f, true) == true);
}

TEST_CASE("Contagem: esgota UMA vez e recomeca") {

    // Sem isto, quem chama pediria a troca de cena em todo quadro depois do
    // primeiro - sessenta pedidos por segundo, e a cena de destino recarregando
    // sem parar.
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

    // Somar 0,016 sessenta vezes nao da 1,0 exato em float. O que se exige e
    // que a espera esgote DENTRO de um quadro do limite - nunca antes.
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
