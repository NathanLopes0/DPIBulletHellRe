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
    REQUIRE(l.size() == static_cast<size_t>(kLinhasNoMaximo));   // com ficha, todas
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

TEST_CASE("Resultado: so o gabinete volta sozinho, e so depois da espera") {
    CHECK_FALSE(VoltaSozinha(true, 0.0f));
    CHECK_FALSE(VoltaSozinha(true, kEsperaNoGabinete - 0.1f));
    CHECK(VoltaSozinha(true, kEsperaNoGabinete));
    CHECK_FALSE(VoltaSozinha(false, 10.0f * kEsperaNoGabinete));   // no computador, nunca
    // A espera tem de passar da trava do menu: senao o gabinete sairia antes
    // de alguem poder escolher.
    CHECK(kEsperaNoGabinete > kTravaDoMenu);
}
