//
// A fita da maquina de Turing: quando uma casa e reescrita.
//

#include "doctest.h"

#include "../Source/Attacks/Fita.h"

#include <vector>

// A PARTE QUE DA PARA TESTAR DE UM COMPORTAMENTO QUE PRECISA DE SDL.
//
// ReescreverBehavior olha a cena, os corpos do chefe e o projetil - nada disso
// entra na suite. O que entra e a decisao, e e nela que estao os dois erros que
// derrubariam a fase: reescrever a casa no quadro em que ela nasce, e nunca
// reescrever porque a cabeca so passa perto.

TEST_CASE("Fita: a casa recem-escrita nao e reescrita pela cabeca que a escreveu") {

    // A CASA NASCE EMBAIXO DA CABECA. Se a idade minima nao segurasse, toda
    // casa viraria "b" no mesmo quadro em que foi escrita e a fita nunca seria
    // vista - seria so mais uma rajada de perseguidores.
    const std::vector<float> cabeca = {500.0f};
    CHECK_FALSE(Fita::DeveReescrever(0.0f, 0.9f, cabeca, 500.0f, 24.0f));
    CHECK_FALSE(Fita::DeveReescrever(0.89f, 0.9f, cabeca, 500.0f, 24.0f));
    CHECK(Fita::DeveReescrever(0.9f, 0.9f, cabeca, 500.0f, 24.0f));
}

TEST_CASE("Fita: so o x da cabeca conta, e o raio e inclusivo") {

    CHECK(Fita::CabecaSobreACasa(500.0f, 524.0f, 24.0f));
    CHECK(Fita::CabecaSobreACasa(524.0f, 500.0f, 24.0f));
    CHECK_FALSE(Fita::CabecaSobreACasa(500.0f, 524.5f, 24.0f));

    // Longe no x, nada - por mais velha que a casa seja.
    const std::vector<float> longe = {300.0f};
    CHECK_FALSE(Fita::DeveReescrever(10.0f, 0.9f, longe, 500.0f, 24.0f));
}

TEST_CASE("Fita: qualquer corpo do chefe e uma cabeca, e sem corpo nada e reescrito") {

    // Dois corpos, so o segundo em cima da casa: basta ele.
    const std::vector<float> dois = {100.0f, 505.0f};
    CHECK(Fita::DeveReescrever(2.0f, 0.9f, dois, 500.0f, 24.0f));

    // Batalha sem chefe em campo (ou ja encerrada): a fita fica como esta.
    const std::vector<float> nenhum;
    CHECK_FALSE(Fita::DeveReescrever(2.0f, 0.9f, nenhum, 500.0f, 24.0f));
}
