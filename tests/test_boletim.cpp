// O boletim do semestre.

#include "doctest.h"

#include "../Source/Aluno/Boletim.h"

namespace {
    const Materias::Lista& Grade() {
        static const Materias::Lista l = Materias::LerMaterias(R"({
          "materias": [
            { "codigo": "A1", "nome": "INF 1", "nomeCompleto": "Um", "professor": "Ana", "coluna": 0,
              "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "B1", "nome": "INF 2", "professor": "Bia", "coluna": 1,
              "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"] } },
            { "codigo": "B2", "nome": "INF 3", "professor": "Caio", "coluna": 1,
              "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"] } },
            { "codigo": "C1", "nome": "TCC", "professor": "Davi", "coluna": 2,
              "desbloqueio": { "tipo": "aprovadoEm", "materias": ["B1", "B2"] } }
          ]
        })");
        return l;
    }
    using Boletim::Situacao;
}

TEST_CASE("Boletim: periodo a periodo, com a situacao de cada materia e o CR das cursadas") {
    Progresso p;
    // A1: duas reprovacoes e uma aprovacao. B1: uma reprovacao.
    p.RegistrarNota(0, 35.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(0, 50.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(0, 80.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(1, 40.0f, Dificuldade::Nivel::Normal);

    const Boletim::Dados d = Boletim::Montar(p, Grade());
    REQUIRE(d.linhas.size() == 4u);
    CHECK(d.total == 4);
    CHECK(d.linhas[0].codigo == "INF 1");
    CHECK(d.linhas[0].nome == "Um");
    CHECK(d.linhas[0].periodo == 1);
    CHECK(d.linhas[2].periodo == 2);
    CHECK(d.linhas[3].periodo == 3);

    CHECK(d.linhas[0].situacao == Situacao::Aprovade);
    CHECK(d.linhas[0].melhor == doctest::Approx(80.0f));
    CHECK(d.linhas[0].tentativas == 3);
    CHECK(d.linhas[0].reprovacoes == 2);
    CHECK(d.linhas[1].situacao == Situacao::Reprovade);
    CHECK(d.linhas[2].situacao == Situacao::ACursar);   // aberta pela A1, e nunca jogada
    CHECK(d.linhas[3].situacao == Situacao::Fechada);   // pede B1 e B2

    // O CR e a media das cursadas: (80 + 40) / 2. As que nunca foram jogadas nao entram.
    CHECK(d.cursadas == 2);
    CHECK(d.cr == doctest::Approx(60.0f));
    CHECK(d.aprovadas == 1);
}

TEST_CASE("Boletim: o Dificil conta, a dupla conta, e o save de antes da contagem nao quebra") {
    Progresso p;
    p.RegistrarNotaDeDupla(0, 65.0f, Dificuldade::Nivel::Dificil, "12345");
    Boletim::Dados d = Boletim::Montar(p, Grade());
    CHECK(d.linhas[0].situacao == Situacao::Aprovade);
    CHECK(d.linhas[0].noDificil);
    CHECK(d.linhas[0].tentativas == 1);

    // Um save antigo: o recorde existe, a contagem nao. A materia foi cursada, com "-" tentativas.
    Progresso antigo;
    Progresso::Entrada e;
    e.materia = 1;
    e.retomada = 30.0f;
    e.De(Progresso::Modo::Solo, Dificuldade::Nivel::Normal).nota = 30.0f;
    antigo.Restaurar({e});
    d = Boletim::Montar(antigo, Grade());
    CHECK(d.linhas[1].situacao == Situacao::Reprovade);
    CHECK(Boletim::Tentativas(d.linhas[1]) == "-");
    CHECK(d.cursadas == 1);
    CHECK(d.cr == doctest::Approx(30.0f));

    // Ninguem cursou nada: CR zero, e nao divisao por zero.
    d = Boletim::Montar(Progresso{}, Grade());
    CHECK(d.cursadas == 0);
    CHECK(d.cr == 0.0f);
    CHECK(d.linhas[0].situacao == Situacao::ACursar);
}

TEST_CASE("Boletim: os textos da tabela") {
    CHECK(std::string(Boletim::NomeDa(Situacao::Aprovade)) == "Aprovade");
    CHECK(std::string(Boletim::NomeDa(Situacao::Fechada)) == "Fechada");
    CHECK(Boletim::Nota(74.321f) == "74.32");
    CHECK(Boletim::Nota(0.0f) == "-");
    CHECK(Boletim::Periodo(2) == "2\xc2\xba per\xc3\xadodo");
}

TEST_CASE("Boletim: as optativas numa secao so, no fim, e fora da conta das aprovadas") {
    // A optativa fica numa coluna antes do TCC de proposito: mesmo assim ela vem por ultimo.
    const Materias::Lista l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A1", "nome": "INF 1", "professor": "Ana", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "OP", "nome": "INF 9", "professor": "Eva", "coluna": 2, "optativa": true,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"], "preco": 5 } },
        { "codigo": "B1", "nome": "INF 2", "professor": "Bia", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A1"] } }
      ]
    })");
    Progresso p;
    p.RegistrarNota(0, 80.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(1, 90.0f, Dificuldade::Nivel::Normal);   // a optativa, aprovada
    const Boletim::Dados d = Boletim::Montar(p, l);
    REQUIRE(d.linhas.size() == 3u);
    CHECK(d.linhas.back().codigo == "INF 9");
    CHECK(d.linhas.back().optativa);
    CHECK(Boletim::Secao(d.linhas.back()) == "Optativas");
    CHECK(Boletim::Secao(d.linhas.front()) == Boletim::Periodo(1));
    CHECK(d.total == 2);       // as obrigatorias, como na selecao de fases
    CHECK(d.aprovadas == 1);   // a optativa aprovada nao conta
    CHECK(d.cursadas == 2);    // mas entra no CR
}

TEST_CASE("Boletim: o nome longo e encurtado com reticencias, contando letras e nao bytes") {
    CHECK(Boletim::Encurtar("curto", 10) == "curto");
    CHECK(Boletim::Encurtar("exatamente", 10) == "exatamente");
    CHECK(Boletim::Encurtar("passou do limite", 10) == "passou do\xe2\x80\xa6");
    // O acento conta como uma letra so, e nunca e partido ao meio.
    const std::string longo = "Programa\xc3\xa7\xc3\xa3o Concorrente";
    const std::string curto = Boletim::Encurtar(longo, 11);
    CHECK(curto == "Programa\xc3\xa7\xc3\xa3\xe2\x80\xa6");
}
