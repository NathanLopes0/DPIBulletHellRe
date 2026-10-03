// Leitura dos tipos de projetil a partir de texto JSON. Camada pura: nao abre
// arquivo, nao escreve log, nao sobe SDL e nao constroi Actor nenhum.
//
// Estes testes cobrem o LEITOR. A conferencia de que o arquivo de verdade
// descreve os projeteis certos, e de que os quadros existem nos atlas de
// verdade, esta em test_arquivos_de_dados.cpp.

#include "doctest.h"

#include <string>

#include "../Source/Attacks/ProjeteisDeChefe.h"

// Uma entrada minima e valida, para os casos mexerem em uma coisa de cada vez.
static const char* kValido = R"({
  "salles": {
    "Capivara": {
      "sprite": "Teachers/Projectiles/DPIBHSallesCapivara.png",
      "dados":  "Teachers/Projectiles/DPIBHSallesCapivara.json",
      "animacoes": { "Normal": [0], "Homing": [1] },
      "animacaoInicial": "Normal",
      "ordemDeDesenho": 90
    }
  }
})";

/// Se algum problema relatado contem o trecho. Usado em vez de comparar a frase
/// inteira: o teste nao deve quebrar porque a mensagem ficou mais clara.
static bool Menciona(const std::vector<std::string>& problemas, const std::string& trecho) {
    for (const auto& p : problemas) {
        if (p.find(trecho) != std::string::npos) return true;
    }
    return false;
}

/// Monta um arquivo de um projetil so, com o corpo dado. Encurta os casos.
static std::string Um(const std::string& corpo) {
    return "{ \"salles\": { \"P\": " + corpo + " } }";
}

// ---------------------------------------------------------------------------
// Leitura do caso bom
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: le uma entrada completa") {
    const auto r = LerProjeteis(kValido);
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.count("salles") == 1);
    REQUIRE(r.conjuntos.at("salles").count("Capivara") == 1);

    const auto& p = r.conjuntos.at("salles").at("Capivara");
    CHECK(p.sprite == "Teachers/Projectiles/DPIBHSallesCapivara.png");
    CHECK(p.dados  == "Teachers/Projectiles/DPIBHSallesCapivara.json");
    CHECK(p.animacaoInicial == "Normal");
    CHECK(p.ordemDeDesenho == 90);
    REQUIRE(p.animacoes.size() == 2);
}

TEST_CASE("Projeteis: os padroes sao os da classe, nao zeros") {
    // Um campo omitido tem de sair com o valor que o C++ usava antes da
    // migracao. Zero em qualquer destes seria um projetil invisivel, sem hitbox
    // ou atras do cenario.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "Normal": [0] }
    })"));
    REQUIRE(r.problemas.empty());
    const auto& p = r.conjuntos.at("salles").at("P");
    CHECK(p.escala == doctest::Approx(1.0f));
    CHECK(p.ordemDeDesenho == 100);
    CHECK(p.colisorDimensao == "largura");
    CHECK(p.colisorDivisor == doctest::Approx(2.0f));
    CHECK(p.posicionarNoDono == true);
}

TEST_CASE("Projeteis: as animacoes guardam a ordem dos quadros") {
    // A lista e uma sequencia de tempo, nao um conjunto: inverter a ordem muda a
    // animacao, e repetir um indice e legitimo (ida e volta).
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "Vai": [0, 1, 2, 1] }
    })"));
    REQUIRE(r.problemas.empty());
    const auto& anims = r.conjuntos.at("salles").at("P").animacoes;
    REQUIRE(anims.size() == 1);
    CHECK(anims[0].nome == "Vai");
    REQUIRE(anims[0].quadros.size() == 4);
    CHECK(anims[0].quadros[0] == 0);
    CHECK(anims[0].quadros[1] == 1);
    CHECK(anims[0].quadros[2] == 2);
    CHECK(anims[0].quadros[3] == 1);
}

TEST_CASE("Projeteis: dois conjuntos podem ter projeteis de mesmo nome") {
    // E a razao de haver conjunto por chefe em vez de um mapa plano.
    const auto r = LerProjeteis(R"({
      "salles": { "Normal": { "sprite": "s.png", "dados": "s.json",
                              "animacoes": { "A": [0] } } },
      "julio":  { "Normal": { "sprite": "j.png", "dados": "j.json",
                              "animacoes": { "A": [0] } } }
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.conjuntos.size() == 2);
    CHECK(r.conjuntos.at("salles").at("Normal").sprite == "s.png");
    CHECK(r.conjuntos.at("julio").at("Normal").sprite  == "j.png");
}

TEST_CASE("Projeteis: aceita comentario, como os outros arquivos de dados") {
    const auto r = LerProjeteis(R"({
      // por que este projetil e assim
      "salles": { "P": { "sprite": "a.png", "dados": "a.json",
                         "animacoes": { "A": [0] } } }
    })");
    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("salles").count("P") == 1);
}

// ---------------------------------------------------------------------------
// animacaoInicial: o campo que uma das fabricas antigas esquecia
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: animacaoInicial omitida vira a primeira animacao") {
    // Este e o bug que a fabrica dos baloes tinha: registrava tres animacoes e
    // nao escolhia nenhuma, entao quem nascia pelo Prewarm era desenhado com
    // animacao vazia. O leitor resolve o padrao para que a ponte nao precise.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "Ba": [1], "Aa": [0] }
    })"));
    REQUIRE(r.problemas.empty());
    const auto& p = r.conjuntos.at("salles").at("P");
    CHECK_FALSE(p.animacaoInicial.empty());
    // "Primeira" e a primeira da lista lida, e o nlohmann ordena as chaves de um
    // objeto por nome - "Aa" antes de "Ba".
    CHECK(p.animacaoInicial == "Aa");
    CHECK(p.animacaoInicial == p.animacoes.front().nome);
}

TEST_CASE("Projeteis: animacaoInicial que nao existe e relatada e corrigida") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "Normal": [0] },
        "animacaoInicial": "Noraml"
    })"));
    CHECK(Menciona(r.problemas, "Noraml"));
    // Corrigida, e nao descartada: um erro de digitacao aqui nao deve apagar o
    // projetil inteiro do chefe.
    REQUIRE(r.conjuntos.at("salles").count("P") == 1);
    CHECK(r.conjuntos.at("salles").at("P").animacaoInicial == "Normal");
}

// ---------------------------------------------------------------------------
// Problemas GRAVES: a entrada e descartada
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: sem sprite a entrada cai") {
    const auto r = LerProjeteis(Um(R"({ "dados": "a.json", "animacoes": { "A": [0] } })"));
    CHECK(Menciona(r.problemas, "sprite"));
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: sem dados a entrada cai") {
    const auto r = LerProjeteis(Um(R"({ "sprite": "a.png", "animacoes": { "A": [0] } })"));
    CHECK(Menciona(r.problemas, "dados"));
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: sem animacao a entrada cai") {
    // Sem animacao nao ha o que desenhar, e um projetil invisivel que mata e
    // pior que um projetil ausente.
    const auto r = LerProjeteis(Um(R"({ "sprite": "a.png", "dados": "a.json" })"));
    CHECK(Menciona(r.problemas, "animacoes"));
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: animacao de lista vazia nao conta como animacao") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [] }
    })"));
    CHECK(Menciona(r.problemas, "vazia"));
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: quadro negativo derruba a animacao") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0, -1] }
    })"));
    CHECK(Menciona(r.problemas, "negativo"));
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: quadro fracionario e recusado") {
    // 1.5 nao e indice de quadro, e truncar silenciosamente esconderia o erro.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0, 1.5] }
    })"));
    CHECK(Menciona(r.problemas, "inteiros"));
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: uma entrada ruim nao leva as boas do mesmo chefe") {
    const auto r = LerProjeteis(R"({
      "salles": {
        "Bom":  { "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] } },
        "Ruim": { "sprite": "b.png", "animacoes": { "A": [0] } }
      }
    })");
    CHECK_FALSE(r.problemas.empty());
    REQUIRE(r.conjuntos.count("salles") == 1);
    CHECK(r.conjuntos.at("salles").count("Bom") == 1);
    CHECK(r.conjuntos.at("salles").count("Ruim") == 0);
}

// ---------------------------------------------------------------------------
// Problemas LEVES: relatados, campo volta ao padrao
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: escala zero ou negativa volta para 1") {
    // Escala zero e um projetil de hitbox zero: atravessa o jogador sem tocar.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "escala": 0
    })"));
    CHECK(Menciona(r.problemas, "escala"));
    REQUIRE(r.conjuntos.at("salles").count("P") == 1);
    CHECK(r.conjuntos.at("salles").at("P").escala == doctest::Approx(1.0f));
}

TEST_CASE("Projeteis: divisor zero volta para 2 em vez de dividir por zero") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "colisorDivisor": 0
    })"));
    CHECK(Menciona(r.problemas, "colisorDivisor"));
    CHECK(r.conjuntos.at("salles").at("P").colisorDivisor == doctest::Approx(2.0f));
}

TEST_CASE("Projeteis: dimensao desconhecida volta para largura") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "colisorDimensao": "diagonal"
    })"));
    CHECK(Menciona(r.problemas, "diagonal"));
    CHECK(r.conjuntos.at("salles").at("P").colisorDimensao == "largura");
}

TEST_CASE("Projeteis: altura e uma dimensao valida") {
    // O projetil de lista duplamente encadeada e mais largo que alto, e usar a
    // largura dele daria uma hitbox bem maior que o desenho.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "colisorDimensao": "altura"
    })"));
    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("salles").at("P").colisorDimensao == "altura");
    CHECK(DimensaoDeColisorExiste("altura"));
    CHECK(DimensaoDeColisorExiste("largura"));
    CHECK_FALSE(DimensaoDeColisorExiste("Largura"));
    CHECK_FALSE(DimensaoDeColisorExiste(""));
}

TEST_CASE("Projeteis: a margem de saida tem como padrao a regra geral do jogo") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] }
    })"));
    REQUIRE(r.problemas.empty());
    const auto& p = r.conjuntos.at("salles").at("P");
    CHECK(p.margemEmSprites == doctest::Approx(1.0f));
    CHECK(p.margemDivisorDeTela == doctest::Approx(12.0f));
}

TEST_CASE("Projeteis: divisor de tela ZERO e valido, nao um erro") {
    // E o que os baloes do Andre usam: a folga e so o tamanho do sprite, sem
    // parcela proporcional a tela. Tratar zero como invalido reescreveria o
    // comportamento deles na migracao.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] },
        "margemEmSprites": 2, "margemDivisorDeTela": 0
    })"));
    CHECK(r.problemas.empty());
    const auto& p = r.conjuntos.at("salles").at("P");
    CHECK(p.margemEmSprites == doctest::Approx(2.0f));
    CHECK(p.margemDivisorDeTela == doctest::Approx(0.0f));
}

TEST_CASE("Projeteis: margem negativa e recusada") {
    // Uma folga negativa encolheria a area viva para DENTRO da tela, e o projetil
    // morreria a vista do jogador - parece um tiro que desaparece sozinho.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] },
        "margemEmSprites": -1, "margemDivisorDeTela": -5
    })"));
    CHECK(Menciona(r.problemas, "margemEmSprites"));
    CHECK(Menciona(r.problemas, "margemDivisorDeTela"));
    const auto& p = r.conjuntos.at("salles").at("P");
    CHECK(p.margemEmSprites == doctest::Approx(1.0f));
    CHECK(p.margemDivisorDeTela == doctest::Approx(12.0f));
}

TEST_CASE("Projeteis: posicionarNoDono pode ser desligado") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "posicionarNoDono": false
    })"));
    CHECK(r.problemas.empty());
    CHECK(r.conjuntos.at("salles").at("P").posicionarNoDono == false);
}

// ---------------------------------------------------------------------------
// Texto que nao serve
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: texto que nao e JSON e relatado, nao lancado") {
    const auto r = LerProjeteis("{ isto nao fecha");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.empty());
}

TEST_CASE("Projeteis: arquivo vazio nao quebra") {
    CHECK(LerProjeteis("").conjuntos.empty());
    CHECK(LerProjeteis("{}").conjuntos.empty());
}

TEST_CASE("Projeteis: raiz que nao e objeto e relatada") {
    const auto r = LerProjeteis("[1, 2, 3]");
    CHECK(Menciona(r.problemas, "raiz"));
    CHECK(r.conjuntos.empty());
}

TEST_CASE("Projeteis: conjunto que nao e objeto e relatado") {
    const auto r = LerProjeteis(R"({ "salles": 7 })");
    CHECK(Menciona(r.problemas, "salles"));
    CHECK(r.conjuntos.empty());
}

TEST_CASE("Projeteis: conjunto que ficou sem projetil util nao entra no mapa") {
    // Melhor nao existir que existir vazio: a ponte distingue "o chefe nao tem
    // conjunto" de "o conjunto tem zero projeteis" e as duas mensagens de erro
    // seriam a mesma.
    const auto r = LerProjeteis(R"({ "salles": { "P": { "sprite": "a.png" } } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.conjuntos.count("salles") == 0);
}

TEST_CASE("Projeteis: por padrao o sprite NAO gira com a velocidade") {

    // Girar so faz sentido para arte que tem frente. A capivara do Salles e os
    // baloes do Andre girariam a toa, entao quem nao pede fica parado.
    const auto r = LerProjeteis(R"({ "salles": { "P": {
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] }
    } } })");

    REQUIRE(r.conjuntos.count("salles") == 1);
    CHECK_FALSE(r.conjuntos.at("salles").at("P").rotacionarComAVelocidade);
}

TEST_CASE("Projeteis: rotacionarComAVelocidade pode ser ligado") {

    const auto r = LerProjeteis(R"({ "salles": { "P": {
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] },
        "rotacionarComAVelocidade": true
    } } })");

    REQUIRE(r.conjuntos.count("salles") == 1);
    CHECK(r.conjuntos.at("salles").at("P").rotacionarComAVelocidade);
    CHECK(r.problemas.empty());
}
