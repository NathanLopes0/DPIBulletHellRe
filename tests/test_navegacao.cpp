// Para onde a selecao vai quando alguem aperta uma seta. Camada pura.
//
// O que estes testes protegem e a ligacao entre o que esta DESENHADO na tela e o
// que a seta faz. Ela ja se rompeu uma vez, em silencio: os botoes passaram a
// sair de materias.json e a navegacao continuou com a grade antiga escrita a
// mao, entao a seta andava por uma tela que nao existia mais.

#include "doctest.h"

#include "../Source/Navegacao.h"

#include <algorithm>
#include <vector>

namespace {

    // A grade de hoje, na ordem em que a StageSelect cria os botoes:
    //
    //   col 0        col 1        col 2        col 3        col 4
    //   INF110 (0)   INF213 (1)   INF250 (2)   INF420 (6)   TCC (10)
    //                             INF220 (3)   INF221 (7)
    //                             INF330 (4)   INF321 (8)
    //                             INF331 (5)   INF452 (9)
    Navegacao::Grade GradeDeHoje() {
        return { {0}, {1}, {2,3,4,5}, {6,7,8,9}, {10} };
    }

    constexpr size_t INF110 = 0, INF213 = 1, INF250 = 2, INF220 = 3,
                     INF330 = 4, INF331 = 5, INF420 = 6, INF452 = 9, TCC = 10;
}

// ------------------------------------------------- os dois casos relatados

TEST_CASE("Navegacao: do INF 110 a direita cai no INF 213, e nao pula para o INF 250") {

    // O BUG RELATADO, numero 1. A navegacao antiga achava que a coluna 1 tinha
    // quatro itens e aplicava a regra "vindo de coluna solitaria, cai no meio",
    // que a jogava no indice 2 - o INF 250, do outro lado da tela. O INF 213 e a
    // unica materia que o INF 110 abre; pula-la era pular o proprio caminho.
    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Direita(grade, INF110) == INF213);
}

TEST_CASE("Navegacao: descer no INF 250 percorre a coluna dele, sem passar pelo INF 213") {

    // O BUG RELATADO, numero 2. A navegacao antiga tratava os indices 1 a 4 como
    // uma coluna so, entao descer tres vezes a partir do INF 250 caia no INF 213
    // - que esta noutra coluna - e descer de novo voltava ao INF 250, um ciclo de
    // quatro que nao existia na tela.
    const auto grade = GradeDeHoje();

    size_t onde = INF250;
    onde = Navegacao::Baixo(grade, onde);   CHECK(onde == INF220);
    onde = Navegacao::Baixo(grade, onde);   CHECK(onde == INF330);
    onde = Navegacao::Baixo(grade, onde);   CHECK(onde == INF331);
    onde = Navegacao::Baixo(grade, onde);   CHECK(onde == INF250);   // deu a volta na coluna

    // E o INF 213 nao aparece em nenhum momento dessa descida.
    onde = INF250;
    for (int i = 0; i < 12; ++i) {
        onde = Navegacao::Baixo(grade, onde);
        CAPTURE(i);
        CHECK(onde != INF213);
    }
}

// ------------------------------------------------- a regra, caso a caso

TEST_CASE("Navegacao: cima e baixo nunca saem da coluna") {

    const auto grade = GradeDeHoje();

    for (const auto& coluna : grade) {
        for (const size_t indice : coluna) {
            const size_t cima  = Navegacao::Cima(grade, indice);
            const size_t baixo = Navegacao::Baixo(grade, indice);
            CAPTURE(indice);
            CHECK(std::find(coluna.begin(), coluna.end(), cima)  != coluna.end());
            CHECK(std::find(coluna.begin(), coluna.end(), baixo) != coluna.end());
        }
    }
}

TEST_CASE("Navegacao: coluna de um item so nao se move na vertical") {

    // Senao o botao solitario piscaria para si mesmo a cada toque.
    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Cima(grade, INF110)  == INF110);
    CHECK(Navegacao::Baixo(grade, INF110) == INF110);
    CHECK(Navegacao::Cima(grade, INF213)  == INF213);
    CHECK(Navegacao::Baixo(grade, TCC)    == TCC);
}

TEST_CASE("Navegacao: subir do topo vai para o fim da coluna") {

    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Cima(grade, INF250) == INF331);
    CHECK(Navegacao::Cima(grade, INF220) == INF250);
}

TEST_CASE("Navegacao: a linha e mantida ao andar de lado") {

    // Entre as duas colunas de quatro, a selecao anda na horizontal mesmo - sem
    // saltar de altura, que e o que faria o jogador perder a selecao de vista.
    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Direita(grade, INF250)  == 6);   // linha 0 -> linha 0
    CHECK(Navegacao::Direita(grade, INF220)  == 7);
    CHECK(Navegacao::Direita(grade, INF330)  == 8);
    CHECK(Navegacao::Direita(grade, INF331)  == 9);
    CHECK(Navegacao::Esquerda(grade, INF452) == INF331);
}

TEST_CASE("Navegacao: indo para uma coluna menor, a linha e limitada") {

    // Da linha 3 para uma coluna de um item so: vai para o unico que existe, em
    // vez de para uma linha que nao ha.
    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Direita(grade, INF452) == TCC);
    CHECK(Navegacao::Esquerda(grade, INF331) == INF213);
    CHECK(Navegacao::Esquerda(grade, INF250) == INF213);

    // E o recorte propriamente dito, com uma grade feita para isso: de uma coluna
    // de quatro para uma de duas, a linha 3 nao existe do outro lado e a selecao
    // para na ultima que existe. Sem o recorte isto leria fora do vetor.
    const Navegacao::Grade desigual = { {0,1,2,3}, {4,5} };
    CHECK(Navegacao::Direita(desigual, 3) == 5);   // linha 3 -> ultima, que e 1
    CHECK(Navegacao::Direita(desigual, 2) == 5);   // linha 2 -> ultima tambem
    CHECK(Navegacao::Direita(desigual, 1) == 5);   // linha 1 existe dos dois lados
    CHECK(Navegacao::Direita(desigual, 0) == 4);
}

TEST_CASE("Navegacao: saindo de um botao solitario, cai no meio da coluna") {

    // A linha de quem esta sozinho e sempre 0, mas o botao aparece CENTRADO na
    // tela - mandar para a linha 0 jogaria a selecao para o topo, longe de onde
    // o olho estava.
    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Direita(grade, INF213) == INF220);   // meio da coluna de 4
    CHECK(Navegacao::Esquerda(grade, TCC)   == 7);        // meio da outra de 4
}

TEST_CASE("Navegacao: as laterais dao a volta nos dois sentidos") {

    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Esquerda(grade, INF110) == TCC);     // primeira -> ultima
    CHECK(Navegacao::Direita(grade, TCC)     == INF110);  // ultima -> primeira
}

TEST_CASE("Navegacao: toda seta leva a um botao que existe") {

    // Varre a grade inteira nas quatro direcoes. Nenhum caminho pode terminar
    // fora da grade - um indice invalido viraria leitura fora do vetor de botoes.
    const auto grade = GradeDeHoje();

    std::vector<size_t> todos;
    for (const auto& c : grade) for (const size_t i : c) todos.push_back(i);

    for (const size_t onde : todos) {
        for (const auto mover : {Navegacao::Cima, Navegacao::Baixo,
                                 Navegacao::Esquerda, Navegacao::Direita}) {
            const size_t destino = mover(grade, onde);
            CAPTURE(onde);
            CHECK(std::find(todos.begin(), todos.end(), destino) != todos.end());
        }
    }
}

// ------------------------------------------------- grades estranhas

TEST_CASE("Navegacao: grade vazia nao quebra") {

    const Navegacao::Grade vazia;
    CHECK(Navegacao::Cima(vazia, 0) == 0);
    CHECK(Navegacao::Direita(vazia, 0) == 0);
}

TEST_CASE("Navegacao: indice fora da grade fica onde esta") {

    // Nao inventa um destino: devolver 0 aqui moveria a selecao sozinha e
    // esconderia o defeito que trouxe o indice errado ate aqui.
    const auto grade = GradeDeHoje();
    CHECK(Navegacao::Baixo(grade, 999) == 999);
    CHECK(Navegacao::Esquerda(grade, 999) == 999);
}

TEST_CASE("Navegacao: coluna vazia no meio e pulada, nao vira destino") {

    // Pode acontecer se uma coluna de materias.json ficar sem materia. Parar numa
    // coluna vazia deixaria a selecao em lugar nenhum, sem nada aceso na tela.
    const Navegacao::Grade comBuraco = { {0}, {}, {1,2} };
    CHECK(Navegacao::Direita(comBuraco, 0) == 1);
    CHECK(Navegacao::Esquerda(comBuraco, 1) == 0);
}

TEST_CASE("Navegacao: uma coluna so, de lado nao mexe") {

    const Navegacao::Grade unica = { {0,1,2} };
    CHECK(Navegacao::Direita(unica, 1) == 1);
    CHECK(Navegacao::Esquerda(unica, 1) == 1);
    CHECK(Navegacao::Baixo(unica, 1) == 2);
}

TEST_CASE("Navegacao: a grade antiga tambem funciona, se um dia voltar") {

    // 1,4,4,1 - o layout de antes do INF 110. A regra nao conhece materia
    // nenhuma, entao vale para qualquer forma que materias.json descreva.
    const Navegacao::Grade antiga = { {0}, {1,2,3,4}, {5,6,7,8}, {9} };
    CHECK(Navegacao::Direita(antiga, 0) == 2);    // solitario -> meio
    CHECK(Navegacao::Baixo(antiga, 4) == 1);      // volta na coluna
    CHECK(Navegacao::Direita(antiga, 8) == 9);
}
