//
// A prova da semana.
//

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Aluno/ProvaDaSemana.h"

using namespace ProvaDaSemana;

namespace {
    Materias::Lista Curso() {
        return Materias::LerMaterias(R"({ "materias": [
            { "codigo": "A1", "nome": "INF 1", "professor": "Ana", "chefe": "ana", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "B1", "nome": "INF 2", "professor": "Bia", "chefe": "bia", "coluna": 1,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "V1", "nome": "INF 3", "professor": "Vera", "coluna": 1,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "T1", "nome": "TCC", "professor": "Tom", "chefe": "tom", "coluna": 2, "orientacao": true,
              "desbloqueio": { "tipo": "sempre" } }
        ] })");
    }
}

TEST_CASE("ProvaDaSemana: a semana comeca na segunda, dura sete dias e troca no domingo a meia-noite") {
    // 2026-11-09 e uma segunda-feira.
    const Semana segunda = Da("2026-11-09 00:00:01");
    REQUIRE(segunda.Valida());
    CHECK(segunda.segunda == "2026-11-09");
    CHECK(segunda.faltam == 7);
    CHECK(DiaDaSegunda(segunda) == "09/11");
    const Semana quarta = Da("2026-11-11 15:30:00");
    CHECK(quarta.indice == segunda.indice);
    CHECK(quarta.segunda == "2026-11-09");
    CHECK(quarta.faltam == 5);
    const Semana domingo = Da("2026-11-15 23:59:59");
    CHECK(domingo.indice == segunda.indice);
    CHECK(domingo.faltam == 1);
    const Semana seguinte = Da("2026-11-16 00:00:00");
    CHECK(seguinte.indice == segunda.indice + 1);
    CHECK(seguinte.segunda == "2026-11-16");

    // A virada do mes e do ano, e o bissexto.
    CHECK(Da("2027-01-01 10:00:00").segunda == "2026-12-28");
    CHECK(Da("2028-03-01 10:00:00").segunda == "2028-02-28");
    CHECK(Da("1970-01-05 00:00:00").indice == 0);

    // A data que nao se le nao tem semana, nem materia.
    for (const char* ruim : {"", "ontem", "2026-13-01", "1969-12-31"}) {
        CAPTURE(ruim);
        CHECK_FALSE(Da(ruim).Valida());
        CHECK(MateriaDa(Da(ruim), Curso()) == -1);
    }
}

TEST_CASE("ProvaDaSemana: a mesma semana da a mesma semente e a mesma materia, e a seguinte troca as duas") {
    const Materias::Lista curso = Curso();
    const Semana s = Da("2026-11-10 09:00:00");
    CHECK(Semente(s) == Semente(Da("2026-11-14 22:00:00")));
    CHECK(MateriaDa(s, curso) == MateriaDa(Da("2026-11-14 22:00:00"), curso));

    // O rodizio so passa pelas que tem batalha e nao pedem orientador, e passa por todas.
    std::set<int> materias;
    std::set<std::uint32_t> sementes;
    Semana k = s;
    for (int i = 0; i < 8; ++i) {
        const int m = MateriaDa(k, curso);
        CHECK((m == 0 || m == 1));
        materias.insert(m);
        sementes.insert(Semente(k));
        ++k.indice;
    }
    CHECK(materias.size() == 2u);
    CHECK(sementes.size() == 8u);
    Semana proxima = s;
    ++proxima.indice;
    CHECK(MateriaDa(proxima, curso) != MateriaDa(s, curso));
    CHECK(MateriaDa(s, Materias::Lista{}) == -1);
}

TEST_CASE("ProvaDaSemana: o arquivo vai e volta, e a linha estragada e pulada") {
    Entrada e{"2026-11-09", "A1", true, false, "200", "100", 71.256f, "2026-11-10 09:00:00"};
    const std::string texto = Cabecalho() + "\n" + Linha(e) + "\r\n" + "lixo\n" + ";;;;;;\n" +
                              "2026-11-09;A1;solo;dificil;300;;abc;x\n" +
                              Linha({"2026-11-09", "A1", false, true, "300", "", 55.0f, ""}) + "\n";
    const std::vector<Entrada> lidas = Ler(texto);
    REQUIRE(lidas.size() == 2u);
    CHECK(lidas[0].semana == "2026-11-09");
    CHECK(lidas[0].materia == "A1");
    CHECK(lidas[0].dupla);
    CHECK_FALSE(lidas[0].dificil);
    CHECK(lidas[0].matricula == "200");
    CHECK(lidas[0].parceiro == "100");
    CHECK(lidas[0].nota == doctest::Approx(71.26f));
    CHECK(lidas[0].quando == "2026-11-10 09:00:00");
    CHECK_FALSE(lidas[1].dupla);
    CHECK(lidas[1].dificil);
    CHECK(Ler("").empty());
}

TEST_CASE("ProvaDaSemana: o ranking e a melhor nota de cada um naquela semana, modo e dificuldade") {
    const Semana s = Da("2026-11-10 09:00:00");
    const std::string sem = s.segunda;
    std::vector<Entrada> entradas = {
        {sem, "A1", false, false, "100", "", 60.0f, ""},
        {sem, "A1", false, false, "100", "", 82.0f, ""},     // a melhor do 100
        {sem, "A1", false, false, "100", "", 70.0f, ""},
        {sem, "A1", false, false, "200", "", 90.0f, ""},
        {sem, "A1", false, false, "300", "", 82.0f, ""},     // empata com o 100
        {sem, "A1", false, true, "400", "", 99.0f, ""},      // no Dificil: outra lista
        {"2026-11-02", "A1", false, false, "500", "", 100.0f, ""},   // semana passada
        // A dupla: uma entrada por aluno, uma linha so no ranking.
        {sem, "A1", true, false, "200", "100", 75.0f, ""},
        {sem, "A1", true, false, "100", "200", 75.0f, ""},
    };
    const auto solo = Ranking(entradas, s, false, false);
    REQUIRE(solo.size() == 3u);
    CHECK(solo[0].quem == "200");
    CHECK(solo[0].posicao == 1);
    CHECK(solo[1].nota == doctest::Approx(82.0f));
    CHECK(solo[1].posicao == 2);
    CHECK(solo[2].posicao == 2);                 // o empate repete a posicao
    CHECK(PosicaoDe(solo, "100") == 2);
    CHECK(PosicaoDe(solo, "500") == 0);

    const auto dificil = Ranking(entradas, s, false, true);
    REQUIRE(dificil.size() == 1u);
    CHECK(dificil[0].quem == "400");

    const auto dupla = Ranking(entradas, s, true, false);
    REQUIRE(dupla.size() == 1u);
    CHECK(dupla[0].quem == "100 + 200");
    CHECK(Quem(entradas[7]) == Quem(entradas[8]));
    CHECK(Ranking({}, s, false, false).empty());
    CHECK(kPodio == 3);
}
