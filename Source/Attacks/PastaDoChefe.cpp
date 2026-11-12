//
// Onde moram os arquivos de ataque de um chefe.
//

#include "PastaDoChefe.h"

namespace PastaDoChefe {

std::string Arquivo(const std::string& chefe, const std::string& arquivo) {

    // Chefe ou arquivo em branco nao viram caminho.
    if (chefe.empty() || arquivo.empty()) return {};

    return "Attacks/" + chefe + "/" + arquivo;
}

} // namespace PastaDoChefe
