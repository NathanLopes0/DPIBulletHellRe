// A devolutiva da prova e a justica da dupla.

#include "doctest.h"

#include "../Source/Interface/Devolutiva.h"

using namespace Devolutiva;

TEST_CASE("Devolutiva: a fase em que a nota mais caiu, e o que mais acertou") {
    Caderno c;
    CHECK_FALSE(c.PiorFase().has_value());
    CHECK(c.Causas().empty());

    c.ComecarFase("push(x)");
    c.Perder("Bloco", 6.0f);
    c.ComecarFase("h(k) = k mod m");
    c.Perder("Chave", 6.0f);
    c.Perder("Chave", 6.0f);
    c.Perder(kEsbarrao, 6.0f);
    c.ComecarFase("arvore");
    c.Perder("Bloco", 3.0f);

    const auto pior = c.PiorFase();
    REQUIRE(pior.has_value());
    CHECK(pior->ordem == 2);
    CHECK(pior->anuncio == "h(k) = k mod m");
    CHECK(pior->perdido == doctest::Approx(18.0f));
    CHECK(pior->acertos == 3);

    // A chave custou 12, o bloco 9, o esbarrao 6.
    const auto causas = c.Causas();
    REQUIRE(causas.size() == 3u);
    CHECK(causas[0].causa == "Chave");
    CHECK(causas[0].acertos == 2);
    CHECK(causas[1].causa == "Bloco");
    CHECK(causas[1].perdido == doctest::Approx(9.0f));
    CHECK(causas[2].causa == kEsbarrao);

    // Perda nenhuma nao conta, nem como acerto.
    c.Perder("Bloco", 0.0f);
    CHECK(c.Causas()[1].acertos == 2);
}

TEST_CASE("Devolutiva: no empate fica a primeira fase, e a prova final tem o nome dela") {
    Caderno c;
    c.ComecarFase("um");
    c.Perder("A", 5.0f);
    c.ComecarFase("dois");
    c.Perder("A", 5.0f);
    CHECK(c.PiorFase()->ordem == 1);

    c.ComecarFase("BOCA", true);
    c.Perder("B", 20.0f);
    const auto pior = c.PiorFase();
    REQUIRE(pior.has_value());
    CHECK(pior->final);
    CHECK(LinhaDaFase(*pior) == "Onde a nota mais caiu: prova final (-20.00)");

    // A perda antes de anunciar fase nenhuma e da primeira.
    Caderno d;
    d.Perder("A", 3.0f);
    REQUIRE(d.PiorFase().has_value());
    CHECK(d.PiorFase()->ordem == 1);
}

TEST_CASE("Devolutiva: as linhas, com duas casas e o plural certo") {
    Fase f;
    f.ordem = 2;
    f.anuncio = "h(k) = k mod m";
    f.perdido = 18.0f;
    f.acertos = 3;
    CHECK(LinhaDaFase(f) == "Onde a nota mais caiu: fase 2, h(k) = k mod m (-18.00)");
    f.anuncio.clear();
    f.perdido = 6.0f;
    CHECK(LinhaDaFase(f) == "Onde a nota mais caiu: fase 2 (-6.00)");

    // O acento conta como uma letra: 58 letras (62 bytes) cabem...
    f.anuncio = "\xc3\xa1rvore bin\xc3\xa1ria \xc3\xa9 j\xc3\xa1";
    CHECK(LinhaDaFase(f) == "Onde a nota mais caiu: fase 2, \xc3\xa1rvore bin\xc3\xa1ria \xc3\xa9 j\xc3\xa1 (-6.00)");
    // ...e o nome que nao cabe sai, e a linha fica com o numero da fase.
    f.anuncio = "\xc3\xa1rvore bin\xc3\xa1ria de busca";
    CHECK(LinhaDaFase(f) == "Onde a nota mais caiu: fase 2 (-6.00)");

    Parte p;
    p.quem = "89384";
    p.acertos = 120;
    p.coberturas = 1;
    p.sofridos = 0;
    CHECK(LinhaDaParte(p) == "89384: 120 acertos, 1 cobertura, 0 sofridos");
}

// ---- A justica que reconhece papeis ----

TEST_CASE("Devolutiva: na dupla, cada aluno fica com o papel de que e dono, e eles nunca se repetem") {
    // Quem ataca chama a mira e apanha mais; o outro desvia mais.
    std::vector<Parte> partes = {{"A", 120, 0, 5}, {"B", 40, 0, 1}};
    auto t = Titulos(partes);
    REQUIRE(t.size() == 2u);
    CHECK(t[0] == "quem mais perguntou");
    CHECK(t[1] == "quem mais desviou");

    // Cobrir vem primeiro: e o gesto de dupla.
    partes = {{"A", 120, 0, 5}, {"B", 40, 3, 6}};
    t = Titulos(partes);
    CHECK(t[0] == "quem mais perguntou");
    CHECK(t[1] == "quem mais cobriu");

    // Quem e dono de mais de um fica com o primeiro, e o outro continua livre para o parceiro.
    partes = {{"A", 120, 2, 1}, {"B", 40, 0, 1}};
    t = Titulos(partes);
    CHECK(t[0] == "quem mais cobriu");
    CHECK(t[1] != t[0]);
}

TEST_CASE("Devolutiva: quem nao e dono de nenhum papel ganha o que fez, sem comparacao") {
    // A e melhor em tudo: B nao fica com um titulo de "quem menos".
    std::vector<Parte> partes = {{"A", 120, 2, 1}, {"B", 12, 0, 4}};
    auto t = Titulos(partes);
    CHECK(t[0] == "quem mais cobriu");
    CHECK(t[1] == "fez 12 perguntas");

    partes = {{"A", 1, 0, 0}, {"B", 1, 0, 0}};
    t = Titulos(partes);
    CHECK(t[0] == "fez 1 pergunta");
    CHECK(t[1] == "fez 1 pergunta");

    // Quase no empate ninguem e "quem mais": 39 a 38 e parecido demais.
    partes = {{"A", 39, 0, 0}, {"B", 38, 0, 0}};
    t = Titulos(partes);
    CHECK(t[0] == "fez 39 perguntas");
    CHECK(t[1] == "fez 38 perguntas");
    partes = {{"A", 44, 0, 0}, {"B", 38, 0, 0}};
    CHECK(Titulos(partes)[0] == "quem mais perguntou");

    partes = {{"A", 0, 1, 2}, {"B", 0, 1, 2}};
    t = Titulos(partes);
    CHECK(t[0] == "cobriu 1 vez");

    partes = {{"A", 0, 0, 0}, {"B", 0, 0, 0}};
    t = Titulos(partes);
    CHECK(t[0] == "esteve junto at\xc3\xa9 o fim");

    // Fora da dupla nao ha titulo.
    CHECK(Titulos({{"A", 10, 0, 0}}).empty());
    CHECK(Titulos({}).empty());
}

TEST_CASE("Devolutiva: os numeros da parte, sem o nome, para ir embaixo do titulo") {
    Parte p;
    p.quem = "89384";
    p.acertos = 1;
    p.coberturas = 2;
    p.sofridos = 1;
    CHECK(NumerosDaParte(p) == "1 acerto, 2 coberturas, 1 sofrido");
    CHECK(LinhaDaParte(p) == "89384: " + NumerosDaParte(p));
}
