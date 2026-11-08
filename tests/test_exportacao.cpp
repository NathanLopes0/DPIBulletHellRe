// As notas da turma em CSV.

#include "doctest.h"
#include "../Source/Regras/Dificuldade.h"

#include <string>
#include <vector>

#include "../Source/Aluno/Exportacao.h"
#include "../Source/Curso/Materias.h"
#include "../Source/Regras/Nota.h"

namespace {

    const Materias::Lista& Curso() {
        static const Materias::Lista l = Materias::LerMaterias(R"({
          "materias": [
            { "codigo": "INF213", "nome": "INF 213", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "INF420", "nome": "INF 420", "coluna": 1,
              "desbloqueio": { "tipo": "sempre" } }
          ]
        })");
        return l;
    }

    Exportacao::FichaDeAluno Aluno(const std::string& matricula,
                                   const std::vector<std::tuple<int, float, float, std::string>>& notas) {
        Exportacao::FichaDeAluno f;
        f.matricula = matricula;
        std::vector<Progresso::Entrada> entradas;
        for (const auto& n : notas) {
            Progresso::Entrada e;
            e.materia = std::get<0>(n);
            e.De(Progresso::Modo::Solo, Dificuldade::Nivel::Normal).nota = std::get<1>(n);
            e.retomada = std::get<2>(n);
            e.quando = std::get<3>(n);
            entradas.push_back(e);
        }
        f.progresso.Restaurar(entradas);
        return f;
    }

    size_t Linhas(const std::string& csv) {
        size_t n = 0;
        for (const char c : csv) if (c == '\n') ++n;
        return n;
    }
}

TEST_CASE("Exportacao: uma linha por aluno-materia, mais o cabecalho") {
    const std::string csv = Exportacao::ParaCsv({
        Aluno("89384", {{0, 85.0f, 45.0f, "2026-10-02 14:00:00"},
                        {1, 92.0f, 92.0f, "2026-10-02 15:00:00"}}),
        Aluno("1", {{0, 50.0f, 50.0f, "2026-09-30 10:00:00"}})
    }, Curso());

    CHECK(Linhas(csv) == 4);   // cabecalho + 3
    CHECK(csv.find("matricula,materia,nome,recorde,recordeDupla,parceiro,retomada,aprovado,quando") == 0);
}

TEST_CASE("Exportacao: o codigo E o nome saem, os dois") {
    // O codigo e o que nao muda e serve para cruzar dados; o nome e o que o
    // professor le.
    const std::string csv = Exportacao::ParaCsv({
        Aluno("89384", {{0, 85.0f, 85.0f, "2026-10-02 14:00:00"}})
    }, Curso());
    CHECK(csv.find("89384,INF213,INF 213,85.00,0.00,,85.00,sim,2026-10-02 14:00:00")
          != std::string::npos);
}

TEST_CASE("Exportacao: aprovado olha o RECORDE, nao a ultima nota") {
    // Quem passou e depois jogou mal continua aprovado.
    const std::string csv = Exportacao::ParaCsv({
        Aluno("1", {{0, 85.0f, 30.0f, ""}})
    }, Curso());
    CHECK(csv.find("85.00,0.00,,30.00,sim") != std::string::npos);
}

TEST_CASE("Exportacao: reprovado sai como nao") {
    const std::string csv = Exportacao::ParaCsv({Aluno("1", {{0, 59.9f, 59.9f, ""}})}, Curso());
    CHECK(csv.find(",nao,") != std::string::npos);
}

TEST_CASE("Exportacao: materia nunca jogada nao vira linha") {
    // Linha vazia poluiria o ranking.
    const std::string csv = Exportacao::ParaCsv({Aluno("1", {{0, 70.0f, 70.0f, ""}})}, Curso());
    CHECK(Linhas(csv) == 2);
    CHECK(csv.find("INF420") == std::string::npos);
}

TEST_CASE("Exportacao: as matriculas saem em ordem NUMERICA") {
    // "9" antes de "89384", e nao a ordem alfabetica, que poria "10" antes de "9".
    const std::string csv = Exportacao::ParaCsv({
        Aluno("89384", {{0, 10.0f, 10.0f, ""}}),
        Aluno("9", {{0, 20.0f, 20.0f, ""}}),
        Aluno("115849", {{0, 30.0f, 30.0f, ""}})
    }, Curso());
    const size_t p9 = csv.find("\n9,");
    const size_t p89 = csv.find("\n89384,");
    const size_t p115 = csv.find("\n115849,");
    CHECK(p9 < p89);
    CHECK(p89 < p115);
}

TEST_CASE("Exportacao: o mesmo estado produz o mesmo arquivo") {
    const auto um = Exportacao::ParaCsv({Aluno("1", {{0, 70.0f, 70.0f, "x"}})}, Curso());
    const auto dois = Exportacao::ParaCsv({Aluno("1", {{0, 70.0f, 70.0f, "x"}})}, Curso());
    CHECK(um == dois);
}

TEST_CASE("Exportacao: materia que saiu do curso nao vira linha") {
    const std::string csv = Exportacao::ParaCsv({Aluno("1", {{99, 70.0f, 70.0f, ""}})}, Curso());
    CHECK(Linhas(csv) == 1);   // so o cabecalho
}

TEST_CASE("Exportacao: turma vazia produz so o cabecalho") {
    CHECK(Linhas(Exportacao::ParaCsv({}, Curso())) == 1);
}

TEST_CASE("Exportacao: sem data o campo fica vazio, nao ausente") {
    // A coluna tem de existir em toda linha, senao a planilha desalinha.
    const std::string csv = Exportacao::ParaCsv({Aluno("1", {{0, 70.0f, 70.0f, ""}})}, Curso());
    // As tres do Dificil vem depois, vazias de recorde (0.00) e de parceiro.
    CHECK(csv.find("70.00,sim,,0.00,0.00,\n") != std::string::npos);
}

TEST_CASE("Exportacao: os recordes do Dificil vem nas ultimas colunas") {
    Exportacao::FichaDeAluno f;
    f.matricula = "1";
    f.progresso.RegistrarNota(0, 55.0f, Dificuldade::Nivel::Normal, "2026-11-01 10:00:00");
    f.progresso.RegistrarNota(0, 64.5f, Dificuldade::Nivel::Dificil, "2026-11-02 10:00:00");
    f.progresso.RegistrarNotaDeDupla(0, 71.0f, Dificuldade::Nivel::Dificil, "90112", "2026-11-03 10:00:00");

    const std::string csv = Exportacao::ParaCsv({f}, Curso());
    CHECK(csv.find(",quando,recordeDificil,recordeDuplaDificil,parceiroDificil\n") != std::string::npos);
    // Recorde (Normal) 55, e no fim os do Dificil: 64.50 sozinho, 71.00 com 90112.
    CHECK(csv.find("1,INF213,") != std::string::npos);
    CHECK(csv.find(",55.00,") != std::string::npos);
    CHECK(csv.find(",2026-11-03 10:00:00,64.50,71.00,90112\n") != std::string::npos);
    // Aprovado pelo Dificil: passar e passar, no nivel que for.
    CHECK(csv.find(",sim,") != std::string::npos);
}

// Escape de CSV.

TEST_CASE("Exportacao: campo com virgula ganha aspas") {
    // Um nome de materia com virgula quebraria a coluna seguinte.
    CHECK(Exportacao::Campo("INF 213") == "INF 213");
    CHECK(Exportacao::Campo("Estruturas, parte 1") == "\"Estruturas, parte 1\"");
}

TEST_CASE("Exportacao: aspas no campo sao duplicadas, como manda o CSV") {
    CHECK(Exportacao::Campo("o \"melhor\"") == "\"o \"\"melhor\"\"\"");
}

TEST_CASE("Exportacao: quebra de linha no campo tambem ganha aspas") {
    CHECK(Exportacao::Campo("a\nb") == "\"a\nb\"");
}

TEST_CASE("Exportacao: um nome com virgula nao desalinha a planilha") {
    const auto comVirgula = Materias::LerMaterias(R"({
      "materias": [ { "codigo": "X", "nome": "Estruturas, parte 1", "coluna": 0,
                      "desbloqueio": { "tipo": "sempre" } } ]
    })");
    const std::string csv = Exportacao::ParaCsv({Aluno("1", {{0, 70.0f, 70.0f, ""}})}, comVirgula);
    CHECK(csv.find("1,X,\"Estruturas, parte 1\",70.00,") != std::string::npos);
}

TEST_CASE("Exportacao: o teto de 99,99 nao vira 100 na planilha") {

    const std::string csv =
        Exportacao::ParaCsv({Aluno("1", {{0, Nota::kTetoComDano, Nota::kTetoComDano, ""}})}, Curso());

    CHECK(csv.find("99.99") != std::string::npos);
    CHECK(csv.find("100.0") == std::string::npos);
}

TEST_CASE("Exportacao: a nota cheia de verdade sai como 100") {

    const std::string csv =
        Exportacao::ParaCsv({Aluno("1", {{0, Nota::kNotaMaxima, Nota::kNotaMaxima, ""}})}, Curso());

    CHECK(csv.find("100.00") != std::string::npos);
}

TEST_CASE("Exportacao: a planilha e o jogo concordam sobre quem passou") {

    // A planilha comparava o recorde de solo com 60 por conta propria.
    Progresso p;
    p.RegistrarNotaDeDupla(0, 75.0f, Dificuldade::Nivel::Normal, "90112", "2026-10-14 15:00:00");

    REQUIRE(p.Aprovado(0));   // o jogo diz que passou

    Exportacao::FichaDeAluno f;
    f.matricula = "89384";
    f.progresso = p;

    const std::string csv = Exportacao::ParaCsv({f}, Curso());

    CHECK(csv.find(",sim,") != std::string::npos);
    CHECK(csv.find("90112") != std::string::npos);   // e com quem
}
