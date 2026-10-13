// Onde moram os arquivos de ataque de um chefe. Camada pura.
//
// Sao quatro linhas de codigo, e mesmo assim tem teste: esta convencao e citada
// por tres pontes e por toda a suite que le os Assets de verdade. Era disso que
// se tratava a refatoracao - antes o caminho estava escrito a mao em cada uma
// delas, e o nome do chefe aparecia tambem DENTRO do arquivo.

#include "doctest.h"

#include "../Source/Attacks/PastaDoChefe.h"

#include <string>

TEST_CASE("PastaDoChefe: o caminho e Attacks/<chefe>/<arquivo>") {

    CHECK(PastaDoChefe::Arquivo("thiago", PastaDoChefe::kFases) == "Attacks/thiago/fases.json");
    CHECK(PastaDoChefe::Arquivo("salles", PastaDoChefe::kRegras) == "Attacks/salles/regras.json");
    CHECK(PastaDoChefe::Arquivo("andre", PastaDoChefe::kProjeteis) == "Attacks/andre/projeteis.json");
}

TEST_CASE("PastaDoChefe: os tres nomes de arquivo sao distintos") {

    // Trocar dois deles sem querer faria um chefe carregar as regras como se
    // fossem fases, e o leitor so diria que o arquivo esta estranho.
    CHECK(std::string(PastaDoChefe::kFases) != std::string(PastaDoChefe::kRegras));
    CHECK(std::string(PastaDoChefe::kFases) != std::string(PastaDoChefe::kProjeteis));
    CHECK(std::string(PastaDoChefe::kRegras) != std::string(PastaDoChefe::kProjeteis));
}

TEST_CASE("PastaDoChefe: chefe vazio nao vira caminho") {

    // Materia sem batalha tem "chefe" vazio em materias.json. Montar
    // "Attacks//fases.json" poderia abrir alguma coisa por acidente, em vez de
    // simplesmente nao existir.
    CHECK(PastaDoChefe::Arquivo("", PastaDoChefe::kFases).empty());
    CHECK(PastaDoChefe::Arquivo("thiago", "").empty());
}

TEST_CASE("PastaDoChefe: o caminho e relativo a Assets, sem barra no comeco") {

    // Quem abre junta com a pasta base - Caminhos::Asset na ponte, DPI_ASSETS_DIR
    // no teste. Uma barra no comeco transformaria a juncao em caminho absoluto.
    const std::string c = PastaDoChefe::Arquivo("julio", PastaDoChefe::kFases);
    REQUIRE_FALSE(c.empty());
    CHECK(c.front() != '/');
    CHECK(c.find("Assets") == std::string::npos);
}
