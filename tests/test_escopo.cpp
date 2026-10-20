//
// O escopo que cresce: a aritmetica da fase 1 da Maria Lucia.
//

#include "doctest.h"

#include "../Source/Attacks/EscopoQueCresce.h"

#include <vector>

// A PARTE QUE DA PARA TESTAR DE UM CHEFE QUE PRECISA DE SDL.
//
// MariaLucia::CustomizeAttackParams nao entra na suite - Boss arrasta SDL
// inteiro atras. O que da para separar e a CONTA: quanto somar nesta rajada e
// qual e a rajada seguinte. E e onde moram os erros de verdade (o modulo que
// nunca reinicia, o passo que cresce sem teto), e nao no "params.x += y".

TEST_CASE("Escopo: a primeira rajada da volta nao cresce nada") {

    // O ZERO PRECISA SER ZERO, e nao "quase". A rajada 0 E o tamanho que
    // fases.json pediu: se ela ja viesse com acrescimo, o numero do arquivo
    // deixaria de ser o escopo de partida e viraria um numero sem significado.
    const auto a = Escopo::DaRajada(0, 2, 10.0f);
    CHECK(a.projeteis == 0);
    CHECK(a.leque == doctest::Approx(0.0f));
}

TEST_CASE("Escopo: cada rajada soma um passo, nas duas dimensoes") {

    for (int sprint = 0; sprint < 5; ++sprint) {
        CAPTURE(sprint);
        const auto a = Escopo::DaRajada(sprint, 2, 10.0f);
        CHECK(a.projeteis == sprint * 2);
        CHECK(a.leque == doctest::Approx(static_cast<float>(sprint) * 10.0f));
    }
}

TEST_CASE("Escopo: nem na maior rajada dois vizinhos se encostam") {

    // O CRITERIO E EM PIXELS, E NAO EM GRAUS - e a primeira versao deste teste
    // errou isso. Eu tinha escrito "o espacamento em graus nao pode apertar
    // mais que 20%", e ele reprovou numeros que na tela estao folgados: com o
    // leque crescendo 10 graus por rajada, o espacamento cai de 9,0 para 6,8
    // graus, que e 24% - mas 6,8 graus a 400px de distancia ainda sao 47
    // pixels, e o sprite tem 32.
    //
    // A regra que importa e a que o Vitor ensinou do jeito dificil: com as
    // tarefas a 0,05s de intervalo elas nasciam a 15px uma da outra e a fila
    // virava uma BARRA SOLIDA verde - o jogador nao via tarefas, via uma
    // parede. Entao o que nao pode acontecer e os vizinhos se tocarem.
    const int   baseProjeteis = 6;
    const float baseLeque     = 45.0f;

    // Onde o jogador ENCONTRA a rajada, e nao onde ela nasce: perto do chefe
    // qualquer leque e apertado, e isso nunca importou.
    const float kDistancia = 400.0f;
    const float kSprite    = 32.0f;
    const float kPi        = 3.14159265f;

    float anterior = 0.0f;
    for (int sprint = 0; sprint < 4; ++sprint) {
        const auto  a = Escopo::DaRajada(sprint, 2, 10.0f);
        const int   n = baseProjeteis + a.projeteis;
        const float leque = baseLeque + a.leque;
        const float emPixels =
            kDistancia * (leque / static_cast<float>(n - 1)) * kPi / 180.0f;

        CAPTURE(sprint);
        CAPTURE(n);
        CAPTURE(leque);
        CAPTURE(emPixels);

        CHECK_MESSAGE(emPixels >= kSprite,
            "a rajada " << sprint << " tem " << n << " projeteis em " << leque
            << " graus: vizinhos a " << emPixels << "px, e o sprite tem " << kSprite);

        // E NAO PODE AFROUXAR: escopo que cresce tem de apertar, senao a fase 1
        // fica mais FACIL a cada rajada e o chefe diz o contrario do que quer.
        if (sprint > 0) CHECK(emPixels <= anterior);
        anterior = emPixels;
    }
}

TEST_CASE("Escopo: a volta reinicia na entrega, e nao cresce para sempre") {

    // SEM O REINICIO A FASE VIRA UMA PAREDE: dezessete segundos a uma rajada
    // por segundo terminariam em rajadas de quarenta projeteis.
    int sprint = 0;
    std::vector<int> vistos;
    for (int i = 0; i < 10; ++i) {
        vistos.push_back(sprint);
        sprint = Escopo::SprintSeguinte(sprint, 4);
    }

    const std::vector<int> esperado = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1};
    CHECK(vistos == esperado);
}

TEST_CASE("Escopo: uma entrega de um sprint so nunca cresce") {

    // Caso de borda real: quem configurar 1 esta dizendo "entrega a cada
    // rajada", e aí nunca ha o que acumular. Tem de ficar em zero, e nao
    // alternar entre 0 e 1.
    CHECK(Escopo::SprintSeguinte(0, 1) == 0);
    CHECK(Escopo::SprintSeguinte(5, 1) == 0);
}

TEST_CASE("Escopo: um numero de sprints invalido nao divide por zero nem foge") {

    // Nao ha leitor de arquivo por tras disto hoje - os passos sao constantes
    // em MariaLucia.h. Mas uma funcao pura que explode com zero e uma bomba
    // esperando o dia em que alguem mover isso para JSON.
    CHECK(Escopo::SprintSeguinte(3, 0) == 0);
    CHECK(Escopo::SprintSeguinte(3, -7) == 0);
}

TEST_CASE("Escopo: um sprint negativo nao encolhe a rajada") {

    // Encolher abaixo do que fases.json pediu seria pior do que nao crescer:
    // o arquivo deixaria de ser o piso.
    const auto a = Escopo::DaRajada(-3, 2, 10.0f);
    CHECK(a.projeteis == 0);
    CHECK(a.leque == doctest::Approx(0.0f));
}
