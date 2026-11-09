//
// A tela de resultado da batalha: o resumo e o que fazer em seguida.
//

#include "doctest.h"

#include "../Source/Interface/Resultado.h"

using namespace Resultado;

TEST_CASE("Resultado: o menu abre no Tentar de novo e da a volta") {
    CHECK(kOpcoes[0] == Opcao::TentarDeNovo);
    CHECK(Mover(0, 1) == 1);
    CHECK(Mover(1, 1) == 0);
    CHECK(Mover(0, -1) == kQuantas - 1);
    CHECK(std::string(Texto(Opcao::TentarDeNovo)) != Texto(Opcao::Voltar));
}

TEST_CASE("Resultado: recorde novo so para quem tem ficha, e por mais que o arredondamento") {
    Resumo r;
    r.temFicha = true;
    r.recordeAntes = 61.5f;
    r.nota = 72.0f;
    CHECK(NovoRecorde(r));
    r.nota = 61.5f;
    CHECK_FALSE(NovoRecorde(r));     // empatar nao e bater
    r.nota = 61.504f;
    CHECK_FALSE(NovoRecorde(r));     // na tela as duas sao 61.50
    r.temFicha = false;
    r.nota = 99.0f;
    CHECK_FALSE(NovoRecorde(r));     // o visitante nao tem recorde
}

TEST_CASE("Resultado: as linhas dizem a nota, o recorde, as fases limpas e os acertos") {
    Resumo r;
    r.nota = 63.256f;
    r.aprovado = true;
    r.temFicha = true;
    r.recordeAntes = 80.0f;
    r.fasesLimpas = 2;
    r.acertosSofridos = 7;
    const auto l = Linhas(r);
    REQUIRE(l.size() == 4u);   // com ficha, as quatro de sempre
    CHECK(l[0] == "Nota final: 63.26");
    CHECK(l[1] == "Recorde: 80.00");
    CHECK(l[2] == "Fases limpas: 2");
    CHECK(l[3] == "Acertos sofridos: 7");

    CHECK(LinhaEmDestaque(r) == -1);

    r.recordeAntes = 40.0f;
    CHECK(Linhas(r)[1] == "NOVO RECORDE!");
    CHECK(LinhaEmDestaque(r) == 1);   // a dourada e a do recorde

    r.temFicha = false;   // o visitante: sem linha de recorde
    CHECK(Linhas(r).size() == 3u);
    CHECK(LinhaEmDestaque(r) == -1);
    CHECK(std::string(Titulo(true)) == "APROVADE!");
    CHECK(std::string(Titulo(false)) == "REPROVADE");
}

TEST_CASE("Resultado: quem reprovou ve a devolutiva - a fase e, numa linha so dela, o que mais acertou") {
    Resumo r;
    r.nota = 31.0f;
    r.aprovado = false;
    r.temFicha = true;
    r.recordeAntes = 50.0f;
    Devolutiva::Fase f;
    f.ordem = 2;
    f.anuncio = "h(k) = k mod m";
    f.perdido = 18.0f;
    f.acertos = 3;
    r.piorFase = f;
    r.causas = {{"Chave", 12.0f, 2}, {"Bloco", 6.0f, 1}};
    const auto l = Linhas(r);
    REQUIRE(l.size() == 6u);
    CHECK(l[4] == "Onde a nota mais caiu: fase 2, h(k) = k mod m (-18.00)");
    CHECK(l[5].empty());               // a linha dos desenhos
    CHECK(LinhaDasCausas(r) == 5);
    CHECK(LinhaDaParte(r, 0) == -1);   // sozinho, nao ha justica

    // Quem passou nao ve a devolutiva, mesmo que a nota tenha caido em alguma fase.
    r.aprovado = true;
    r.nota = 70.0f;
    CHECK(Linhas(r).size() == 4u);
    CHECK(LinhaDasCausas(r) == -1);

    // Sem ficha, a linha do recorde nao existe, e as outras sobem.
    r.aprovado = false;
    r.temFicha = false;
    CHECK(Linhas(r)[3] == "Onde a nota mais caiu: fase 2, h(k) = k mod m (-18.00)");
    CHECK(LinhaDasCausas(r) == 4);
}

TEST_CASE("Resultado: na dupla, uma linha por aluno com a parte dele - depois da devolutiva") {
    Resumo r;
    r.nota = 75.0f;
    r.aprovado = true;
    r.temFicha = true;
    r.recordeAntes = 80.0f;
    r.partes = {{"89384", 120, 8, 3}, {"Visitante", 95, 2, 5}};
    auto l = Linhas(r);
    REQUIRE(l.size() == 6u);
    CHECK(l[4] == "89384: 120 acertos, 8 coberturas, 3 sofridos");
    CHECK(l[5] == "Visitante: 95 acertos, 2 coberturas, 5 sofridos");
    CHECK(LinhaDaParte(r, 0) == 4);
    CHECK(LinhaDaParte(r, 1) == 5);
    CHECK(LinhaDaParte(r, 2) == -1);

    // A dupla que reprova ve tudo: e o maior resumo que ha.
    r.aprovado = false;
    r.nota = 30.0f;
    Devolutiva::Fase f;
    f.ordem = 1;
    f.perdido = 6.0f;
    f.acertos = 1;
    r.piorFase = f;
    r.causas = {{"Bloco", 6.0f, 1}};
    l = Linhas(r);
    CHECK(l.size() == static_cast<size_t>(kLinhasNoMaximo));
    CHECK(LinhaDasCausas(r) == 5);
    CHECK(LinhaDaParte(r, 0) == 6);
    CHECK(LinhaDaParte(r, 1) == 7);
}

TEST_CASE("Resultado: quem se formou ve FORMADE! e que ganhou o chapeu de formatura") {
    Resumo r;
    r.nota = 72.0f;
    r.aprovado = true;
    r.temFicha = true;
    r.recordeAntes = 50.0f;
    CHECK_FALSE(Formou(r));
    CHECK(std::string(Titulo(r)) == "APROVADE!");
    CHECK(LinhaDaFormatura(r) == -1);

    r.formados = {"89384"};
    r.formaturaGravada = true;
    CHECK(Formou(r));
    CHECK(std::string(Titulo(r)) == "FORMADE!");
    auto l = Linhas(r);
    REQUIRE(l.size() == 5u);
    CHECK(l[4] == "Voc\xc3\xaa ganhou o chap\xc3\xa9u de formatura, para sempre.");
    CHECK(LinhaDaFormatura(r) == 4);
    CHECK(LinhaEmDestaque(r) == 1);   // o recorde novo continua dourado

    // O visitante se forma so enquanto a sessao dura: nada de "para sempre".
    r.temFicha = false;
    r.formaturaGravada = false;
    l = Linhas(r);
    REQUIRE(l.size() == 4u);
    CHECK(l[3] == "Voc\xc3\xaa ganhou o chap\xc3\xa9u de formatura.");
    CHECK(LinhaDaFormatura(r) == 3);

    // Quem reprovou nao se forma, diga a lista o que disser.
    r.aprovado = false;
    CHECK_FALSE(Formou(r));
    CHECK(std::string(Titulo(r)) == "REPROVADE");
    CHECK(LinhaDaFormatura(r) == -1);
}

TEST_CASE("Resultado: na dupla, a formatura diz quem se formou, antes da parte de cada um") {
    Resumo r;
    r.nota = 75.0f;
    r.aprovado = true;
    r.temFicha = true;
    r.recordeAntes = 80.0f;
    r.partes = {{"89384", 120, 8, 3}, {"Aluno 2", 95, 2, 5}};
    r.formados = {"89384", "Aluno 2"};
    auto l = Linhas(r);
    REQUIRE(l.size() == 7u);
    CHECK(l[4] == "A dupla ganhou o chap\xc3\xa9u de formatura.");
    CHECK(l[5] == "89384: 120 acertos, 8 coberturas, 3 sofridos");
    CHECK(LinhaDaFormatura(r) == 4);
    CHECK(LinhaDaParte(r, 0) == 5);
    CHECK(LinhaDaParte(r, 1) == 6);
    CHECK(l.size() <= static_cast<size_t>(kLinhasNoMaximo));

    // So um se formou agora: o outro ja tinha o capelo.
    r.formados = {"89384"};
    r.formaturaGravada = true;
    CHECK(Linhas(r)[4] == "89384 ganhou o chap\xc3\xa9u de formatura, para sempre.");
    r.formados = {"Aluno 2"};
    r.formaturaGravada = false;
    CHECK(Linhas(r)[4] == "Aluno 2 ganhou o chap\xc3\xa9u de formatura.");
}

TEST_CASE("Resultado: so o gabinete volta sozinho, e so depois da espera") {
    CHECK_FALSE(VoltaSozinha(true, 0.0f));
    CHECK_FALSE(VoltaSozinha(true, kEsperaNoGabinete - 0.1f));
    CHECK(VoltaSozinha(true, kEsperaNoGabinete));
    CHECK_FALSE(VoltaSozinha(false, 10.0f * kEsperaNoGabinete));   // no computador, nunca
    // A espera tem de passar da trava do menu.
    CHECK(kEsperaNoGabinete > kTravaDoMenu);
}
