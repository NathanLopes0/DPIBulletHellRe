// As materias do curso.

#include "doctest.h"
#include "../Source/Regras/Dificuldade.h"

#include <string>

#include "../Source/Curso/Materias.h"
#include "../Source/Aluno/Progresso.h"

namespace {

    bool Menciona(const std::vector<std::string>& problemas, const std::string& trecho) {
        for (const auto& p : problemas) {
            if (p.find(trecho) != std::string::npos) return true;
        }
        return false;
    }

    /// Um curso pequeno com as tres formas de desbloqueio.
    const char* kCurso = R"({
      "materias": [
        { "codigo": "A", "nome": "Materia A", "coluna": 0, "chefe": "salles",
          "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "Materia B", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } },
        { "codigo": "C", "nome": "Materia C", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } },
        { "codigo": "D", "nome": "Materia D", "coluna": 2,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 2, "coluna": 1 } }
      ]
    })";

    Progresso ComAprovacoes(const Materias::Lista& l, const std::vector<std::string>& codigos) {
        Progresso p;
        for (const auto& c : codigos) p.RegistrarNota(l.IndiceDe(c), 80.0f, Dificuldade::Nivel::Normal);
        return p;
    }
}

// Leitura.

TEST_CASE("Materias: le a lista inteira") {
    const auto l = Materias::LerMaterias(kCurso);
    CHECK(l.problemas.empty());
    REQUIRE(l.Quantas() == 4);
    CHECK(l.materias[0].codigo == "A");
    CHECK(l.materias[0].nome == "Materia A");
    CHECK(l.materias[0].chefe == "salles");
    CHECK(l.materias[1].chefe.empty());   // materia sem chefe e normal
}

TEST_CASE("Materias: o nome pode ser qualquer coisa, o codigo nao") {
    // O nome e so o que aparece no botao; o codigo e a chave que vai para o save.
    CHECK(Materias::CodigoServe("INF213"));
    CHECK(Materias::CodigoServe("TCC"));
    CHECK(Materias::CodigoServe("A1"));
    CHECK_FALSE(Materias::CodigoServe("INF 213"));   // espaco
    CHECK_FALSE(Materias::CodigoServe("INF-213"));   // hifen
    CHECK_FALSE(Materias::CodigoServe(""));
}

TEST_CASE("Materias: indice e codigo se convertem nos dois sentidos") {
    const auto l = Materias::LerMaterias(kCurso);
    for (int i = 0; i < l.Quantas(); ++i) {
        CAPTURE(i);
        CHECK(l.IndiceDe(l.CodigoDe(i)) == i);
    }
    CHECK(l.IndiceDe("NAOEXISTE") == -1);
    CHECK(l.CodigoDe(99).empty());
    CHECK(l.Por(99) == nullptr);
}

TEST_CASE("Materias: a coluna agrupa, e e declarada uma vez so") {
    const auto l = Materias::LerMaterias(kCurso);
    CHECK(l.DaColuna(0).size() == 1);
    CHECK(l.DaColuna(1).size() == 2);
    CHECK(l.DaColuna(2).size() == 1);
    CHECK(l.DaColuna(7).empty());
    CHECK(l.QuantasColunas() == 3);
}

// Desbloqueio.

TEST_CASE("Materias: 'sempre' abre com progresso zerado") {
    const auto l = Materias::LerMaterias(kCurso);
    const Progresso vazio;
    CHECK(l.Desbloqueada(l.IndiceDe("A"), vazio));
    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("B"), vazio));
    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("D"), vazio));
}

TEST_CASE("Materias: 'aprovadoEm' exige a nota, nao a tentativa") {
    const auto l = Materias::LerMaterias(kCurso);

    Progresso reprovado;
    reprovado.RegistrarNota(l.IndiceDe("A"), 55.0f, Dificuldade::Nivel::Normal);   // jogou e nao passou
    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("B"), reprovado));

    const auto aprovado = ComAprovacoes(l, {"A"});
    CHECK(l.Desbloqueada(l.IndiceDe("B"), aprovado));
    CHECK(l.Desbloqueada(l.IndiceDe("C"), aprovado));
}

TEST_CASE("Materias: 'aprovadasNaColuna' conta quantas, nao quais") {
    const auto l = Materias::LerMaterias(kCurso);

    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("D"), ComAprovacoes(l, {"A"})));
    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("D"), ComAprovacoes(l, {"A", "B"})));
    CHECK(l.Desbloqueada(l.IndiceDe("D"), ComAprovacoes(l, {"A", "B", "C"})));
}

TEST_CASE("Materias: uma reprovacao depois de aprovar nao re-tranca") {
    // O desbloqueio le o recorde, nao a ultima nota.
    const auto l = Materias::LerMaterias(kCurso);
    Progresso p;
    p.RegistrarNota(l.IndiceDe("A"), 80.0f, Dificuldade::Nivel::Normal);
    p.RegistrarNota(l.IndiceDe("A"), 30.0f, Dificuldade::Nivel::Normal);
    CHECK(l.Desbloqueada(l.IndiceDe("B"), p));
}

TEST_CASE("Materias: exigencia mutua nao causa laco infinito") {
    // "A exige B" e "B exige A" olham a nota um do outro, nao o desbloqueio.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "X", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "A", "coluna": 1, "desbloqueio": { "tipo": "aprovadoEm", "materias": ["B"] } },
        { "codigo": "B", "coluna": 1, "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } }
      ]
    })");
    const Progresso vazio;
    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("A"), vazio));
    CHECK_FALSE(l.Desbloqueada(l.IndiceDe("B"), vazio));
}

TEST_CASE("Materias: indice fora da lista nunca esta desbloqueado") {
    const auto l = Materias::LerMaterias(kCurso);
    CHECK_FALSE(l.Desbloqueada(-1, Progresso()));
    CHECK_FALSE(l.Desbloqueada(99, Progresso()));
}

// As conferencias que so dao para fazer com a lista inteira.

TEST_CASE("Materias: regra que cita codigo inexistente e relatada") {
    // Em jogo isso nao daria erro: a materia so ficaria fechada para sempre.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["NAOEXISTE"] } }
      ]
    })");
    CHECK(Menciona(l.problemas, "NAOEXISTE"));
    CHECK(Menciona(l.problemas, "nunca vai abrir"));
}

TEST_CASE("Materias: regra IMPOSSIVEL e relatada") {
    // Duas aprovacoes numa coluna de uma materia so: aquela materia nunca abre.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 2, "coluna": 0 } }
      ]
    })");
    CHECK(Menciona(l.problemas, "nunca vai abrir"));
    CHECK(Menciona(l.problemas, "so tem 1"));
}

TEST_CASE("Materias: curso sem nenhuma porta de entrada e relatado") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } }
      ]
    })");
    CHECK(Menciona(l.problemas, "sempre"));
}

TEST_CASE("Materias: codigo repetido fica com a primeira") {
    // Duas materias com o mesmo codigo dividiriam a mesma nota no save.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "primeira", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "A", "nome": "segunda", "coluna": 1, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK(Menciona(l.problemas, "repetido"));
    REQUIRE(l.Quantas() == 1);
    CHECK(l.materias[0].nome == "primeira");
}

TEST_CASE("Materias: uma materia ruim nao leva as boas") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1 },
        { "codigo": "C", "coluna": 1, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK(Menciona(l.problemas, "desbloqueio"));
    CHECK(l.Quantas() == 2);
    CHECK(l.IndiceDe("B") == -1);
}

TEST_CASE("Materias: tipo de desbloqueio desconhecido e recusado") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "quandoEuQuiser" } }
      ]
    })");
    CHECK(Menciona(l.problemas, "quandoEuQuiser"));
}

TEST_CASE("Materias: falta coluna e a materia cai") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [ { "codigo": "A", "desbloqueio": { "tipo": "sempre" } } ]
    })");
    CHECK(Menciona(l.problemas, "coluna"));
}

TEST_CASE("Materias: texto que nao e JSON e relatado, nao lancado") {
    CHECK_FALSE(Materias::LerMaterias("{ nao fecha").problemas.empty());
    CHECK(Materias::LerMaterias("{ nao fecha").Quantas() == 0);
    CHECK(Materias::LerMaterias("{}").Quantas() == 0);
    CHECK(Materias::LerMaterias("").Quantas() == 0);
}

// A propriedade que motiva o arquivo inteiro.

TEST_CASE("Materias: reordenar a lista NAO muda os codigos") {
    // Aqui a mesma materia tem o mesmo codigo em qualquer ordem.
    const auto antes = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    const auto depois = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "Z", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "coluna": 1, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");

    // O indice de A mudou...
    CHECK(antes.IndiceDe("A") == 0);
    CHECK(depois.IndiceDe("A") == 1);

    // ...mas o codigo, que e o que vai para o disco, continua "A".
    CHECK(antes.CodigoDe(antes.IndiceDe("A")) == depois.CodigoDe(depois.IndiceDe("A")));
}

// A frase que explica por que a materia esta fechada.

TEST_CASE("Materias: materia que abre sempre nao tem o que explicar") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "INF 110", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("A")).empty());
}

TEST_CASE("Materias: a exigencia cita o NOME em tela, nao o codigo") {
    // O botao ao lado diz "INF 213"; o codigo gravado e "INF213".
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "INF213", "nome": "INF 213", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "INF250", "nome": "INF 250", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF213"] } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("INF250")) == "Precisa passar em INF 213");
}

TEST_CASE("Materias: a exigencia lista varias materias com virgula e 'e'") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "INF 110", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "INF 213", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "C", "nome": "INF 250", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "D", "nome": "TCC", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A", "B", "C"] } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("D")) == "Precisa passar em INF 110, INF 213 e INF 250");
}

TEST_CASE("Materias: aprovacoes na coluna de tras sao 'a coluna anterior'") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "B", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "C", "nome": "C", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 2, "coluna": 0 } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("C")) == "Precisa de 2 aprovacoes na coluna anterior");
}

TEST_CASE("Materias: coluna que NAO e a anterior aparece numerada a partir de 1") {
    // Nada no formato obriga a regra a olhar para a coluna de tras.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "B", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "C", "nome": "C", "coluna": 3,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 2, "coluna": 0 } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("C")) == "Precisa de 2 aprovacoes na coluna 1");
}

TEST_CASE("Materias: uma aprovacao so e dita no singular") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "C", "nome": "C", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 1, "coluna": 0 } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("C")) == "Precisa de 1 aprovacao na coluna anterior");
}

TEST_CASE("Materias: indice fora da lista nao tem exigencia nem quebra") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK(l.ExigenciaDe(-1).empty());
    CHECK(l.ExigenciaDe(99).empty());
}

TEST_CASE("Materias: a exigencia concorda com a regra que fecha a materia") {
    // A propriedade que importa.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "INF110", "nome": "INF 110", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "INF213", "nome": "INF 213", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["INF110"] } }
      ]
    })");
    const int fechada = l.IndiceDe("INF213");
    const std::string frase = l.ExigenciaDe(fechada);

    Progresso vazio;
    CHECK_FALSE(l.Desbloqueada(fechada, vazio));
    CHECK(frase.find(l.Por(l.IndiceDe("INF110"))->nome) != std::string::npos);

    Progresso passou;
    passou.RegistrarNota(l.IndiceDe("INF110"), 70.0f, Dificuldade::Nivel::Normal);
    CHECK(l.Desbloqueada(fechada, passou));
}

// O nome da disciplina e o professor. Sao campos de apresentacao: nao vao para o disco, nao entram
// em regra de desbloqueio e nao escolhem chefe.

TEST_CASE("Materias: le o nome da disciplina e o professor") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "INF220", "nome": "INF 220",
          "nomeCompleto": "Banco de Dados", "professor": "Thiago",
          "coluna": 0, "chefe": "thiago", "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    const Materias::Materia* m = l.Por(l.IndiceDe("INF220"));
    REQUIRE(m != nullptr);
    CHECK(m->nomeCompleto == "Banco de Dados");
    CHECK(m->professor == "Thiago");
}

TEST_CASE("Materias: sem os campos novos a materia continua valendo, com eles vazios") {
    // Toda materia anterior a estes campos cai aqui.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK(l.problemas.empty());
    const Materias::Materia* m = l.Por(l.IndiceDe("A"));
    REQUIRE(m != nullptr);
    CHECK(m->nomeCompleto.empty());
    CHECK(m->professor.empty());
}

TEST_CASE("Materias: ter professor NAO e ter batalha") {
    // O TCC de verdade: professor conhecido, chefe ainda nao feito.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "TCC", "nome": "TCC",
          "nomeCompleto": "Trabalho de Conclusao de Curso", "professor": "Hugo",
          "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    const Materias::Materia* m = l.Por(l.IndiceDe("TCC"));
    REQUIRE(m != nullptr);
    CHECK(m->professor == "Hugo");
    CHECK(m->chefe.empty());
}

// A ligacao desenhada entre duas colunas. A tela de selecao liga as colunas vizinhas com um tronco.

TEST_CASE("Materias: coluna que depende toda da anterior pode ser ligada") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "B", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } },
        { "codigo": "C", "nome": "C", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } }
      ]
    })");
    CHECK(l.ColunaDependeDaAnterior(1));
}

TEST_CASE("Materias: a coluna 0 nunca e ligada - ela nao tem anterior") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK_FALSE(l.ColunaDependeDaAnterior(0));
    CHECK_FALSE(l.ColunaDependeDaAnterior(-1));
    CHECK_FALSE(l.ColunaDependeDaAnterior(9));   // coluna vazia
}

TEST_CASE("Materias: dependencia que PULA uma coluna nao vira linha") {
    // O caso que faria o desenho mentir: a linha sairia da coluna 1 e a regra
    // olharia para a 0.
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "B", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } },
        { "codigo": "C", "nome": "C", "coluna": 2,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } }
      ]
    })");
    CHECK(l.ColunaDependeDaAnterior(1));
    CHECK_FALSE(l.ColunaDependeDaAnterior(2));
}

TEST_CASE("Materias: basta UMA materia fora do padrao para nao ligar a coluna") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "B", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadoEm", "materias": ["A"] } },
        { "codigo": "C", "nome": "C", "coluna": 1, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK_FALSE(l.ColunaDependeDaAnterior(1));
}

TEST_CASE("Materias: aprovadasNaColuna so liga quando aponta para a coluna de tras") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "A", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "B", "nome": "B", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "C", "nome": "C", "coluna": 1,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 2, "coluna": 0 } },
        { "codigo": "D", "nome": "D", "coluna": 2,
          "desbloqueio": { "tipo": "aprovadasNaColuna", "quantas": 1, "coluna": 0 } }
      ]
    })");
    CHECK(l.ColunaDependeDaAnterior(1));
    CHECK_FALSE(l.ColunaDependeDaAnterior(2));
}
