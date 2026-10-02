// A ficha do aluno em texto. Camada pura: nao abre arquivo nem sobe SDL.
//
// O teste que mais importa aqui e o de IDA E VOLTA: gravar e ler de volta tem de
// devolver exatamente o mesmo estado. Um save que perde metade do progresso nao
// da erro nenhum - o aluno so descobre que voltou para o comeco.

#include "doctest.h"

#include <string>

#include "../Source/Ficha.h"
#include "../Source/Progresso.h"

namespace {

    bool Menciona(const std::vector<std::string>& problemas, const std::string& trecho) {
        for (const auto& p : problemas) {
            if (p.find(trecho) != std::string::npos) return true;
        }
        return false;
    }

    /// Uma ficha com progresso em tres materias, incluindo o caso que separa
    /// recorde de retomada.
    Ficha::Dados FichaDeExemplo() {
        Ficha::Dados d;
        d.matricula = "89384";
        d.progresso.RegistrarNota(0, 85.0f);   // depois cai
        d.progresso.RegistrarNota(0, 45.0f);   // recorde 85, retomada 45
        d.progresso.RegistrarNota(2, 100.0f);
        d.progresso.RegistrarNota(5, 40.0f);
        return d;
    }
}

// ---------------------------------------------------------------------------
// Ida e volta
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: gravar e ler de volta devolve o mesmo estado") {
    const auto original = FichaDeExemplo();
    const auto lida = Ficha::Desserializar(Ficha::Serializar(original));

    REQUIRE(lida.ok);
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.matricula == "89384");

    const auto a = original.progresso.Entradas();
    const auto b = lida.dados.progresso.Entradas();
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        CAPTURE(i);
        CHECK(a[i].materia == b[i].materia);
        CHECK(a[i].recorde == doctest::Approx(b[i].recorde));
        CHECK(a[i].retomada == doctest::Approx(b[i].retomada));
    }
}

TEST_CASE("Ficha: a ida e volta preserva recorde DIFERENTE da retomada") {
    // O caso que um save ingenuo perde. Guardar so o recorde faria o aluno voltar
    // a comecar do 40; guardar so a retomada o faria perder a aprovacao.
    const auto lida = Ficha::Desserializar(Ficha::Serializar(FichaDeExemplo()));
    REQUIRE(lida.ok);
    CHECK(lida.dados.progresso.MelhorNota(0) == doctest::Approx(85.0f));
    CHECK(lida.dados.progresso.NotaDeRetomada(0) == doctest::Approx(45.0f));
    CHECK(lida.dados.progresso.Aprovado(0));
}

TEST_CASE("Ficha: aluno sem nenhuma nota tambem fecha a ida e volta") {
    Ficha::Dados novo;
    novo.matricula = "1";
    const auto lida = Ficha::Desserializar(Ficha::Serializar(novo));
    REQUIRE(lida.ok);
    CHECK(lida.dados.matricula == "1");
    CHECK(lida.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: o mesmo estado produz sempre o MESMO texto") {
    // Para dar para comparar dois saves num diff quando algo parecer errado.
    CHECK(Ficha::Serializar(FichaDeExemplo()) == Ficha::Serializar(FichaDeExemplo()));
}

TEST_CASE("Ficha: o texto gravado traz a versao do formato") {
    const auto texto = Ficha::Serializar(FichaDeExemplo());
    CHECK(texto.find("\"versao\"") != std::string::npos);
    CHECK(texto.find("\"matricula\"") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Texto que nao serve
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: texto que nao e JSON e relatado, nao lancado") {
    const auto r = Ficha::Desserializar("{ isto nao fecha");
    CHECK_FALSE(r.ok);
    CHECK_FALSE(r.problemas.empty());
}

TEST_CASE("Ficha: arquivo vazio nao quebra") {
    CHECK_FALSE(Ficha::Desserializar("").ok);
    CHECK_FALSE(Ficha::Desserializar("{}").ok);
}

TEST_CASE("Ficha: sem matricula a ficha nao serve") {
    // Sem matricula nao da para saber de quem e o progresso.
    const auto r = Ficha::Desserializar(R"({ "versao": 1, "materias": [] })");
    CHECK_FALSE(r.ok);
    CHECK(Menciona(r.problemas, "matricula"));
}

TEST_CASE("Ficha: sem versao a ficha nao serve") {
    // Todo arquivo gravado por este jogo tem versao; um sem ela nao veio daqui.
    const auto r = Ficha::Desserializar(R"({ "matricula": "89384", "materias": [] })");
    CHECK_FALSE(r.ok);
    CHECK(Menciona(r.problemas, "versao"));
}

TEST_CASE("Ficha: versao do futuro e recusada com uma frase que explica") {
    // Melhor recusar do que ler errado: um formato mais novo pode ter mudado o
    // sentido de um campo que esta leitura ainda entende.
    const auto r = Ficha::Desserializar(R"({ "versao": 99, "matricula": "1", "materias": [] })");
    CHECK_FALSE(r.ok);
    CHECK(Menciona(r.problemas, "versao"));
}

// ---------------------------------------------------------------------------
// Materias com problema: descartadas uma a uma
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: uma materia estragada nao leva as outras") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [
        { "materia": 0, "recorde": 70, "retomada": 70 },
        { "recorde": 50, "retomada": 50 },
        { "materia": 3, "recorde": 90, "retomada": 90 }
      ]
    })");
    CHECK(r.ok);
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.dados.progresso.QuantasRegistradas() == 2);
    CHECK(r.dados.progresso.MelhorNota(0) == doctest::Approx(70.0f));
    CHECK(r.dados.progresso.MelhorNota(3) == doctest::Approx(90.0f));
}

TEST_CASE("Ficha: nota fora de 0 a 100 e recusada") {
    // A batalha trava a nota nessa faixa (Math::Clamp em Battle.cpp), entao um
    // valor fora dela so pode vir de arquivo editado a mao ou corrompido - e
    // aceitar um recorde de 1e30 deixaria o aluno aprovado para sempre.
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [
        { "materia": 0, "recorde": 101, "retomada": 50 },
        { "materia": 1, "recorde": 50, "retomada": -1 },
        { "materia": 2, "recorde": 80, "retomada": 80 }
      ]
    })");
    CHECK(r.ok);
    CHECK(r.dados.progresso.QuantasRegistradas() == 1);
    CHECK(r.dados.progresso.MelhorNota(2) == doctest::Approx(80.0f));
}

TEST_CASE("Ficha: zero e cem sao notas validas, nao extremos recusados") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [
        { "materia": 0, "recorde": 0, "retomada": 0 },
        { "materia": 1, "recorde": 100, "retomada": 100 }
      ]
    })");
    CHECK(r.ok);
    CHECK(r.problemas.empty());
    CHECK(r.dados.progresso.QuantasRegistradas() == 2);
}

TEST_CASE("Ficha: retomada acima do recorde e recusada") {
    // Estado impossivel: o recorde e o maximo de todas as notas, entao nenhuma
    // retomada pode passar dele. Se passasse, "melhor nota" deixaria de ser a
    // melhor nota e as regras de aprovacao ficariam erradas.
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [ { "materia": 0, "recorde": 50, "retomada": 80 } ]
    })");
    CHECK(r.dados.progresso.QuantasRegistradas() == 0);
    CHECK_FALSE(r.problemas.empty());
}

TEST_CASE("Ficha: materia repetida fica com a ultima e avisa") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [
        { "materia": 0, "recorde": 70, "retomada": 70 },
        { "materia": 0, "recorde": 90, "retomada": 90 }
      ]
    })");
    CHECK(r.dados.progresso.QuantasRegistradas() == 1);
    CHECK(Menciona(r.problemas, "repetida"));
}

TEST_CASE("Ficha: materia negativa e recusada") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [ { "materia": -1, "recorde": 70, "retomada": 70 } ]
    })");
    CHECK(r.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: lista de materias ausente e um aluno sem nota, nao um erro") {
    // Aluno que se identificou e ainda nao jogou.
    const auto r = Ficha::Desserializar(R"({ "versao": 1, "matricula": "89384" })");
    CHECK(r.ok);
    CHECK(r.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: a matricula gravada e lida de volta igual") {
    for (const char* m : {"1", "89384", "115849", "999999"}) {
        CAPTURE(m);
        Ficha::Dados d;
        d.matricula = m;
        const auto r = Ficha::Desserializar(Ficha::Serializar(d));
        REQUIRE(r.ok);
        CHECK(r.dados.matricula == m);
    }
}
