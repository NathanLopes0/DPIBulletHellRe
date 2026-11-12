//
// Onde moram os arquivos de ataque de um chefe.
//

#pragma once

#include <string>

/// A convencao de pastas dos arquivos de ataque, num lugar so.
namespace PastaDoChefe {

constexpr const char* kFases = "fases.json";
constexpr const char* kRegras = "regras.json";
constexpr const char* kProjeteis = "projeteis.json";

/// O caminho de um arquivo de ataque, relativo a Assets/. Arquivo("thiago", kFases) ->
/// "Attacks/thiago/fases.json"
[[nodiscard]] std::string Arquivo(const std::string& chefe, const std::string& arquivo);

} // namespace PastaDoChefe
