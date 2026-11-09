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
