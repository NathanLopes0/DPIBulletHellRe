// Leitura de formas de trajetoria a partir de texto JSON.
// Camada pura: nao abre arquivo, nao escreve log, nao sobe SDL.

#include "../../doctest.h"
#include "../../../Source/Attacks/Engine/PathShapes.h"

using PathShapes::LerFormas;

TEST_CASE("Formas: le uma lista de pontos") {
    const auto r = LerFormas(R"({ "Serpente": { "pontos": [[60,-40],[120,-50],[160,-20]] } })");
    CHECK(r.problemas.empty());
    REQUIRE(r.formas.count("Serpente") == 1);
    REQUIRE(r.formas.at("Serpente").size() == 3);
    CHECK(r.formas.at("Serpente")[0].x == doctest::Approx(60.f));
    CHECK(r.formas.at("Serpente")[0].y == doctest::Approx(-40.f));
}

TEST_CASE("Formas: le um gerador, reaproveitando as formas em codigo") {
    const auto r = LerFormas(R"({ "Laco": { "gerador": {"tipo":"Loop","a":70,"b":500,"n":12} } })");
    CHECK(r.problemas.empty());
    REQUIRE(r.formas.count("Laco") == 1);
    // Loop gera 'n' pontos do circulo mais o ponto de fuga.
    CHECK(r.formas.at("Laco").size() == 13);
}

TEST_CASE("Formas: varias no mesmo arquivo") {
    const auto r = LerFormas(R"({
        "A": { "gerador": {"tipo":"Reta","a":800} },
        "B": { "pontos": [[10,0],[20,5]] }
    })");
    CHECK(r.problemas.empty());
    CHECK(r.formas.size() == 2);
}

// --- o arquivo nunca e descartado inteiro por causa de uma entrada errada ---

TEST_CASE("Formas: entrada invalida e ignorada, as boas continuam valendo") {
    const auto r = LerFormas(R"({
        "Boa":  { "pontos": [[10,0]] },
        "Ruim": { "gerador": {"tipo":"Espiral"} }
    })");
    CHECK(r.formas.count("Boa") == 1);
    CHECK(r.formas.count("Ruim") == 0);
    REQUIRE(r.problemas.size() == 1);
    // O problema diz QUAL forma e O QUE fazer - nao some em silencio.
    CHECK(r.problemas[0].find("Ruim") != std::string::npos);
    CHECK(r.problemas[0].find("Espiral") != std::string::npos);
}

TEST_CASE("Formas: ponto malformado e relatado com o nome da forma") {
    const auto r = LerFormas(R"({ "X": { "pontos": [[10,0],[20]] } })");
    CHECK(r.formas.empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("\"X\"") != std::string::npos);
}

TEST_CASE("Formas: falta pontos e gerador") {
    const auto r = LerFormas(R"({ "Y": { "cor": "azul" } })");
    CHECK(r.formas.empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("pontos") != std::string::npos);
}

TEST_CASE("Formas: lista de pontos vazia e recusada") {
    const auto r = LerFormas(R"({ "Z": { "pontos": [] } })");
    CHECK(r.formas.empty());
    CHECK(r.problemas.size() == 1);
}

TEST_CASE("Formas: JSON invalido nao derruba nada") {
    const auto r = LerFormas("{ isto nao e json");
    CHECK(r.formas.empty());
    REQUIRE(r.problemas.size() == 1);
    CHECK(r.problemas[0].find("JSON") != std::string::npos);
}

TEST_CASE("Formas: arquivo vazio nao e erro, so nao traz formas") {
    const auto r = LerFormas("{}");
    CHECK(r.formas.empty());
    CHECK(r.problemas.empty());
}

// ---------------------------------------------------------------------------
// Comentarios
// ---------------------------------------------------------------------------

TEST_CASE("Formas: o arquivo aceita comentario") {
    // O JSON padrao nao tem comentario, e sem ele migrar codigo comentado para
    // dados jogaria a explicacao fora. Ver Source/Data/JsonDeDados.h.
    const auto r = LerFormas(R"({
        // Doze pontos: mais que isso e o projetil gasta quadro "chegando" em
        // cada waypoint e a curva fica truncada.
        "Laco": { "gerador": {"tipo":"Loop","a":70,"b":500,"n":12} }   // lento de proposito
        /* o bloco tambem vale */
    })");
    CHECK(r.problemas.empty());
    CHECK(r.formas.count("Laco") == 1);
}

// ---------------------------------------------------------------------------
// Arvore binaria
//
// Os testes abaixo conferem PROPRIEDADES, e nao numeros decorados: que as irmas
// compartilham prefixo, que cada no interno fica no meio dos filhos, que a copa
// e simetrica. Sao essas propriedades que fazem a coisa parecer uma arvore em
// jogo - um teste que so repetisse as coordenadas passaria igual com a arvore
// torta.
// ---------------------------------------------------------------------------

TEST_CASE("Arvore: o numero de folhas dobra a cada divisao") {
    CHECK(PathShapes::FolhasDeArvore(0) == 1);
    CHECK(PathShapes::FolhasDeArvore(1) == 2);
    CHECK(PathShapes::FolhasDeArvore(2) == 4);
    CHECK(PathShapes::FolhasDeArvore(3) == 8);
    CHECK(PathShapes::FolhasDeArvore(4) == 16);
}

TEST_CASE("Arvore: numero de divisoes sem sentido devolve zero folhas") {
    // Zero e o sinal de "nao da": quem chama trata, em vez de receber uma arvore
    // de mil projeteis por um erro de digitacao.
    CHECK(PathShapes::FolhasDeArvore(-1) == 0);
    CHECK(PathShapes::FolhasDeArvore(11) == 0);
}

TEST_CASE("Arvore: o caminho tem um ponto por divisao, mais tronco e saida") {
    for (int div = 1; div <= 4; ++div) {
        CAPTURE(div);
        const auto p = PathShapes::ArvoreBinaria(div, 0);
        CHECK(p.size() == static_cast<size_t>(div) + 2);
    }
}

TEST_CASE("Arvore: folha fora da faixa devolve vazio, nao um caminho errado") {
    CHECK(PathShapes::ArvoreBinaria(3, -1).empty());
    CHECK(PathShapes::ArvoreBinaria(3, 8).empty());
    CHECK_FALSE(PathShapes::ArvoreBinaria(3, 7).empty());
}

TEST_CASE("Arvore: todas as folhas saem do MESMO tronco") {
    // E o que faz os projeteis saírem sobrepostos e descerem como um no so.
    const int div = 3;
    const auto primeira = PathShapes::ArvoreBinaria(div, 0);
    for (int f = 1; f < PathShapes::FolhasDeArvore(div); ++f) {
        CAPTURE(f);
        const auto p = PathShapes::ArvoreBinaria(div, f);
        CHECK(p[0].x == doctest::Approx(primeira[0].x));
        CHECK(p[0].y == doctest::Approx(primeira[0].y));
        CHECK(p[0].y == doctest::Approx(0.0f));
    }
}

TEST_CASE("Arvore: irmas compartilham o prefixo ate o pai, e divergem depois") {
    // A PROPRIEDADE CENTRAL. Duas folhas que so se separam na ultima divisao tem
    // de ter todos os pontos iguais ate ali; se divergissem antes, o jogador veria
    // dois tiros paralelos em vez de um no que abre.
    const int div = 3;
    for (int par = 0; par < 4; ++par) {
        const int a = par * 2, b = par * 2 + 1;
        CAPTURE(a); CAPTURE(b);
        const auto pa = PathShapes::ArvoreBinaria(div, a);
        const auto pb = PathShapes::ArvoreBinaria(div, b);

        // Pontos 0..div-1 iguais (tronco e divisoes anteriores a ultima).
        for (int i = 0; i < div; ++i) {
            CAPTURE(i);
            CHECK(pa[i].x == doctest::Approx(pb[i].x));
            CHECK(pa[i].y == doctest::Approx(pb[i].y));
        }
        // A ultima divisao separa.
        CHECK(pa[div].y != doctest::Approx(pb[div].y));
    }
}

TEST_CASE("Arvore: folhas de avos diferentes divergem MAIS CEDO") {
    // Complemento do teste acima: a profundidade em que duas folhas se separam
    // tem de ser a profundidade do ancestral comum delas. E o que da a forma de
    // arvore em vez de um leque.
    const int div = 3;
    const auto f0 = PathShapes::ArvoreBinaria(div, 0);
    const auto f7 = PathShapes::ArvoreBinaria(div, 7);
    // 0 e 7 so compartilham a raiz: ja divergem na PRIMEIRA divisao.
    CHECK(f0[0].y == doctest::Approx(f7[0].y));
    CHECK(f0[1].y != doctest::Approx(f7[1].y));

    const auto f0b = PathShapes::ArvoreBinaria(div, 0);
    const auto f3 = PathShapes::ArvoreBinaria(div, 3);
    // 0 e 3 compartilham o no do nivel 1, e divergem no nivel 2.
    CHECK(f0b[1].y == doctest::Approx(f3[1].y));
    CHECK(f0b[2].y != doctest::Approx(f3[2].y));
}

TEST_CASE("Arvore: cada no interno fica no PONTO MEDIO dos dois filhos") {
    // E isto que alinha os ramos. Sem isso, os ramos sairiam tortos e a copa
    // nao pareceria uma arvore, mesmo com as folhas nos lugares certos.
    const int div = 3;
    for (int f = 0; f < 8; f += 2) {
        CAPTURE(f);
        const auto a = PathShapes::ArvoreBinaria(div, f);
        const auto b = PathShapes::ArvoreBinaria(div, f + 1);
        // o pai das duas e o ponto de indice div-1
        const float meio = (a[div].y + b[div].y) / 2.0f;
        CHECK(a[div - 1].y == doctest::Approx(meio));
    }
}

TEST_CASE("Arvore: a copa e simetrica em torno do tronco") {
    const int div = 3;
    const int n = PathShapes::FolhasDeArvore(div);
    for (int f = 0; f < n; ++f) {
        CAPTURE(f);
        const auto esq = PathShapes::ArvoreBinaria(div, f);
        const auto dir = PathShapes::ArvoreBinaria(div, n - 1 - f);
        CHECK(esq.back().y == doctest::Approx(-dir.back().y));
    }
}

TEST_CASE("Arvore: a largura da copa e espacamento x (folhas - 1)") {
    const int div = 3;
    const float esp = 60.0f;
    const auto primeira = PathShapes::ArvoreBinaria(div, 0, 120.f, 90.f, esp);
    const auto ultima   = PathShapes::ArvoreBinaria(div, 7, 120.f, 90.f, esp);
    CHECK(ultima.back().y - primeira.back().y == doctest::Approx(esp * 7.0f));
}

TEST_CASE("Arvore: folhas vizinhas ficam a um espacamento uma da outra") {
    const int div = 3;
    const float esp = 60.0f;
    for (int f = 0; f + 1 < 8; ++f) {
        CAPTURE(f);
        const auto a = PathShapes::ArvoreBinaria(div, f, 120.f, 90.f, esp);
        const auto b = PathShapes::ArvoreBinaria(div, f + 1, 120.f, 90.f, esp);
        CHECK(b.back().y - a.back().y == doctest::Approx(esp));
    }
}

TEST_CASE("Arvore: o avanco em x e tronco, passo por divisao, e a saida no fim") {
    const auto p = PathShapes::ArvoreBinaria(3, 0, 120.f, 90.f, 60.f, 560.f);
    CHECK(p[0].x == doctest::Approx(120.f));
    CHECK(p[1].x == doctest::Approx(210.f));
    CHECK(p[2].x == doctest::Approx(300.f));
    CHECK(p[3].x == doctest::Approx(390.f));
    CHECK(p[4].x == doctest::Approx(950.f));
}

TEST_CASE("Arvore: zero divisoes e uma reta, nao um erro") {
    // O caso degenerado tem de ser util: uma arvore que nao se divide e um tiro
    // reto, e nao uma lista vazia que faria o projetil voar sem caminho.
    const auto p = PathShapes::ArvoreBinaria(0, 0, 120.f, 90.f, 60.f, 560.f);
    REQUIRE(p.size() == 2);
    CHECK(p[0].y == doctest::Approx(0.0f));
    CHECK(p[1].y == doctest::Approx(0.0f));
}

// ------------------------------------ familia escrita a mao ("caminhos")

TEST_CASE("Formas: \"caminhos\" declara uma familia, um caminho por projetil") {

    // POR QUE ESTA PRIMITIVA EXISTE. "arvore" ja fazia familias, mas so arvores
    // binarias. Qualquer outro conjunto de caminhos que so faca sentido JUNTO
    // tinha de virar N formas simples mais N regras por indice - o par de
    // listas que tem de bater, que e o que a familia foi inventada para evitar.
    const auto lidas = PathShapes::LerFormas(R"({
      "Escada": { "caminhos": [ [[10, 0]], [[20, 0]], [[30, 0]] ] }
    })");

    CHECK(lidas.problemas.empty());

    // Familia NAO entra no mapa de formas simples: sao coisas diferentes, e
    // pedir uma pela outra tem de falhar em vez de devolver meia resposta.
    CHECK(lidas.formas.count("Escada") == 0);

    REQUIRE(lidas.familias.count("Escada") == 1);
    const auto& fam = lidas.familias.at("Escada");
    REQUIRE(fam.size() == 3);
    CHECK(fam[0][0].x == doctest::Approx(10.0f));
    CHECK(fam[1][0].x == doctest::Approx(20.0f));
    CHECK(fam[2][0].x == doctest::Approx(30.0f));
}

TEST_CASE("Formas: a ordem dos caminhos E o contrato") {

    // O indice do projetil na rajada e o indice na familia (PathShapes.h). Se a
    // leitura reordenasse, a arvore do Salles cresceria torta e a pilha do
    // Vladimir desempilharia fora de ordem - sem erro nenhum, so errado.
    const auto lidas = PathShapes::LerFormas(R"({
      "Fundo": { "caminhos": [ [[0, 1]], [[0, 2]], [[0, 3]], [[0, 4]] ] }
    })");

    REQUIRE(lidas.familias.count("Fundo") == 1);
    const auto& fam = lidas.familias.at("Fundo");
    REQUIRE(fam.size() == 4);
    for (size_t i = 0; i < fam.size(); ++i) {
        CAPTURE(i);
        CHECK(fam[i][0].y == doctest::Approx(static_cast<float>(i) + 1.0f));
    }
}

TEST_CASE("Formas: um caminho de varios pontos continua sendo um caminho") {

    const auto lidas = PathShapes::LerFormas(R"({
      "Gancho": { "caminhos": [ [[10, 0], [20, 30], [90, 30]], [[5, 0]] ] }
    })");

    REQUIRE(lidas.familias.count("Gancho") == 1);
    const auto& fam = lidas.familias.at("Gancho");
    REQUIRE(fam.size() == 2);
    CHECK(fam[0].size() == 3);
    CHECK(fam[1].size() == 1);
    CHECK(fam[0][2].x == doctest::Approx(90.0f));
}

TEST_CASE("Formas: \"caminhos\" vazio ou mal formado e recusado, com a frase certa") {

    // O ARQUIVO INTEIRO NUNCA E DESCARTADO por causa de uma entrada errada -
    // mesma filosofia das formas simples. A entrada ruim fica de fora e vira
    // uma frase; as outras continuam valendo.
    const auto lidas = PathShapes::LerFormas(R"({
      "Vazia":  { "caminhos": [] },
      "Torta":  { "caminhos": [ [[1, 2]], "nao sou uma lista" ] },
      "Ponto3": { "caminhos": [ [[1, 2, 3]] ] },
      "Boa":    { "caminhos": [ [[7, 7]] ] }
    })");

    CHECK(lidas.familias.count("Vazia") == 0);
    CHECK(lidas.familias.count("Torta") == 0);
    CHECK(lidas.familias.count("Ponto3") == 0);
    CHECK(lidas.familias.count("Boa") == 1);
    CHECK(lidas.problemas.size() == 3);

    // A frase diz QUAL caminho da familia estava torto, e nao so o nome da
    // forma: numa familia de doze, "a forma Pilha esta errada" nao ajuda.
    bool citaOIndice = false;
    for (const auto& p : lidas.problemas) {
        if (p.find("caminho 1") != std::string::npos) citaOIndice = true;
    }
    CHECK(citaOIndice);
}

TEST_CASE("Formas: a entrada sem pontos, gerador, arvore nem caminhos se explica") {

    const auto lidas = PathShapes::LerFormas(R"({ "Nada": { "cor": "azul" } })");

    REQUIRE(lidas.problemas.size() == 1);
    CHECK(lidas.problemas[0].find("caminhos") != std::string::npos);
}
