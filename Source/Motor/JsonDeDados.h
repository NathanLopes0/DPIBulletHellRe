//
// A leitura de JSON dos arquivos de dados do jogo.
//

#pragma once

#include <string>

#include "Json.h"

/// Interpreta o texto de um arquivo de dados do jogo.
inline nlohmann::json LerJsonDeDados(const std::string& texto) {
    // Callback excecoes ignora comentarios.
    return nlohmann::json::parse(texto, nullptr, true, /*ignore_comments*/ true);
}
