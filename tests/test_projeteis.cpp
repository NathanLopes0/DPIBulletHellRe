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
  "Capivara": {
    "sprite": "Teachers/Projectiles/DPIBHSallesCapivara.png",
    "dados":  "Teachers/Projectiles/DPIBHSallesCapivara.json",
    "animacoes": { "Normal": [0], "Homing": [1] },
    "animacaoInicial": "Normal",
    "ordemDeDesenho": 90
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
    return "{ \"P\": " + corpo + " }";
}

// ---------------------------------------------------------------------------
// Leitura do caso bom
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: le uma entrada completa") {
    const auto r = LerProjeteis(kValido);
    CHECK(r.problemas.empty());
    REQUIRE(r.projeteis.count("Capivara") == 1);

    const auto& p = r.projeteis.at("Capivara");
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
    const auto& p = r.projeteis.at("P");
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
    const auto& anims = r.projeteis.at("P").animacoes;
    REQUIRE(anims.size() == 1);
    CHECK(anims[0].nome == "Vai");
    REQUIRE(anims[0].quadros.size() == 4);
    CHECK(anims[0].quadros[0] == 0);
    CHECK(anims[0].quadros[1] == 1);
    CHECK(anims[0].quadros[2] == 2);
    CHECK(anims[0].quadros[3] == 1);
}

TEST_CASE("Projeteis: a raiz e o mapa de projeteis do chefe, sem nivel a mais") {
    // DOIS CHEFES PODEM TER UM PROJETIL "Normal" sem colidir, e quem garante
    // isso agora e a PASTA: cada um le o proprio arquivo e nunca enxerga o do
    // outro. Antes havia aqui dentro um nivel com o nome do chefe - a mesma
    // informacao que a pasta, e livre para discordar dela.
    const auto r = LerProjeteis(R"({
      "Normal": { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] } },
      "Outro":  { "sprite": "o.png", "dados": "o.json", "animacoes": { "A": [0] } }
    })");
    CHECK(r.problemas.empty());
    REQUIRE(r.projeteis.size() == 2);
    CHECK(r.projeteis.at("Normal").sprite == "s.png");
    CHECK(r.projeteis.at("Outro").sprite  == "o.png");
}

TEST_CASE("Projeteis: aceita comentario, como os outros arquivos de dados") {
    const auto r = LerProjeteis(R"({
      // por que este projetil e assim
      "P": { "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] } }
    })");
    CHECK(r.problemas.empty());
    CHECK(r.projeteis.count("P") == 1);
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
    const auto& p = r.projeteis.at("P");
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
    REQUIRE(r.projeteis.count("P") == 1);
    CHECK(r.projeteis.at("P").animacaoInicial == "Normal");
}

// ---------------------------------------------------------------------------
// Problemas GRAVES: a entrada e descartada
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: sem sprite a entrada cai") {
    const auto r = LerProjeteis(Um(R"({ "dados": "a.json", "animacoes": { "A": [0] } })"));
    CHECK(Menciona(r.problemas, "sprite"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: sem dados a entrada cai") {
    const auto r = LerProjeteis(Um(R"({ "sprite": "a.png", "animacoes": { "A": [0] } })"));
    CHECK(Menciona(r.problemas, "dados"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: sem animacao a entrada cai") {
    // Sem animacao nao ha o que desenhar, e um projetil invisivel que mata e
    // pior que um projetil ausente.
    const auto r = LerProjeteis(Um(R"({ "sprite": "a.png", "dados": "a.json" })"));
    CHECK(Menciona(r.problemas, "animacoes"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: animacao de lista vazia nao conta como animacao") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [] }
    })"));
    CHECK(Menciona(r.problemas, "vazia"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: quadro negativo derruba a animacao") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0, -1] }
    })"));
    CHECK(Menciona(r.problemas, "negativo"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: quadro fracionario e recusado") {
    // 1.5 nao e indice de quadro, e truncar silenciosamente esconderia o erro.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0, 1.5] }
    })"));
    CHECK(Menciona(r.problemas, "inteiros"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: uma entrada ruim nao leva as boas do mesmo chefe") {
    const auto r = LerProjeteis(R"({
      "Bom":  { "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] } },
      "Ruim": { "sprite": "b.png", "animacoes": { "A": [0] } }
    })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.projeteis.count("Bom") == 1);
    CHECK(r.projeteis.count("Ruim") == 0);
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
    REQUIRE(r.projeteis.count("P") == 1);
    CHECK(r.projeteis.at("P").escala == doctest::Approx(1.0f));
}

TEST_CASE("Projeteis: divisor zero volta para 2 em vez de dividir por zero") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "colisorDivisor": 0
    })"));
    CHECK(Menciona(r.problemas, "colisorDivisor"));
    CHECK(r.projeteis.at("P").colisorDivisor == doctest::Approx(2.0f));
}

TEST_CASE("Projeteis: dimensao desconhecida volta para largura") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "colisorDimensao": "diagonal"
    })"));
    CHECK(Menciona(r.problemas, "diagonal"));
    CHECK(r.projeteis.at("P").colisorDimensao == "largura");
}

TEST_CASE("Projeteis: altura e uma dimensao valida") {
    // O projetil de lista duplamente encadeada e mais largo que alto, e usar a
    // largura dele daria uma hitbox bem maior que o desenho.
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "colisorDimensao": "altura"
    })"));
    CHECK(r.problemas.empty());
    CHECK(r.projeteis.at("P").colisorDimensao == "altura");
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
    const auto& p = r.projeteis.at("P");
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
    const auto& p = r.projeteis.at("P");
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
    const auto& p = r.projeteis.at("P");
    CHECK(p.margemEmSprites == doctest::Approx(1.0f));
    CHECK(p.margemDivisorDeTela == doctest::Approx(12.0f));
}

TEST_CASE("Projeteis: posicionarNoDono pode ser desligado") {
    const auto r = LerProjeteis(Um(R"({
        "sprite": "a.png", "dados": "a.json",
        "animacoes": { "A": [0] }, "posicionarNoDono": false
    })"));
    CHECK(r.problemas.empty());
    CHECK(r.projeteis.at("P").posicionarNoDono == false);
}

// ---------------------------------------------------------------------------
// Texto que nao serve
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: texto que nao e JSON e relatado, nao lancado") {
    const auto r = LerProjeteis("{ isto nao fecha");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: arquivo vazio nao quebra") {
    CHECK(LerProjeteis("").projeteis.empty());
    CHECK(LerProjeteis("{}").projeteis.empty());
}

TEST_CASE("Projeteis: raiz que nao e objeto e relatada") {
    const auto r = LerProjeteis("[1, 2, 3]");
    CHECK(Menciona(r.problemas, "raiz"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: entrada que nao e objeto e relatada") {
    const auto r = LerProjeteis(R"({ "P": 7 })");
    CHECK(Menciona(r.problemas, "P"));
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: arquivo sem nenhum projetil util sai vazio, e nao inventado") {
    // Quem diz que um chefe sem projetil e um problema e a PONTE, que sabe de
    // quem era o arquivo. Aqui so se exige nao sobrar lixo no mapa.
    const auto r = LerProjeteis(R"({ "P": { "sprite": "a.png" } })");
    CHECK_FALSE(r.problemas.empty());
    CHECK(r.projeteis.empty());
}

TEST_CASE("Projeteis: por padrao o sprite NAO gira com a velocidade") {

    // Girar so faz sentido para arte que tem frente. A capivara do Salles e os
    // baloes do Andre girariam a toa, entao quem nao pede fica parado.
    const auto r = LerProjeteis(R"({ "P": {
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] }
    } })");

    REQUIRE(r.projeteis.count("P") == 1);
    CHECK_FALSE(r.projeteis.at("P").rotacionarComAVelocidade);
}

TEST_CASE("Projeteis: rotacionarComAVelocidade pode ser ligado") {

    const auto r = LerProjeteis(R"({ "P": {
        "sprite": "a.png", "dados": "a.json", "animacoes": { "A": [0] },
        "rotacionarComAVelocidade": true
    } })");

    REQUIRE(r.projeteis.count("P") == 1);
    CHECK(r.projeteis.at("P").rotacionarComAVelocidade);
    CHECK(r.problemas.empty());
}

// ---------------------------------------------------------------------------
// "acorrenta": o acerto que quase nao cobra nota e em troca prende
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: \"acorrenta\" e lido inteiro") {

    const auto p = LerProjeteis(R"({
      "Corrente": {
        "sprite": "s.png", "dados": "s.json",
        "animacoes": { "A": [0] },
        "acorrenta": { "segundos": 3.5, "fatorDeVelocidade": 0.45,
                       "multiplicadorDaPerda": 0.15 }
      }
    })");

    CHECK(p.problemas.empty());
    REQUIRE(p.projeteis.count("Corrente") == 1);
    const auto& a = p.projeteis.at("Corrente").acorrenta;
    CHECK(a.segundos == doctest::Approx(3.5f));
    CHECK(a.fatorDeVelocidade == doctest::Approx(0.45f));
    CHECK(a.multiplicadorDaPerda == doctest::Approx(0.15f));
}

TEST_CASE("Projeteis: sem \"acorrenta\", o projetil nao prende ninguem") {

    // O PADRAO TEM DE SER INOFENSIVO. Todo projetil do jogo menos um nao tem
    // este campo, e um default errado aqui prenderia o aluno em TODA batalha.
    const auto p = LerProjeteis(R"({
      "Normal": { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] } }
    })");

    REQUIRE(p.projeteis.count("Normal") == 1);
    const auto& a = p.projeteis.at("Normal").acorrenta;
    CHECK(a.segundos == doctest::Approx(0.0f));
    CHECK(a.fatorDeVelocidade == doctest::Approx(1.0f));
    CHECK(a.multiplicadorDaPerda == doctest::Approx(1.0f));
}

TEST_CASE("Projeteis: um fator de velocidade fora de (0,1] derruba o efeito") {

    // FATOR MAIOR QUE 1 FARIA O ALUNO ACELERAR AO SER ACERTADO, e fator zero ou
    // negativo o deixaria parado ou andando para tras. Os tres sao engano de
    // digitacao, e nenhum deles e coisa que alguem procuraria no arquivo depois
    // - por isso o efeito inteiro e descartado e o problema e dito em voz alta.
    for (const char* ruim : {"1.4", "0", "-0.5"}) {
        CAPTURE(ruim);
        const auto texto = std::string(R"({ "C": { "sprite": "s.png", "dados": "s.json",
            "animacoes": { "A": [0] },
            "acorrenta": { "segundos": 3, "fatorDeVelocidade": )") + ruim + " } } }";

        const auto p = LerProjeteis(texto);
        REQUIRE(p.projeteis.count("C") == 1);
        CHECK(p.projeteis.at("C").acorrenta.segundos == doctest::Approx(0.0f));
        REQUIRE(p.problemas.size() == 1);
        CHECK(p.problemas[0].find("fatorDeVelocidade") != std::string::npos);
    }
}

TEST_CASE("Projeteis: \"acorrenta\" sem segundos nao prende, e reclama") {

    // Um objeto "acorrenta" vazio e alguem que comecou a escrever o efeito e
    // parou no meio. Aceitar calado daria um projetil que diz prender e nao
    // prende - pior do que nao ter o campo.
    const auto p = LerProjeteis(R"({
      "C": { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] },
             "acorrenta": { "fatorDeVelocidade": 0.5 } }
    })");

    CHECK(p.projeteis.at("C").acorrenta.segundos == doctest::Approx(0.0f));
    REQUIRE(p.problemas.size() == 1);
    CHECK(p.problemas[0].find("segundos") != std::string::npos);
}

// ---------------------------------------------------------------------------
// O projetil que o aluno derruba (os processos do Lucas)
// ---------------------------------------------------------------------------

TEST_CASE("Projeteis: \"derrubavel\" e falso por padrao, e pode ser ligado") {

    // O PADRAO TEM DE SER FALSO: verdadeiro em todo projetil faria cada tiro do
    // aluno parar no primeiro projetil do caminho, em toda batalha.
    const auto p = LerProjeteis(R"({
      "Normal":   { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] } },
      "Processo": { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] },
                    "derrubavel": true }
    })");

    CHECK(p.problemas.empty());
    REQUIRE(p.projeteis.count("Normal") == 1);
    REQUIRE(p.projeteis.count("Processo") == 1);
    CHECK_FALSE(p.projeteis.at("Normal").derrubavel);
    CHECK(p.projeteis.at("Processo").derrubavel);
}

TEST_CASE("Projeteis: \"derrubavel\" que nao e booleano reclama e fica falso") {

    // "sim" ou 1 seriam lidos como falso sem aviso, e o processo ficaria
    // imortal - o aluno atiraria nele a fase inteira sem saber por que nao cai.
    for (const char* ruim : {"\"sim\"", "1"}) {
        CAPTURE(ruim);
        const auto texto = std::string(R"({ "P": { "sprite": "s.png", "dados": "s.json",
            "animacoes": { "A": [0] }, "derrubavel": )") + ruim + " } }";
        const auto p = LerProjeteis(texto);
        REQUIRE(p.projeteis.count("P") == 1);
        CHECK_FALSE(p.projeteis.at("P").derrubavel);
        REQUIRE(p.problemas.size() == 1);
        CHECK(p.problemas[0].find("derrubavel") != std::string::npos);
    }
}

TEST_CASE("Projeteis: a resistencia e 1 por padrao, e so vale num derrubavel") {
    const auto p = LerProjeteis(R"({
      "Processo": { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] },
                    "derrubavel": true },
      "Janela":   { "sprite": "s.png", "dados": "s.json", "animacoes": { "A": [0] },
                    "derrubavel": true, "resistencia": 3 }
    })");
    CHECK(p.problemas.empty());
    CHECK(p.projeteis.at("Processo").resistencia == 1);
    CHECK(p.projeteis.at("Janela").resistencia == 3);

    // Zero, negativa, quebrada, ou num projetil que o tiro nao derruba: reclama
    // e fica 1. A ultima e quase sempre um "derrubavel" esquecido.
    for (const char* resto : { R"("derrubavel": true, "resistencia": 0)",
                               R"("derrubavel": true, "resistencia": 2.5)",
                               R"("resistencia": 3)" }) {
        CAPTURE(resto);
        const auto r = LerProjeteis(std::string(R"({ "P": { "sprite": "s.png", "dados": "s.json",
            "animacoes": { "A": [0] }, )") + resto + " } }");
        REQUIRE(r.projeteis.count("P") == 1);
        CHECK(r.projeteis.at("P").resistencia == 1);
        REQUIRE(r.problemas.size() == 1);
        CHECK(r.problemas[0].find("resistencia") != std::string::npos);
    }
}
