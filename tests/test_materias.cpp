// As materias do curso. Camada pura: nao abre arquivo, nao sobe SDL.
//
// Os testes de desbloqueio sao os que importam: as regras saiam de dois lugares
// em C++ que tinham o MESMO conjunto de materias com nomes trocados (o que
// IsStageUnlocked chamava de col2, o StageSelect chamava de col1Data). Agora a
// coluna e declarada uma vez e a regra deriva dela.

#include "doctest.h"

#include <string>

#include "../Source/Materias.h"
#include "../Source/Progresso.h"

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
        for (const auto& c : codigos) p.RegistrarNota(l.IndiceDe(c), 80.0f);
        return p;
    }
}

// ---------------------------------------------------------------------------
// Leitura
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Desbloqueio
// ---------------------------------------------------------------------------

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
    reprovado.RegistrarNota(l.IndiceDe("A"), 55.0f);   // jogou e nao passou
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
    // O desbloqueio le o RECORDE, nao a ultima nota. Era um bug que ja existiu
    // neste jogo antes de Progresso separar as duas coisas.
    const auto l = Materias::LerMaterias(kCurso);
    Progresso p;
    p.RegistrarNota(l.IndiceDe("A"), 80.0f);
    p.RegistrarNota(l.IndiceDe("A"), 30.0f);
    CHECK(l.Desbloqueada(l.IndiceDe("B"), p));
}

TEST_CASE("Materias: exigencia mutua nao causa laco infinito") {
    // "A exige B" e "B exige A" olham a NOTA um do outro, nao o desbloqueio.
    // Entao as duas ficam fechadas, que e o resultado certo, e nada recorre.
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

// ---------------------------------------------------------------------------
// As conferencias que so dao para fazer com a lista inteira
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// A propriedade que motiva o arquivo inteiro
// ---------------------------------------------------------------------------

TEST_CASE("Materias: reordenar a lista NAO muda os codigos") {
    // Era isto que o enum nao dava: a posicao mudava e os saves iam junto. Aqui a
    // mesma materia tem o mesmo codigo em qualquer ordem - so o indice, que e de
    // uso interno, se mexe.
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

// ---------------------------------------------------------------------------
// A frase que explica por que a materia esta fechada
//
// A tela de selecao mostrava um losango escuro e nada mais: o aluno via que nao
// dava para entrar e nao tinha como saber o que faltava. A frase vem da MESMA
// regra que fecha a materia, entao ela nao pode explicar uma coisa e o jogo
// exigir outra.
// ---------------------------------------------------------------------------

TEST_CASE("Materias: materia que abre sempre nao tem o que explicar") {
    const auto l = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "A", "nome": "INF 110", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    CHECK(l.ExigenciaDe(l.IndiceDe("A")).empty());
}

TEST_CASE("Materias: a exigencia cita o NOME em tela, nao o codigo") {
    // O botao ao lado diz "INF 213"; o codigo gravado e "INF213". Citar o
    // codigo mandaria o aluno procurar na tela uma materia que nao esta escrita
    // daquele jeito em lugar nenhum.
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
    // Nada no formato obriga a regra a olhar para a coluna de tras. Quando ela
    // olha para outra, dizer "anterior" seria mentira - e o aluno conta coluna
    // olhando para a tela, da esquerda para a direita, comecando em um.
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
    // A PROPRIEDADE QUE IMPORTA. Nao basta a frase existir: ela tem de falar da
    // mesma regra que Desbloqueada aplica. Aqui o aluno passa na materia que a
    // frase cita e a porta abre - se a frase citasse outra, abriria sem ela.
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
    passou.RegistrarNota(l.IndiceDe("INF110"), 70.0f);
    CHECK(l.Desbloqueada(fechada, passou));
}
