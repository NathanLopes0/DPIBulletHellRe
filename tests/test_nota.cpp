// A curva da nota e o teto por dano. Camada pura.
//
// O que estes testes protegem nao e um numero bonito: e a propriedade de que a
// nota continua servindo de ranking. Com ganho constante, 100 era questao de
// tempo de tela, e um ranking em que todos tem 100 nao ordena nada.

#include "doctest.h"

#include "../Source/Nota.h"

// ---------------------------------------------------------------- a curva

TEST_CASE("Nota: abaixo de 60 o ganho e inteiro") {

    // A faixa ate a aprovacao tem de continuar tao facil quanto era antes da
    // curva existir - foi pedido explicitamente.
    CHECK(Nota::Ganho(0.0f, 0.56f) == doctest::Approx(0.56f));
    CHECK(Nota::Ganho(40.0f, 0.56f) == doctest::Approx(0.56f));
    CHECK(Nota::Ganho(59.9f, 0.56f) == doctest::Approx(0.56f));
}

TEST_CASE("Nota: em 60 a curva nao tem degrau") {

    // O fator vale exatamente 1 no inicio da faixa dificil. Sem isso haveria um
    // tranco visivel no instante em que o aluno passa de 60.
    CHECK(Nota::Ganho(60.0f, 0.56f) == doctest::Approx(0.56f));
}

TEST_CASE("Nota: acima de 60 o ganho so diminui") {

    float anterior = Nota::Ganho(60.0f, 1.0f);
    for (float n = 61.0f; n <= 100.0f; n += 1.0f) {
        const float agora = Nota::Ganho(n, 1.0f);
        CAPTURE(n);
        CHECK(agora <= anterior);
        anterior = agora;
    }
}

TEST_CASE("Nota: em 100 ainda sobra ganho") {

    // ESTA E A PROPRIEDADE QUE SEPARA OS DOIS FREIOS. Se a curva zerasse em
    // 100, a nota tenderia a 100 sem chegar, e a comemoracao de nota cheia
    // nunca aconteceria. Quem impede o 100 e o teto por dano, nao a curva.
    CHECK(Nota::Ganho(100.0f, 1.0f) > 0.0f);
    CHECK(Nota::Ganho(99.0f, 1.0f) > 0.0f);
}

TEST_CASE("Nota: os ultimos pontos custam muito mais que os primeiros") {

    // Em numeros: subir de 90 para 91 tem de custar bem mais esforco do que
    // subir de 60 para 61.
    //
    // ISTO E UM PISO PARA kExpoente, e nao so uma checagem de monotonia: um
    // expoente abaixo de ~1,1 faz o ganho minguar tao devagar que a curva deixa
    // de morder, e e esta linha que recusa. Se ela falhar depois de alguem
    // baixar o expoente, a falha e a resposta - nao o teste.
    const float em60 = Nota::Ganho(60.0f, 1.0f);
    const float em90 = Nota::Ganho(90.0f, 1.0f);
    const float em99 = Nota::Ganho(99.0f, 1.0f);

    CHECK(em90 < em60 / 2.0f);
    CHECK(em99 < em90 / 2.0f);
}

TEST_CASE("Nota: a nota cheia cabe numa batalha, e so na boa") {

    // O NUMERO QUE JUSTIFICA O EXPOENTE. Medido no jogo rodando: uma batalha
    // entrega entre 268 e 382 acertos no chefe. A nota cheia tem de custar menos
    // que o piso dessa faixa (senao ninguem chega) e mais que a metade dela
    // (senao chega todo mundo, e a nota para de ordenar um ranking).
    int acertos = 0;
    float n = 40.0f;
    while (n < Nota::kNotaMaxima && acertos < 10000) {
        n = Nota::Somar(n, 0.56f, 0);
        ++acertos;
    }

    CAPTURE(acertos);
    CHECK(acertos < 268);
    CHECK(acertos > 134);
}

TEST_CASE("Nota: a curva nao mexe em perda") {

    // Ganho negativo passa reto. Quem perde, perde inteiro - fazer o dano
    // minguar junto deixaria quem esta perto de 100 quase imune.
    CHECK(Nota::Ganho(95.0f, -6.0f) == doctest::Approx(-6.0f));
}

// ---------------------------------------------------------------- o teto

TEST_CASE("Nota: poucos acertos sofridos nao tiram a nota cheia") {

    for (int i = 0; i < Nota::kAcertosQuePerdemACheia; ++i) {
        CAPTURE(i);
        CHECK_FALSE(Nota::PerdeuACheia(i));
        CHECK(Nota::TetoCom(i) == doctest::Approx(Nota::kNotaMaxima));
    }
}

TEST_CASE("Nota: a partir do limite de acertos o teto cai") {

    CHECK(Nota::PerdeuACheia(Nota::kAcertosQuePerdemACheia));
    CHECK(Nota::TetoCom(Nota::kAcertosQuePerdemACheia) == doctest::Approx(Nota::kTetoComDano));
    CHECK(Nota::TetoCom(99) == doctest::Approx(Nota::kTetoComDano));
}

TEST_CASE("Nota: o teto e abaixo de 100, e nao 100") {

    // Se fosse 100, quem levou dano demais empataria com quem fez corrida limpa
    // - e o ranking perderia justamente a distincao que o teto existe para criar.
    CHECK(Nota::kTetoComDano < Nota::kNotaMaxima);
}

TEST_CASE("Nota: somar respeita o teto de quem levou dano") {

    const int muitoDano = Nota::kAcertosQuePerdemACheia;

    // Um ganho enorme nao fura o teto.
    CHECK(Nota::Somar(99.0f, 1000.0f, muitoDano) == doctest::Approx(Nota::kTetoComDano));

    // E sem dano, o mesmo ganho chega a 100.
    CHECK(Nota::Somar(99.0f, 1000.0f, 0) == doctest::Approx(Nota::kNotaMaxima));
}

TEST_CASE("Nota: o teto nao desacelera a subida, so a interrompe") {

    // O dano ja custa nota na hora que acontece; se tambem encolhesse o ganho,
    // puniria duas vezes. Entao ate 99.99 a subida de quem levou dano e igual a
    // de quem nao levou.
    const float semDano = Nota::Somar(70.0f, 1.0f, 0);
    const float comDano = Nota::Somar(70.0f, 1.0f, Nota::kAcertosQuePerdemACheia);
    CHECK(semDano == doctest::Approx(comDano));
}

// ---------------------------------------------------------------- limites

TEST_CASE("Nota: nunca passa de 100 nem cai abaixo de zero") {

    CHECK(Nota::Somar(99.9f, 50.0f, 0) == doctest::Approx(Nota::kNotaMaxima));
    CHECK(Nota::Subtrair(3.0f, 6.0f) == doctest::Approx(0.0f));
    CHECK(Nota::Subtrair(0.0f, 6.0f) == doctest::Approx(0.0f));
}

TEST_CASE("Nota: perder e perder, qualquer que seja a nota") {

    // A perda e a mesma em 40 e em 99: a curva so governa o ganho.
    CHECK(Nota::Subtrair(40.0f, 6.0f) == doctest::Approx(34.0f));
    CHECK(Nota::Subtrair(99.0f, 6.0f) == doctest::Approx(93.0f));
}

TEST_CASE("Nota: chegar a 60 continua custando o mesmo de sempre") {

    // A fase comeca em 40 e aprovar e 60. Com ganho de 0,56 por acerto, sao
    // 36 acertos - e a curva nao pode ter mudado isso.
    float n = 40.0f;
    int acertos = 0;
    while (n < 60.0f && acertos < 1000) {
        n = Nota::Somar(n, 0.56f, 0);
        ++acertos;
    }
    CHECK(acertos == 36);
}

// ------------------------------------------------------------- a nota cheia

TEST_CASE("Nota: so 100 e nota cheia - o teto por dano nao e") {

    // O QUE SEPARA O DOURADO DO QUASE. Quem levou dano demais chega a 99,99 e
    // para ali; se isso acendesse o dourado, o jogador leria nota cheia sem ter
    // tirado, e as duas coisas que o teto cria - a distincao e a comemoracao -
    // morreriam juntas.
    CHECK(Nota::ECheia(Nota::kNotaMaxima));
    CHECK_FALSE(Nota::ECheia(Nota::kTetoComDano));
    CHECK_FALSE(Nota::ECheia(99.9f));
    CHECK_FALSE(Nota::ECheia(60.0f));
    CHECK_FALSE(Nota::ECheia(0.0f));
}

TEST_CASE("Nota: quem sobe limpo chega na cheia; quem leva dano nao chega") {

    // A travessia inteira, do inicio de fase ate onde cada um para. Protege a
    // ligacao entre as tres pecas - curva, teto e limiar do dourado - que
    // separadas ja estao testadas, mas que so juntas respondem "o dourado
    // acende?".
    float limpo = 40.0f, machucado = 40.0f;
    for (int i = 0; i < 400; ++i) {
        limpo     = Nota::Somar(limpo,     0.56f, 0);
        machucado = Nota::Somar(machucado, 0.56f, Nota::kAcertosQuePerdemACheia);
    }

    CHECK(Nota::ECheia(limpo));
    CHECK_FALSE(Nota::ECheia(machucado));
    CHECK(machucado == doctest::Approx(Nota::kTetoComDano));
}
