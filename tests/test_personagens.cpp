// O catalogo de pecas da personagem, e o que ele faz com uma aparencia.
//
// Testes PUROS: montam um catalogo pequeno em memoria. Que o personagens.json
// de verdade esteja certo e pergunta de test_arquivos_de_dados.cpp.

#include "doctest.h"

#include <string>

#include "../Source/Personagens.h"

using namespace Personagens;

namespace {

    /// Um catalogo minimo, com duas categorias e uma camada fixa no meio delas.
    std::string CatalogoBasico() {
        return R"({
          "fixas": [ { "arte": "fixa/olho", "ordem": 20 } ],
          "categorias": [
            {
              "id": "pele", "nome": "Pele", "ordem": 10,
              "pecas": [ { "id": "corpo", "nome": "Corpo", "arte": "arte/corpo" } ],
              "cores": [ { "id": "claro", "nome": "Claro", "rgb": "ffcc99" },
                         { "id": "escuro", "nome": "Escuro", "rgb": "553322" } ]
            },
            {
              "id": "cabelo", "nome": "Cabelo", "ordem": 30,
              "pecas": [ { "id": "curto", "nome": "Curto", "arte": "arte/curto" },
                         { "id": "longo", "nome": "Longo", "arte": "arte/longo" } ],
              "cores": [ { "id": "preto", "nome": "Preto", "rgb": "222222" },
                         { "id": "loiro", "nome": "Loiro", "rgb": "ddbb66" } ]
            }
          ]
        })";
    }

    bool Menciona(const std::vector<std::string>& problemas, const std::string& trecho) {
        for (const auto& p : problemas) {
            if (p.find(trecho) != std::string::npos) return true;
        }
        return false;
    }
}

// ---------------------------------------------------------------- basico

TEST_CASE("Personagens: um catalogo valido entra inteiro") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    CHECK(c.problemas.empty());
    REQUIRE(c.categorias.size() == 2);
    CHECK(c.Por("pele") != nullptr);
    CHECK(c.Por("cabelo") != nullptr);
    CHECK(c.Por("barba") == nullptr);
    REQUIRE(c.fixas.size() == 1);
    CHECK(c.fixas[0].arte == "fixa/olho");
}

TEST_CASE("Personagens: a aparencia padrao e a primeira peca e a primeira cor") {

    const Catalogo c = LerCatalogo(CatalogoBasico());
    const Aparencia a = c.Padrao();

    REQUIRE(a.Por("pele") != nullptr);
    CHECK(a.Por("pele")->peca == "corpo");
    CHECK(a.Por("pele")->cor == "claro");

    REQUIRE(a.Por("cabelo") != nullptr);
    CHECK(a.Por("cabelo")->peca == "curto");
    CHECK(a.Por("cabelo")->cor == "preto");
}

// ---------------------------------------------------------------- cores

TEST_CASE("Personagens: cor hexadecimal com e sem cerquilha") {

    unsigned char r = 0, g = 0, b = 0;

    REQUIRE(LerCorHex("ff8000", r, g, b));
    CHECK(r == 255); CHECK(g == 128); CHECK(b == 0);

    // Com "#" tambem vale: quem edita o arquivo a mao escreve das duas formas.
    REQUIRE(LerCorHex("#00FF10", r, g, b));
    CHECK(r == 0); CHECK(g == 255); CHECK(b == 16);

    CHECK_FALSE(LerCorHex("ff80", r, g, b));        // curto demais
    CHECK_FALSE(LerCorHex("ff80000", r, g, b));     // comprido demais
    CHECK_FALSE(LerCorHex("gg0000", r, g, b));      // nao e hexadecimal
    CHECK_FALSE(LerCorHex("", r, g, b));
}

TEST_CASE("Personagens: cor com rgb invalido e descartada, e as outras ficam") {

    const Catalogo c = LerCatalogo(R"({
      "categorias": [ {
        "id": "cabelo", "ordem": 1,
        "pecas": [ { "id": "curto", "arte": "a" } ],
        "cores": [ { "id": "bom", "rgb": "112233" },
                   { "id": "ruim", "rgb": "naoehex" },
                   { "id": "outro", "rgb": "445566" } ]
      } ]
    })");

    REQUIRE(c.categorias.size() == 1);
    CHECK(c.categorias[0].cores.size() == 2);
    CHECK(c.categorias[0].CorPor("ruim") == nullptr);
    CHECK(Menciona(c.problemas, "rgb"));
}

// ---------------------------------------------------------------- descartes

TEST_CASE("Personagens: categoria sem peca ou sem cor e descartada inteira") {

    // Nao daria para escolher nada nela, e na tela seria uma linha que nao
    // responde a nada.
    const Catalogo c = LerCatalogo(R"({
      "categorias": [
        { "id": "semcor", "ordem": 1,
          "pecas": [ { "id": "x", "arte": "a" } ], "cores": [] },
        { "id": "sempeca", "ordem": 2,
          "pecas": [], "cores": [ { "id": "y", "rgb": "112233" } ] },
        { "id": "boa", "ordem": 3,
          "pecas": [ { "id": "z", "arte": "a" } ],
          "cores": [ { "id": "w", "rgb": "112233" } ] }
      ]
    })");

    REQUIRE(c.categorias.size() == 1);
    CHECK(c.categorias[0].id == "boa");
}

TEST_CASE("Personagens: peca sem arte e descartada") {

    const Catalogo c = LerCatalogo(R"({
      "categorias": [ {
        "id": "cabelo", "ordem": 1,
        "pecas": [ { "id": "bom", "arte": "a" }, { "id": "mudo" } ],
        "cores": [ { "id": "c", "rgb": "112233" } ]
      } ]
    })");

    REQUIRE(c.categorias.size() == 1);
    CHECK(c.categorias[0].pecas.size() == 1);
    CHECK(Menciona(c.problemas, "arte"));
}

TEST_CASE("Personagens: id repetido fica com o primeiro") {

    const Catalogo c = LerCatalogo(R"({
      "categorias": [ {
        "id": "cabelo", "ordem": 1,
        "pecas": [ { "id": "curto", "arte": "primeiro" },
                   { "id": "curto", "arte": "segundo" } ],
        "cores": [ { "id": "c", "rgb": "112233" } ]
      } ]
    })");

    REQUIRE(c.categorias.size() == 1);
    REQUIRE(c.categorias[0].pecas.size() == 1);
    CHECK(c.categorias[0].pecas[0].arte == "primeiro");
    CHECK(Menciona(c.problemas, "duas vezes"));
}

TEST_CASE("Personagens: ordem de desenho repetida e relatada") {

    // Nao descarta - so avisa. Mas sem o aviso, qual camada fica por cima vira
    // detalhe de implementacao em vez de decisao de quem escreveu o arquivo.
    const Catalogo c = LerCatalogo(R"({
      "fixas": [ { "arte": "f", "ordem": 10 } ],
      "categorias": [
        { "id": "a", "ordem": 10, "pecas": [ { "id": "p", "arte": "x" } ],
          "cores": [ { "id": "c", "rgb": "112233" } ] },
        { "id": "b", "ordem": 20, "pecas": [ { "id": "p", "arte": "y" } ],
          "cores": [ { "id": "c", "rgb": "112233" } ] }
      ]
    })");

    CHECK(c.categorias.size() == 2);
    CHECK(Menciona(c.problemas, "mesma ordem"));
}

// ---------------------------------------------------------------- Resolver

TEST_CASE("Personagens: uma aparencia valida passa intacta por Resolver") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia pedida;
    pedida.Definir("pele", Escolha{"corpo", "escuro"});
    pedida.Definir("cabelo", Escolha{"longo", "loiro"});

    std::vector<std::string> trocas;
    const Aparencia r = c.Resolver(pedida, &trocas);

    CHECK(trocas.empty());
    CHECK(r.Por("pele")->cor == "escuro");
    CHECK(r.Por("cabelo")->peca == "longo");
    CHECK(r.Por("cabelo")->cor == "loiro");
}

TEST_CASE("Personagens: peca que sumiu cai no padrao SEM levar a cor junto") {

    // E a regra que importa (RNF3): a aparencia conserta camada por camada, e
    // dentro da camada, metade por metade. Se o tipo de cabelo sai do catalogo,
    // a cor que o aluno escolheu continua valendo.
    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia pedida;
    pedida.Definir("pele", Escolha{"corpo", "escuro"});
    pedida.Definir("cabelo", Escolha{"moicano", "loiro"});   // "moicano" nao existe

    std::vector<std::string> trocas;
    const Aparencia r = c.Resolver(pedida, &trocas);

    CHECK(r.Por("cabelo")->peca == "curto");    // caiu no padrao
    CHECK(r.Por("cabelo")->cor == "loiro");     // mas a cor ficou
    CHECK(r.Por("pele")->cor == "escuro");      // e a outra camada nem foi tocada
    CHECK(Menciona(trocas, "moicano"));
}

TEST_CASE("Personagens: cor que sumiu cai no padrao SEM levar a peca junto") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia pedida;
    pedida.Definir("cabelo", Escolha{"longo", "verde"});   // "verde" nao existe

    std::vector<std::string> trocas;
    const Aparencia r = c.Resolver(pedida, &trocas);

    CHECK(r.Por("cabelo")->peca == "longo");    // a peca ficou
    CHECK(r.Por("cabelo")->cor == "preto");     // so a cor caiu
    CHECK(Menciona(trocas, "verde"));
}

TEST_CASE("Personagens: categoria ausente na aparencia cai no padrao, calada") {

    // E o caso de um save gravado antes de a categoria existir. Nao ha nada de
    // errado nisso, entao nao vira problema relatado.
    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia pedida;
    pedida.Definir("cabelo", Escolha{"longo", "loiro"});   // nao fala de pele

    std::vector<std::string> trocas;
    const Aparencia r = c.Resolver(pedida, &trocas);

    REQUIRE(r.Por("pele") != nullptr);
    CHECK(r.Por("pele")->peca == "corpo");
    CHECK(r.Por("pele")->cor == "claro");
    CHECK(trocas.empty());
}

TEST_CASE("Personagens: escolha de categoria que nao existe mais e descartada") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia pedida = c.Padrao();
    pedida.Definir("chapeu", Escolha{"cartola", "preto"});   // categoria aposentada

    std::vector<std::string> trocas;
    const Aparencia r = c.Resolver(pedida, &trocas);

    CHECK(r.Por("chapeu") == nullptr);
    CHECK(r.escolhas.size() == 2);
    CHECK(Menciona(trocas, "chapeu"));
}

TEST_CASE("Personagens: Resolver de uma aparencia vazia da a padrao") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    const Aparencia r = c.Resolver(Aparencia{});
    const Aparencia padrao = c.Padrao();

    REQUIRE(r.escolhas.size() == padrao.escolhas.size());
    for (const auto& par : padrao.escolhas) {
        REQUIRE(r.Por(par.first) != nullptr);
        CHECK(r.Por(par.first)->peca == par.second.peca);
        CHECK(r.Por(par.first)->cor == par.second.cor);
    }
}

// ---------------------------------------------------------------- Camadas

TEST_CASE("Personagens: as camadas saem em ordem de desenho, com as fixas no meio") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia a;
    a.Definir("pele", Escolha{"corpo", "escuro"});
    a.Definir("cabelo", Escolha{"longo", "loiro"});

    const std::vector<Camada> camadas = c.Camadas(a);

    REQUIRE(camadas.size() == 3);

    // pele (10), olho fixo (20), cabelo (30)
    CHECK(camadas[0].arte == "arte/corpo");
    CHECK(camadas[1].arte == "fixa/olho");
    CHECK(camadas[2].arte == "arte/longo");

    for (size_t i = 1; i < camadas.size(); ++i) {
        CHECK(camadas[i - 1].ordem <= camadas[i].ordem);
    }
}

TEST_CASE("Personagens: a camada fixa nao e tingida, e as outras sao") {

    const Catalogo c = LerCatalogo(CatalogoBasico());
    const std::vector<Camada> camadas = c.Camadas(c.Padrao());

    REQUIRE(camadas.size() == 3);
    CHECK(camadas[0].tingida);
    CHECK_FALSE(camadas[1].tingida);
    CHECK(camadas[2].tingida);
}

TEST_CASE("Personagens: a cor da camada e a cor escolhida") {

    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia a;
    a.Definir("pele", Escolha{"corpo", "escuro"});    // 553322
    const std::vector<Camada> camadas = c.Camadas(a);

    REQUIRE(!camadas.empty());
    CHECK(camadas[0].r == 0x55);
    CHECK(camadas[0].g == 0x33);
    CHECK(camadas[0].b == 0x22);
}

TEST_CASE("Personagens: Camadas conserta sozinha uma aparencia estragada") {

    // Quem desenha nao precisa lembrar de chamar Resolver antes: uma aparencia
    // lida do disco serve direto.
    const Catalogo c = LerCatalogo(CatalogoBasico());

    Aparencia a;
    a.Definir("cabelo", Escolha{"moicano", "roxo"});   // nada disso existe

    const std::vector<Camada> camadas = c.Camadas(a);

    REQUIRE(camadas.size() == 3);
    CHECK(camadas[2].arte == "arte/curto");
}

// ---------------------------------------------------------------- prontas

TEST_CASE("Personagens: combinacoes prontas entram ja resolvidas") {

    const Catalogo c = LerCatalogo(R"({
      "categorias": [
        { "id": "cabelo", "ordem": 10,
          "pecas": [ { "id": "curto", "arte": "a" }, { "id": "longo", "arte": "b" } ],
          "cores": [ { "id": "preto", "rgb": "222222" } ] }
      ],
      "predefinidas": [
        { "id": "ok",   "escolhas": { "cabelo": { "peca": "longo", "cor": "preto" } } },
        { "id": "ruim", "escolhas": { "cabelo": { "peca": "moicano", "cor": "preto" } } }
      ]
    })");

    REQUIRE(c.predefinidas.size() == 2);
    CHECK(c.predefinidas[0].aparencia.Por("cabelo")->peca == "longo");

    // A que citava peca inexistente nao e perdida: vira valida, e o problema
    // aparece na leitura em vez de so quando alguem a escolher.
    CHECK(c.predefinidas[1].aparencia.Por("cabelo")->peca == "curto");
    CHECK(Menciona(c.problemas, "moicano"));
}

// ---------------------------------------------------------------- entrada ruim

TEST_CASE("Personagens: texto que nao e JSON nao derruba nada") {

    const Catalogo c = LerCatalogo("isto nao e json {{{");

    CHECK(c.Vazio());
    CHECK_FALSE(c.problemas.empty());
}

TEST_CASE("Personagens: catalogo sem categorias e relatado") {

    const Catalogo c = LerCatalogo(R"({ "fixas": [] })");

    CHECK(c.Vazio());
    CHECK(Menciona(c.problemas, "categorias"));
}

TEST_CASE("Personagens: identificador so aceita letra, numero, hifen e sublinhado") {

    CHECK(IdServe("curto"));
    CHECK(IdServe("tom3"));
    CHECK(IdServe("cabelo_longo"));
    CHECK(IdServe("meio-termo"));

    CHECK_FALSE(IdServe(""));
    CHECK_FALSE(IdServe("com espaco"));
    CHECK_FALSE(IdServe("acento/barra"));
}

TEST_CASE("Personagens: Padrao de um catalogo vazio nao quebra") {

    const Catalogo c;
    const Aparencia a = c.Padrao();
    CHECK(a.Vazia());
    CHECK(c.Camadas(a).empty());
}
