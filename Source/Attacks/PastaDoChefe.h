//
// Onde moram os arquivos de ataque de um chefe.
//

#pragma once

#include <string>

/**
 * @brief A CONVENCAO DE PASTAS dos arquivos de ataque, num lugar so.
 *
 *     Assets/Attacks/thiago/fases.json
 *     Assets/Attacks/thiago/regras.json
 *     Assets/Attacks/thiago/projeteis.json
 *
 * Antes eram tres arquivos unicos com todos os chefes dentro, cada um com um
 * nivel a mais dizendo de quem era cada bloco. Com uma pasta por chefe, esse
 * nivel sumiu: a PASTA e o nome do chefe, e nao ha como o arquivo discordar
 * dela. Tambem deixou de ser preciso ler os cinco chefes para montar um.
 *
 * Esta funcao existe para a convencao nao ficar escrita em tres pontes
 * diferentes, mais uma vez em cada teste. Ela e PURA de proposito - devolve o
 * caminho relativo a Assets/, e quem abre resolve a pasta base: a ponte com
 * Caminhos::Asset, o teste com DPI_ASSETS_DIR.
 */
namespace PastaDoChefe {

constexpr const char* kFases = "fases.json";
constexpr const char* kRegras = "regras.json";
constexpr const char* kProjeteis = "projeteis.json";

/**
 * @brief O caminho de um arquivo de ataque, relativo a Assets/.
 *
 * Arquivo("thiago", kFases) -> "Attacks/thiago/fases.json"
 *
 * Chefe vazio devolve vazio: nome de chefe em branco e materia sem batalha, e
 * montar "Attacks//fases.json" acabaria lendo algo por acidente.
 */
[[nodiscard]] std::string Arquivo(const std::string& chefe, const std::string& arquivo);

} // namespace PastaDoChefe
