//
// O rival da turma (Aluno/Rival.h).
//

#include "doctest.h"
#include "../Source/Aluno/Rival.h"

using namespace Rival;

namespace {
    const std::vector<Colega> kTurma = {{"90112", 72.5f}, {"88001", 64.0f}, {"77002", 90.0f}, {"66003", 64.0f}};
}

TEST_CASE("rival: ninguem mais jogou") {
    CHECK(Da({}, 50.0f).tipo == Tipo::Sozinho);
    CHECK(Da({{"1", 0.0f}}, 0.0f).tipo == Tipo::Sozinho);
    CHECK(Frase(Da({}, 0.0f)).find("ranking") != std::string::npos);
}

TEST_CASE("rival: quem nunca jogou mira o ultimo da turma") {
    const Situacao s = Da(kTurma, 0.0f);
    CHECK(s.tipo == Tipo::Estreante);
    CHECK(s.alvo.nota == doctest::Approx(64.0f));
    CHECK(s.alvo.quem == "88001");          // no empate, o primeiro da lista
    CHECK(s.posicaoDoAlvo == 3);            // empatado em 3o com o 66003
}

TEST_CASE("rival: o mais perto acima, e quanto falta") {
    const Situacao s = Da(kTurma, 70.0f);
    CHECK(s.tipo == Tipo::Atras);
    CHECK(s.alvo.quem == "90112");
    CHECK(s.posicaoDoAlvo == 2);
    CHECK(s.distancia == doctest::Approx(2.5f));
    CHECK(Frase(s) == "Rival: 90112 (2\xc2\xba) \xc2\xb7 faltam 2.50");
}

TEST_CASE("rival: empatar nao poe o colega acima") {
    const Situacao s = Da(kTurma, 64.0f);
    CHECK(s.alvo.quem == "90112");
}

TEST_CASE("rival: o lider ve quem vem atras") {
    const Situacao s = Da(kTurma, 95.0f);
    CHECK(s.tipo == Tipo::Lider);
    CHECK(s.alvo.quem == "77002");
    CHECK(s.distancia == doctest::Approx(5.0f));
    CHECK(Frase(s).rfind("1\xc2\xba da turma", 0) == 0);
    CHECK(Frase(Da(kTurma, 90.0f)) == "1\xc2\xba da turma, empatado com 77002");
}

TEST_CASE("rival: quem a nota nova passou") {
    const auto p = Passados(kTurma, 60.0f, 80.0f);
    REQUIRE(p.size() == 3);
    CHECK(p[0].quem == "90112");
    CHECK(Passados(kTurma, 60.0f, 64.0f).empty());      // empatar nao e passar
    CHECK(PosicaoCom(kTurma, 80.0f) == 2);
}

TEST_CASE("rival: a frase do resultado") {
    CHECK(FraseDoResultado(kTurma, 60.0f, 73.0f) == "Voc\xc3\xaa passou 90112 e mais 2: agora \xc3\xa9 o 2\xc2\xba da turma");
    CHECK(FraseDoResultado(kTurma, 70.0f, 73.0f) == "Voc\xc3\xaa passou 90112: agora \xc3\xa9 o 2\xc2\xba da turma");
    CHECK(FraseDoResultado(kTurma, 50.0f, 62.0f) == "Faltaram 2.00 para passar 88001");
    CHECK(FraseDoResultado(kTurma, 30.0f, 40.0f).empty());   // longe demais
    CHECK(FraseDoResultado({}, 0.0f, 80.0f).empty());
    // O visitante nao entra no ranking: le no condicional.
    CHECK(FraseDoResultado(kTurma, 70.0f, 73.0f, false) == "Com essa nota voc\xc3\xaa passaria 90112 e seria o 2\xc2\xba");
}

TEST_CASE("rival: os colegas saem do ranking sem o proprio aluno") {
    std::vector<Ranking::Linha> solo(2);
    solo[0].matricula = "1"; solo[0].nota = 80.0f;
    solo[1].matricula = "2"; solo[1].nota = 70.0f;
    const auto c = DoRanking(solo, "1");
    REQUIRE(c.size() == 1);
    CHECK(c[0].quem == "2");

    std::vector<Ranking::LinhaDeDupla> duplas(2);
    duplas[0].um = "1"; duplas[0].dois = "2"; duplas[0].nota = 80.0f;
    duplas[1].um = "3"; duplas[1].dois = "4"; duplas[1].nota = 70.0f;
    const auto d = DoRanking(duplas, "2", "1");
    REQUIRE(d.size() == 1);
    CHECK(d[0].quem == "3 + 4");
}
