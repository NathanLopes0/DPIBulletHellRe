// Montagem e busca de caminhos. Camada pura: nenhum diretorio e criado, nenhum
// arquivo e aberto. A busca recebe de fora a pergunta "esta pasta serve?", e e
// isso que a torna testavel.

#include "doctest.h"

#include <set>
#include <string>

#include "../Source/Caminhos.h"

using Caminhos::Juntar;
using Caminhos::DiretorioPai;
using Caminhos::ProcurarSubindo;

// ---------------------------------------------------------------------------
// Juntar
// ---------------------------------------------------------------------------

TEST_CASE("Juntar: o caso comum") {
    CHECK(Juntar("C:/jogo", "Assets") == "C:/jogo/Assets");
}

TEST_CASE("Juntar: UMA barra, nunca duas, nunca nenhuma") {
    // O SDL_GetBasePath devolve o caminho JA com barra no fim, entao o caso de
    // barra sobrando e o normal, nao a excecao.
    CHECK(Juntar("C:/jogo/", "Assets") == "C:/jogo/Assets");
    CHECK(Juntar("C:/jogo", "/Assets") == "C:/jogo/Assets");
    CHECK(Juntar("C:/jogo/", "/Assets") == "C:/jogo/Assets");
}

TEST_CASE("Juntar: normaliza a barra invertida do Windows") {
    // As duas funcionam na API do sistema, mas um caminho com as duas misturadas
    // fica ilegivel no log - que e onde a gente olha quando um arquivo nao abre.
    CHECK(Juntar("C:\\jogo", "Assets") == "C:/jogo/Assets");
    CHECK(Juntar("C:\\jogo\\", "Teachers\\salles.png") == "C:/jogo/Teachers/salles.png");
}

TEST_CASE("Juntar: parte vazia nao vira barra solta") {
    // Quem chama nao deve precisar tratar o caso de a base ainda nao ter sido
    // descoberta.
    CHECK(Juntar("", "Assets") == "Assets");
    CHECK(Juntar("C:/jogo", "") == "C:/jogo");
    CHECK(Juntar("", "") == "");
}

TEST_CASE("Juntar: caminho relativo continua relativo") {
    CHECK(Juntar("..", "Assets") == "../Assets");
    CHECK(Juntar(".", "Assets") == "./Assets");
}

TEST_CASE("Juntar: raiz unix nao perde a barra") {
    CHECK(Juntar("/", "opt") == "/opt");
    CHECK(Juntar("/opt", "jogo") == "/opt/jogo");
}

// ---------------------------------------------------------------------------
// DiretorioPai
// ---------------------------------------------------------------------------

TEST_CASE("DiretorioPai: sobe um nivel") {
    CHECK(DiretorioPai("C:/jogo/build/Debug") == "C:/jogo/build");
    CHECK(DiretorioPai("/opt/jogo/build") == "/opt/jogo");
}

TEST_CASE("DiretorioPai: barra no fim nao conta como nivel") {
    // "C:/jogo/build/" e "C:/jogo/build" sao o mesmo diretorio, entao sobem para
    // o mesmo lugar. Sem isto, a busca gastaria um nivel para nada.
    CHECK(DiretorioPai("C:/jogo/build/") == "C:/jogo");
    CHECK(DiretorioPai("C:/jogo/build") == "C:/jogo");
}

TEST_CASE("DiretorioPai: a raiz nao vira string vazia") {
    // Se a raiz virasse vazia, a subida perderia a referencia de onde estava e o
    // laco de busca nao teria como parar com sentido.
    CHECK(DiretorioPai("/opt") == "/");
    CHECK(DiretorioPai("/") == "");
}

TEST_CASE("DiretorioPai: sem nivel acima devolve vazio") {
    CHECK(DiretorioPai("jogo") == "");
    CHECK(DiretorioPai("") == "");
}

// ---------------------------------------------------------------------------
// ProcurarSubindo
// ---------------------------------------------------------------------------

/// Simula um disco: um conjunto de diretorios que "tem Assets dentro".
static Caminhos::Predicado Disco(const std::set<std::string>& comAssets) {
    return [comAssets](const std::string& d) { return comAssets.count(d) == 1; };
}

TEST_CASE("Procurar: acha na propria pasta de partida") {
    CHECK(ProcurarSubindo("C:/jogo", 5, Disco({"C:/jogo"})) == "C:/jogo");
}

TEST_CASE("Procurar: sobe a partir da pasta de build") {
    // O caso real: o executavel fica em cmake-build-debug e Assets esta um nivel
    // acima.
    CHECK(ProcurarSubindo("C:/jogo/cmake-build-debug", 5, Disco({"C:/jogo"})) == "C:/jogo");
}

TEST_CASE("Procurar: sobe varios niveis") {
    // O Visual Studio enterra o executavel mais fundo: out/build/x64-Debug.
    const auto achou = ProcurarSubindo("C:/jogo/out/build/x64-Debug", 5, Disco({"C:/jogo"}));
    CHECK(achou == "C:/jogo");
}

TEST_CASE("Procurar: para no limite de niveis") {
    // Sem limite, uma arvore sem Assets faria a busca ir ate a raiz do disco e
    // potencialmente achar um "Assets" de outro projeto.
    CHECK(ProcurarSubindo("C:/a/b/c/d/e/f/g", 2, Disco({"C:/a"})).empty());
    CHECK(ProcurarSubindo("C:/a/b/c", 2, Disco({"C:/a"})) == "C:/a");
}

TEST_CASE("Procurar: devolve a PRIMEIRA que serve, de baixo para cima") {
    // Se houver Assets em dois niveis, o mais proximo do executavel ganha - e o
    // que um build dentro do projeto espera.
    CHECK(ProcurarSubindo("C:/jogo/build", 5, Disco({"C:/jogo", "C:/jogo/build"}))
          == "C:/jogo/build");
}

TEST_CASE("Procurar: nao achou devolve vazio, e nao um palpite") {
    CHECK(ProcurarSubindo("C:/jogo/build", 5, Disco({"D:/outro"})).empty());
}

TEST_CASE("Procurar: predicado nulo nao quebra") {
    CHECK(ProcurarSubindo("C:/jogo", 5, nullptr).empty());
}

TEST_CASE("Procurar: nao entra em laco infinito na raiz") {
    // DiretorioPai("/") devolve vazio, entao a subida precisa parar sozinha.
    CHECK(ProcurarSubindo("/", 100, Disco({"nenhum"})).empty());
    CHECK(ProcurarSubindo("/a", 100, Disco({"nenhum"})).empty());
}

TEST_CASE("Procurar: aceita barra invertida na partida") {
    // O SDL_GetBasePath no Windows devolve com barra invertida.
    CHECK(ProcurarSubindo("C:\\jogo\\cmake-build-debug\\", 5, Disco({"C:/jogo"}))
          == "C:/jogo");
}
