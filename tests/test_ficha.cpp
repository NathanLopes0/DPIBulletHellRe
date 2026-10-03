// A ficha do aluno em texto. Camada pura: nao abre arquivo nem sobe SDL.
//
// O teste que mais importa aqui e o de IDA E VOLTA: gravar e ler de volta tem de
// devolver exatamente o mesmo estado. Um save que perde metade do progresso nao
// da erro nenhum - o aluno so descobre que voltou para o comeco.

#include "doctest.h"

#include <string>

#include "../Source/Ficha.h"
#include "../Source/Materias.h"
#include "../Source/Relogio.h"
#include "../Source/Progresso.h"

namespace {

    /// Um curso pequeno so para estes testes. Os codigos sao o que vai para o
    /// arquivo; os indices (0, 1, 2, ...) sao internos e seguem esta ordem.
    const Materias::Lista& Curso() {
        static const Materias::Lista l = Materias::LerMaterias(R"({
          "materias": [
            { "codigo": "AAA", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "BBB", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "CCC", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "DDD", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "EEE", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
            { "codigo": "FFF", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
          ]
        })");
        return l;
    }

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
    const auto lida = Ficha::Desserializar(Ficha::Serializar(original, Curso()), Curso());

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
    const auto lida = Ficha::Desserializar(Ficha::Serializar(FichaDeExemplo(), Curso()), Curso());
    REQUIRE(lida.ok);
    CHECK(lida.dados.progresso.MelhorNota(0) == doctest::Approx(85.0f));
    CHECK(lida.dados.progresso.UltimaNota(0) == doctest::Approx(45.0f));
    CHECK(lida.dados.progresso.Aprovado(0));
}

TEST_CASE("Ficha: aluno sem nenhuma nota tambem fecha a ida e volta") {
    Ficha::Dados novo;
    novo.matricula = "1";
    const auto lida = Ficha::Desserializar(Ficha::Serializar(novo, Curso()), Curso());
    REQUIRE(lida.ok);
    CHECK(lida.dados.matricula == "1");
    CHECK(lida.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: o mesmo estado produz sempre o MESMO texto") {
    // Para dar para comparar dois saves num diff quando algo parecer errado.
    CHECK(Ficha::Serializar(FichaDeExemplo(), Curso()) == Ficha::Serializar(FichaDeExemplo(), Curso()));
}

TEST_CASE("Ficha: o texto gravado traz a versao do formato") {
    const auto texto = Ficha::Serializar(FichaDeExemplo(), Curso());
    CHECK(texto.find("\"versao\"") != std::string::npos);
    CHECK(texto.find("\"matricula\"") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Texto que nao serve
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: texto que nao e JSON e relatado, nao lancado") {
    const auto r = Ficha::Desserializar("{ isto nao fecha", Curso());
    CHECK_FALSE(r.ok);
    CHECK_FALSE(r.problemas.empty());
}

TEST_CASE("Ficha: arquivo vazio nao quebra") {
    CHECK_FALSE(Ficha::Desserializar("", Curso()).ok);
    CHECK_FALSE(Ficha::Desserializar("{}", Curso()).ok);
}

TEST_CASE("Ficha: sem matricula a ficha nao serve") {
    // Sem matricula nao da para saber de quem e o progresso.
    const auto r = Ficha::Desserializar(R"({ "versao": 2, "materias": [] })", Curso());
    CHECK_FALSE(r.ok);
    CHECK(Menciona(r.problemas, "matricula"));
}

TEST_CASE("Ficha: sem versao a ficha nao serve") {
    // Todo arquivo gravado por este jogo tem versao; um sem ela nao veio daqui.
    const auto r = Ficha::Desserializar(R"({ "matricula": "89384", "materias": [] })", Curso());
    CHECK_FALSE(r.ok);
    CHECK(Menciona(r.problemas, "versao"));
}

TEST_CASE("Ficha: versao do futuro e recusada com uma frase que explica") {
    // Melhor recusar do que ler errado: um formato mais novo pode ter mudado o
    // sentido de um campo que esta leitura ainda entende.
    const auto r = Ficha::Desserializar(R"({ "versao": 99, "matricula": "1", "materias": [] })", Curso());
    CHECK_FALSE(r.ok);
    CHECK(Menciona(r.problemas, "versao"));
}

// ---------------------------------------------------------------------------
// Materias com problema: descartadas uma a uma
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: uma materia estragada nao leva as outras") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [
        { "materia": "AAA", "recorde": 70, "retomada": 70 },
        { "recorde": 50, "retomada": 50 },
        { "materia": "DDD", "recorde": 90, "retomada": 90 }
      ]
    })", Curso());
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
      "versao": 2, "matricula": "89384",
      "materias": [
        { "materia": "AAA", "recorde": 101, "retomada": 50 },
        { "materia": "BBB", "recorde": 50, "retomada": -1 },
        { "materia": "CCC", "recorde": 80, "retomada": 80 }
      ]
    })", Curso());
    CHECK(r.ok);
    CHECK(r.dados.progresso.QuantasRegistradas() == 1);
    CHECK(r.dados.progresso.MelhorNota(2) == doctest::Approx(80.0f));
}

TEST_CASE("Ficha: zero e cem sao notas validas, nao extremos recusados") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [
        { "materia": "AAA", "recorde": 0, "retomada": 0 },
        { "materia": "BBB", "recorde": 100, "retomada": 100 }
      ]
    })", Curso());
    CHECK(r.ok);
    CHECK(r.problemas.empty());
    CHECK(r.dados.progresso.QuantasRegistradas() == 2);
}

TEST_CASE("Ficha: retomada acima do recorde e recusada") {
    // Estado impossivel: o recorde e o maximo de todas as notas, entao nenhuma
    // retomada pode passar dele. Se passasse, "melhor nota" deixaria de ser a
    // melhor nota e as regras de aprovacao ficariam erradas.
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [ { "materia": "AAA", "recorde": 50, "retomada": 80 } ]
    })", Curso());
    CHECK(r.dados.progresso.QuantasRegistradas() == 0);
    CHECK_FALSE(r.problemas.empty());
}

TEST_CASE("Ficha: materia repetida fica com a ultima e avisa") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [
        { "materia": "AAA", "recorde": 70, "retomada": 70 },
        { "materia": "AAA", "recorde": 90, "retomada": 90 }
      ]
    })", Curso());
    CHECK(r.dados.progresso.QuantasRegistradas() == 1);
    CHECK(Menciona(r.problemas, "repetida"));
}

TEST_CASE("Ficha: materia negativa e recusada") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [ { "materia": "NAOEXISTE", "recorde": 70, "retomada": 70 } ]
    })", Curso());
    CHECK(r.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: lista de materias ausente e um aluno sem nota, nao um erro") {
    // Aluno que se identificou e ainda nao jogou.
    const auto r = Ficha::Desserializar(R"({ "versao": 2, "matricula": "89384" })", Curso());
    CHECK(r.ok);
    CHECK(r.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: a matricula gravada e lida de volta igual") {
    for (const char* m : {"1", "89384", "115849", "999999"}) {
        CAPTURE(m);
        Ficha::Dados d;
        d.matricula = m;
        const auto r = Ficha::Desserializar(Ficha::Serializar(d, Curso()), Curso());
        REQUIRE(r.ok);
        CHECK(r.dados.matricula == m);
    }
}

// ---------------------------------------------------------------------------
// Migracao da versao 1 para a 2
//
// A versao 1 gravava a POSICAO da materia na lista; a 2 grava o CODIGO. Converter
// e procurar o codigo que estava naquela posicao. Enquanto houver save da versao 1
// no mundo, a ORDEM de materias.json nao pode mudar - ha um teste em
// test_arquivos_de_dados.cpp que trava isso e diz por que.
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: save da versao 1 e lido, convertendo posicao em codigo") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [
        { "materia": 0, "recorde": 85, "retomada": 45 },
        { "materia": 2, "recorde": 100, "retomada": 100 }
      ]
    })", Curso());

    REQUIRE(r.ok);
    // posicao 0 era "AAA" e posicao 2 era "CCC" no curso de teste
    CHECK(r.dados.progresso.MelhorNota(Curso().IndiceDe("AAA")) == doctest::Approx(85.0f));
    CHECK(r.dados.progresso.UltimaNota(Curso().IndiceDe("AAA")) == doctest::Approx(45.0f));
    CHECK(r.dados.progresso.MelhorNota(Curso().IndiceDe("CCC")) == doctest::Approx(100.0f));
}

TEST_CASE("Ficha: regravar um save antigo o deixa na versao atual") {
    const auto lido = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [ { "materia": 1, "recorde": 70, "retomada": 70 } ]
    })", Curso());
    REQUIRE(lido.ok);

    const std::string regravado = Ficha::Serializar(lido.dados, Curso());
    // Compara com kVersaoAtual, e nao com um numero escrito aqui: o formato vai
    // mudar de novo, e um literal faria este teste quebrar por isso em vez de por
    // um defeito de verdade.
    CHECK(regravado.find("\"versao\": " + std::to_string(Ficha::kVersaoAtual)) != std::string::npos);
    CHECK(regravado.find("\"materia\": \"BBB\"") != std::string::npos);
    CHECK(regravado.find("\"materia\": 1") == std::string::npos);
}

TEST_CASE("Ficha: posicao que nao existe mais num save antigo e descartada") {
    const auto r = Ficha::Desserializar(R"({
      "versao": 1, "matricula": "89384",
      "materias": [
        { "materia": 0, "recorde": 70, "retomada": 70 },
        { "materia": 99, "recorde": 80, "retomada": 80 }
      ]
    })", Curso());
    CHECK(r.ok);
    CHECK(r.dados.progresso.QuantasRegistradas() == 1);
    CHECK(Menciona(r.problemas, "nao existe mais"));
}

TEST_CASE("Ficha: na versao 2, materia que saiu do curso perde a nota") {
    // E o correto: guardar nota de materia que nao existe mais nao serve a
    // ninguem, e seria lixo que o ranking do professor teria de filtrar depois.
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [
        { "materia": "AAA", "recorde": 70, "retomada": 70 },
        { "materia": "APOSENTADA", "recorde": 90, "retomada": 90 }
      ]
    })", Curso());
    CHECK(r.ok);
    CHECK(r.dados.progresso.QuantasRegistradas() == 1);
    CHECK(Menciona(r.problemas, "APOSENTADA"));
}

TEST_CASE("Ficha: a NOTA sobrevive a uma reordenacao das materias") {
    // A propriedade que motivou esta versao inteira. Grava com uma ordem, le com
    // outra, e a nota continua na materia certa - que era exatamente o que o
    // formato antigo nao fazia.
    const auto antes = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "AAA", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "BBB", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");
    const auto depois = Materias::LerMaterias(R"({
      "materias": [
        { "codigo": "NOVA", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "BBB", "coluna": 0, "desbloqueio": { "tipo": "sempre" } },
        { "codigo": "AAA", "coluna": 0, "desbloqueio": { "tipo": "sempre" } }
      ]
    })");

    Ficha::Dados d;
    d.matricula = "89384";
    d.progresso.RegistrarNota(antes.IndiceDe("AAA"), 85.0f);

    const auto r = Ficha::Desserializar(Ficha::Serializar(d, antes), depois);
    REQUIRE(r.ok);
    CHECK(r.dados.progresso.MelhorNota(depois.IndiceDe("AAA")) == doctest::Approx(85.0f));
    CHECK(r.dados.progresso.MelhorNota(depois.IndiceDe("NOVA")) == doctest::Approx(0.0f));
    CHECK(r.dados.progresso.MelhorNota(depois.IndiceDe("BBB")) == doctest::Approx(0.0f));
}

// ---------------------------------------------------------------------------
// A data da ultima partida
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: a data da ultima partida sobrevive a ida e volta") {
    Ficha::Dados d;
    d.matricula = "89384";
    d.progresso.RegistrarNota(0, 70.0f, "2026-10-02 14:30:00");

    const auto r = Ficha::Desserializar(Ficha::Serializar(d, Curso()), Curso());
    REQUIRE(r.ok);
    REQUIRE(r.dados.progresso.Entradas().size() == 1);
    CHECK(r.dados.progresso.Entradas()[0].quando == "2026-10-02 14:30:00");
}

TEST_CASE("Ficha: a data e a da ULTIMA partida, nao a do recorde") {
    // E a pergunta que ela responde: "quem jogou esta semana". Jogar mal depois de
    // bem atualiza a data e nao o recorde.
    Progresso p;
    p.RegistrarNota(0, 90.0f, "2026-01-01 10:00:00");
    p.RegistrarNota(0, 40.0f, "2026-10-02 14:30:00");

    REQUIRE(p.Entradas().size() == 1);
    CHECK(p.Entradas()[0].recorde == doctest::Approx(90.0f));
    CHECK(p.Entradas()[0].quando == "2026-10-02 14:30:00");
}

TEST_CASE("Ficha: save sem data abre normal, e nao inventa uma") {
    // Todo save gravado antes deste campo existir.
    const auto r = Ficha::Desserializar(R"({
      "versao": 2, "matricula": "89384",
      "materias": [ { "materia": "AAA", "recorde": 70, "retomada": 70 } ]
    })", Curso());
    REQUIRE(r.ok);
    CHECK(r.problemas.empty());
    REQUIRE(r.dados.progresso.Entradas().size() == 1);
    CHECK(r.dados.progresso.Entradas()[0].quando.empty());
}

TEST_CASE("Ficha: uma nota sem data nao APAGA a data que ja havia") {
    Progresso p;
    p.RegistrarNota(0, 70.0f, "2026-10-02 14:30:00");
    p.RegistrarNota(0, 80.0f, "");
    CHECK(p.Entradas()[0].quando == "2026-10-02 14:30:00");
}

TEST_CASE("Ficha: entrada sem data nao escreve o campo vazio no arquivo") {
    Ficha::Dados d;
    d.matricula = "89384";
    d.progresso.RegistrarNota(0, 70.0f);
    CHECK(Ficha::Serializar(d, Curso()).find("quando") == std::string::npos);
}

TEST_CASE("Relogio: o formato ordena alfabeticamente na ordem cronologica") {
    // E o que permite ao ranking do professor ordenar por data sem converter nada.
    const std::string jan = Relogio::Formatar(1767225600);   // 2026-01-01, aproximado
    const std::string out = Relogio::Formatar(1790000000);   // bem depois
    REQUIRE_FALSE(jan.empty());
    REQUIRE_FALSE(out.empty());
    CHECK(jan < out);
}

TEST_CASE("Relogio: o formato tem o tamanho e os separadores esperados") {
    const std::string s = Relogio::Formatar(1790000000);
    REQUIRE(s.size() == 19);          // AAAA-MM-DD HH:MM:SS
    CHECK(s[4] == '-');
    CHECK(s[7] == '-');
    CHECK(s[10] == ' ');
    CHECK(s[13] == ':');
    CHECK(s[16] == ':');
}

TEST_CASE("Relogio: Agora devolve algo no mesmo formato") {
    const std::string s = Relogio::Agora();
    CHECK(s.size() == 19);
}
