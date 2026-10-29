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
#include "Ajudantes.h"

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
    CHECK(Ajudantes::UltimaNota(lida.dados.progresso, 0) == doctest::Approx(45.0f));
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
    CHECK(Ajudantes::UltimaNota(r.dados.progresso, Curso().IndiceDe("AAA")) == doctest::Approx(45.0f));
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

// ---------------------------------------------------------------------------
// Aparencia (versao 4 do formato)
// ---------------------------------------------------------------------------

namespace {

    Personagens::Aparencia AparenciaDeExemplo() {
        Personagens::Aparencia a;
        a.Definir("pele",   Personagens::Escolha{"corpo", "tom4"});
        a.Definir("cabelo", Personagens::Escolha{"longo", "ruivo"});
        a.Definir("camisa", Personagens::Escolha{"moletom", "cinza"});
        a.Definir("calca",  Personagens::Escolha{"bermuda", "jeans"});
        return a;
    }
}

TEST_CASE("Ficha: a aparencia sobrevive a ida e volta") {

    Ficha::Dados original = FichaDeExemplo();
    original.aparencia = AparenciaDeExemplo();

    const auto lida = Ficha::Desserializar(Ficha::Serializar(original, Curso()), Curso());

    REQUIRE(lida.ok);
    REQUIRE(lida.dados.aparencia.escolhas.size() == 4);
    CHECK(lida.dados.aparencia.Por("cabelo")->peca == "longo");
    CHECK(lida.dados.aparencia.Por("cabelo")->cor == "ruivo");
    CHECK(lida.dados.aparencia.Por("calca")->peca == "bermuda");
    CHECK(lida.dados.aparencia.Por("pele")->cor == "tom4");
}

TEST_CASE("Ficha: a ficha gravada hoje carrega a versao ATUAL") {

    // O NUMERO SAI DE Ficha::kVersaoAtual, e nao escrito a mao aqui.
    //
    // Estava escrito: "versao": 4. Ao subir para a 5 este teste quebrou - o que
    // foi sorte, porque ele quebrou pelo motivo ERRADO. Ele nao protegia nada:
    // era so uma segunda copia do numero, e a unica coisa que ele podia pegar
    // era a propria copia envelhecendo.
    //
    // Derivado, ele volta a ter um trabalho: garantir que o numero declarado no
    // cabecalho chega mesmo ao arquivo gravado.
    Ficha::Dados d = FichaDeExemplo();
    d.aparencia = AparenciaDeExemplo();

    const std::string esperado = "\"versao\": " + std::to_string(Ficha::kVersaoAtual);
    CHECK(Ficha::Serializar(d, Curso()).find(esperado) != std::string::npos);
}

TEST_CASE("Ficha: um save da versao 3 abre sem aparencia, e isso nao e erro") {

    // A MIGRACAO. Os saves que o jogo ja gravou durante o desenvolvimento sao
    // todos da versao 3; eles tem de continuar abrindo, com a aparencia vazia -
    // que quem compoe le como "use a padrao".
    const auto r = Ficha::Desserializar(R"({
      "versao": 3,
      "matricula": "89384",
      "materias": [ { "materia": "AAA", "recorde": 72.5, "retomada": 60 } ]
    })", Curso());

    REQUIRE(r.ok);
    CHECK(r.problemas.empty());
    CHECK(r.dados.aparencia.Vazia());
    CHECK(r.dados.progresso.MelhorNota(0) == doctest::Approx(72.5f));
}

TEST_CASE("Ficha: aparencia vazia nao escreve o campo no arquivo") {

    // Um objeto vazio no arquivo nao diria nada que a ausencia ja nao diga, e
    // ainda faria todo save de visitante parecer diferente de um da versao 3.
    const Ficha::Dados d = FichaDeExemplo();   // sem aparencia

    CHECK(Ficha::Serializar(d, Curso()).find("aparencia") == std::string::npos);
}

TEST_CASE("Ficha: a aparencia sai sempre na mesma ordem") {

    // Duas fichas iguais tem de dar o MESMO texto, senao dois saves do mesmo
    // estado nao dao para comparar num diff.
    Ficha::Dados a = FichaDeExemplo();
    Ficha::Dados b = FichaDeExemplo();

    a.aparencia.Definir("pele",   Personagens::Escolha{"corpo", "tom1"});
    a.aparencia.Definir("cabelo", Personagens::Escolha{"curto", "preto"});

    // A MESMA aparencia, inserida na ordem inversa.
    b.aparencia.Definir("cabelo", Personagens::Escolha{"curto", "preto"});
    b.aparencia.Definir("pele",   Personagens::Escolha{"corpo", "tom1"});

    CHECK(Ficha::Serializar(a, Curso()) == Ficha::Serializar(b, Curso()));
}

TEST_CASE("Ficha: escolha de aparencia malformada e descartada, e as outras ficam") {

    const auto r = Ficha::Desserializar(R"({
      "versao": 4,
      "matricula": "89384",
      "aparencia": {
        "pele":   { "peca": "corpo", "cor": "tom2" },
        "cabelo": { "peca": 123, "cor": "preto" },
        "camisa": { "peca": "regata" },
        "calca":  "nao sou objeto"
      },
      "materias": []
    })", Curso());

    REQUIRE(r.ok);
    CHECK(r.dados.aparencia.escolhas.size() == 1);
    CHECK(r.dados.aparencia.Por("pele")->cor == "tom2");
    CHECK(Menciona(r.problemas, "cabelo"));
    CHECK(Menciona(r.problemas, "camisa"));
    CHECK(Menciona(r.problemas, "calca"));
}

TEST_CASE("Ficha: \"aparencia\" que nao e objeto e ignorada, e o resto vale") {

    const auto r = Ficha::Desserializar(R"({
      "versao": 4,
      "matricula": "89384",
      "aparencia": "cabelo bonito",
      "materias": [ { "materia": "AAA", "recorde": 50, "retomada": 50 } ]
    })", Curso());

    REQUIRE(r.ok);
    CHECK(r.dados.aparencia.Vazia());
    CHECK(r.dados.progresso.MelhorNota(0) == doctest::Approx(50.0f));
    CHECK(Menciona(r.problemas, "aparencia"));
}

TEST_CASE("Ficha: a aparencia NAO e conferida contra catalogo nenhum") {

    // Esta camada transforma texto em dado e nada mais. Uma peca que saiu do
    // catalogo entra aqui como esta e e consertada por Personagens::Resolver na
    // hora de compor - que e onde o catalogo existe. Sem isso, trocar o catalogo
    // invalidaria saves.
    const auto r = Ficha::Desserializar(R"({
      "versao": 4,
      "matricula": "89384",
      "aparencia": { "cabelo": { "peca": "moicano-que-nunca-existiu", "cor": "neon" } },
      "materias": []
    })", Curso());

    REQUIRE(r.ok);
    CHECK(r.problemas.empty());
    CHECK(r.dados.aparencia.Por("cabelo")->peca == "moicano-que-nunca-existiu");
}

// ---------------------------------------------------------------------------
// A VERSAO 5: o registro das batalhas em dupla
// ---------------------------------------------------------------------------

TEST_CASE("Ficha: o registro de dupla vai e volta do arquivo") {

    Ficha::Dados d;
    d.matricula = "89384";

    std::vector<Progresso::Entrada> entradas(1);
    entradas[0].materia = 0;
    entradas[0].recorde = 55.0f;
    entradas[0].retomada = 88.0f;
    entradas[0].quando = "2026-10-14 15:00:00";
    entradas[0].recordeDeDupla = 88.0f;
    entradas[0].parceiroDoRecorde = "90112";
    entradas[0].quandoDaDupla = "2026-10-14 15:00:00";
    d.progresso.Restaurar(entradas);

    const Ficha::Lida lida = Ficha::Desserializar(Ficha::Serializar(d, Curso()), Curso());

    REQUIRE(lida.ok);
    CHECK(lida.dados.progresso.MelhorNotaSolo(0) == doctest::Approx(55.0f));
    CHECK(lida.dados.progresso.MelhorNotaDeDupla(0) == doctest::Approx(88.0f));
    CHECK(lida.dados.progresso.ParceiroDoRecorde(0) == "90112");
}

TEST_CASE("Ficha: quem nunca jogou em dupla nao ganha campos de dupla no arquivo") {

    // Mesma regra da aparencia: tres campos zerados num arquivo que ninguem vai
    // ler sao tres coisas para manter em dia por nada - e e o que faz um save
    // da versao 4 continuar abrindo sem conversao.
    Ficha::Dados d;
    d.matricula = "89384";

    std::vector<Progresso::Entrada> entradas(1);
    entradas[0].materia = 0;
    entradas[0].recorde = 70.0f;
    entradas[0].retomada = 70.0f;
    d.progresso.Restaurar(entradas);

    const std::string texto = Ficha::Serializar(d, Curso());

    CHECK(texto.find("recordeDupla") == std::string::npos);
    CHECK(texto.find("parceiro") == std::string::npos);
    CHECK(texto.find("quandoDupla") == std::string::npos);
}

TEST_CASE("Ficha: um save da versao 4 abre sem registro de dupla, e isso nao e erro") {

    // A MIGRACAO. Todo save ja gravado por este jogo e da versao 4 ou anterior,
    // e todos tem de continuar abrindo - sem registro de co-op, que e a verdade
    // sobre um aluno que nunca jogou acompanhado.
    const std::string daVersao4 = R"({
        "versao": 4,
        "matricula": "89384",
        "materias": [ { "materia": "AAA", "recorde": 72.5, "retomada": 68 } ]
    })";

    const Ficha::Lida lida = Ficha::Desserializar(daVersao4, Curso());

    REQUIRE(lida.ok);
    CHECK(lida.problemas.empty());
    CHECK(lida.dados.progresso.MelhorNotaSolo(0) == doctest::Approx(72.5f));
    CHECK(lida.dados.progresso.MelhorNotaDeDupla(0) == doctest::Approx(0.0f));
    CHECK(lida.dados.progresso.ParceiroDoRecorde(0).empty());
}

TEST_CASE("Ficha: retomada acima do recorde de SOLO deixou de ser impossivel") {

    // ESTE E O CASO QUE O CO-OP CRIOU, e que a regra antiga descartaria.
    //
    // A ficha recusava retomada > recorde como "estado impossivel jogando", e
    // era mesmo - com um modo so. Um aluno que tem 50 sozinho e faz 80 em dupla
    // fica com retomada 80 e recorde de solo 50, e a ficha dele e legitima.
    const std::string comDupla = R"({
        "versao": 5,
        "matricula": "89384",
        "materias": [ { "materia": "AAA", "recorde": 50, "retomada": 80,
                        "recordeDupla": 80, "parceiro": "90112" } ]
    })";

    const Ficha::Lida lida = Ficha::Desserializar(comDupla, Curso());

    REQUIRE(lida.ok);
    CHECK(lida.problemas.empty());
    CHECK(Ajudantes::UltimaNota(lida.dados.progresso, 0) == doctest::Approx(80.0f));
    CHECK(lida.dados.progresso.MelhorNotaSolo(0) == doctest::Approx(50.0f));
}

TEST_CASE("Ficha: retomada acima dos DOIS recordes continua sendo descartada") {

    // A regra nao sumiu, so passou a olhar os dois numeros. Uma retomada acima
    // de ambos continua impossivel jogando, e deixar passar faria "melhor nota"
    // mentir.
    const std::string impossivel = R"({
        "versao": 5,
        "matricula": "89384",
        "materias": [ { "materia": "AAA", "recorde": 50, "retomada": 95,
                        "recordeDupla": 80 } ]
    })";

    const Ficha::Lida lida = Ficha::Desserializar(impossivel, Curso());

    CHECK(lida.ok);
    CHECK_FALSE(lida.problemas.empty());
    CHECK(lida.dados.progresso.QuantasRegistradas() == 0);
}

TEST_CASE("Ficha: nota de dupla fora da faixa derruba so o registro de dupla") {

    // A materia inteira nao se perde por causa de um campo opcional estragado:
    // o recorde de solo e a retomada sao bons e continuam valendo.
    const std::string estragado = R"({
        "versao": 5,
        "matricula": "89384",
        "materias": [ { "materia": "AAA", "recorde": 70, "retomada": 70,
                        "recordeDupla": 450 } ]
    })";

    const Ficha::Lida lida = Ficha::Desserializar(estragado, Curso());

    REQUIRE(lida.ok);
    CHECK_FALSE(lida.problemas.empty());
    CHECK(lida.dados.progresso.MelhorNotaSolo(0) == doctest::Approx(70.0f));
    CHECK(lida.dados.progresso.MelhorNotaDeDupla(0) == doctest::Approx(0.0f));
}

TEST_CASE("Ficha: parceiro sem recorde de dupla e ignorado") {

    // Sem recorde, o nome seria o de alguem com quem nunca se jogou aquilo.
    const std::string solto = R"({
        "versao": 5,
        "matricula": "89384",
        "materias": [ { "materia": "AAA", "recorde": 70, "retomada": 70,
                        "parceiro": "90112", "quandoDupla": "2026-10-14 15:00:00" } ]
    })";

    const Ficha::Lida lida = Ficha::Desserializar(solto, Curso());

    REQUIRE(lida.ok);
    CHECK(lida.dados.progresso.ParceiroDoRecorde(0).empty());
}
