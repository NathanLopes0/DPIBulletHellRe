// Onde moram os arquivos de ataque de um chefe.

#include "doctest.h"

#include "../Source/Attacks/PastaDoChefe.h"

#include <string>

TEST_CASE("PastaDoChefe: o caminho e Attacks/<chefe>/<arquivo>") {

    CHECK(PastaDoChefe::Arquivo("thiago", PastaDoChefe::kFases) == "Attacks/thiago/fases.json");
    CHECK(PastaDoChefe::Arquivo("salles", PastaDoChefe::kRegras) == "Attacks/salles/regras.json");
    CHECK(PastaDoChefe::Arquivo("andre", PastaDoChefe::kProjeteis) == "Attacks/andre/projeteis.json");
}

TEST_CASE("PastaDoChefe: os tres nomes de arquivo sao distintos") {

    CHECK(std::string(PastaDoChefe::kFases) != std::string(PastaDoChefe::kRegras));
    CHECK(std::string(PastaDoChefe::kFases) != std::string(PastaDoChefe::kProjeteis));
    CHECK(std::string(PastaDoChefe::kRegras) != std::string(PastaDoChefe::kProjeteis));
}

TEST_CASE("PastaDoChefe: chefe vazio nao vira caminho") {

    // Materia sem batalha tem "chefe" vazio em materias.json.
    CHECK(PastaDoChefe::Arquivo("", PastaDoChefe::kFases).empty());
    CHECK(PastaDoChefe::Arquivo("thiago", "").empty());
}

TEST_CASE("PastaDoChefe: o caminho e relativo a Assets, sem barra no comeco") {

    // Quem abre junta com a pasta base - Caminhos::Asset na ponte, DPI_ASSETS_DIR no teste.
    const std::string c = PastaDoChefe::Arquivo("julio", PastaDoChefe::kFases);
    REQUIRE_FALSE(c.empty());
    CHECK(c.front() != '/');
    CHECK(c.find("Assets") == std::string::npos);
}
